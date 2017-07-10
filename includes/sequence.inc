<?php
/**
* Implements hook_islandora_iiif_manifest_query_backends().
*/
function islandora_iiif_islandora_iiif_sequence_query_backends() {
  $module_path = drupal_get_path('module', 'islandora_solr');
  return array(
    'islandora_solr_query_backend' => array(
      'title' => t('Sequence via Solr'),
      'callable' => 'islandora_iiif_islandora_sequence_solr_backend_callable',
      'file' => "$module_path/includes/sequence.inc",
    ),
  );
}

/**
* Implements hook_islandora_iiif_parentsolr_predicate()
*/
function islandora_iiif_islandora_iiif_parentsolr_predicate(array &$predicates, AbstractObject $object = NULL) {
  
  // This is the simplest implementation of this, we won't
  // care here about a particular CMODEL, we simply respond with 
  // all predicates we care for. 
  // other implementations can simply alter or overwrite what they find.
  // No particular order here
  $rels_predicate_compound = 'RELS_EXT_'.variable_get('islandora_compound_object_relationship', 'isConstituentOf').'_ms';
  $rels_predicate_paged = 'RELS_EXT_isPageOf_ms';
  $rels_predicate_generic_member = variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms');
  
  // using literal keys is faster way to avoid duplicates than
  // de-duplicating
  $predicates[$rels_predicate_compound] = $rels_predicate_compound;
  $predicates[$rels_predicate_paged] = $rels_predicate_paged;
  $predicates[$rels_predicate_generic_member] = $rels_predicate_generic_member;
  
}






/**
* Implements callback_islandora_iiif_manifest_query_backends()
*/
function islandora_iiif_islandora_sequence_solr_backend_callable($object, $page, $limit) {
  module_load_include('inc', 'islandora_solr', 'includes/db');
  $qp = new IslandoraSolrQueryProcessor();
  $solr_predicates = array();
  $solr_predicate_providers = module_invoke_all('islandora_iiif_parentsolr_predicate', $solr_predicates, $object);  
  // And the alter for those who like to alter.
  drupal_alter('islandora_iiif_parentsolr_predicate', $object, $solr_predicates);
 
  $qp->buildQuery(format_string("!predicates", array(
    '!predicates' => !empty($solr_predicates) ? (implode('OR ', $solr_predicates) . ':("info:fedora/!pid" OR "!pid")') : '',
    '!pid' => $object->id,
  )), drupal_get_query_parameters());

  $limit = variable_get('islandora_solr_num_of_results', '20');

  $qp->solrStart = $page * $limit;
  $qp->solrLimit = $limit;

  // We should have a default sort here for sequences.
  $qp->solrParams['sort'] = "RELS_EXT_isSequenceNumber_s ASC, RELS_EXT_isPageNumber_s ASC"; 
  if (!empty($qp->internalSolrParams['sort'])) {
    $qp->solrParams['sort'] .= ",". $qp->internalSolrParams['sort'];
  }


  $qp->executeQuery();

  $map_to_pids = function ($result) {
    return $result['PID'];
  };
  
  
  return array(
    $qp->islandoraSolrResult['response']['numFound'],
    $qp->islandoraSolrResult['response']['objects'],
  );
};

/**
* This is the actual manifest building function
*/ 
function islandora_iiif_sequence_generator(AbstractObject $object) { 
  // This is weird. Just to test really. I´m putting the limit to 2000 here. Would love if 
  $canvas_list = islandora_iiif_islandora_sequence_solr_backend_callable($object, 1, 2000);
  

  $sequence = new \stdClass;
  $sequence->{'@context'} = "http://iiif.io/api/presentation/2/context.json";
  $sequence->{'@id'} = urldecode(
    url(current_path(), array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
      )
    )
  );
  $sequence->{'@type'} = "sc:Sequence";
  $sequence->label = "Normal";
  if ($canvas_list[0] > 0) {
  foreach ($canvas_list[1] as $canvas_info) {
    $id = urldecode(url("islandora_iiif/{$canvas_info['PID']}/canvas/default", array(
        'absolute' => TRUE,
        'language' => (object)array('language' => FALSE),
        )));
    
    $canvases[] = array(
      "@id" => $id , 
      "@type" => "sc:Canvas", 
      "label" => "Label", 
      "height" => $canvas_info['RELS_INT_height_literal_s'], 
      "width" => $canvas_info['RELS_INT_width_literal_s'], 
      "images" => array (
        "@type" => "oa:Annotation", 
        "motivation" => "sc:painting", 
        "resource" => array(
          "@id" => urldecode(url("islandora_iiif/{$canvas_info['PID']}/full/max/default.jp2", array(
            'absolute' => TRUE,
            'language' => (object)array('language' => FALSE),
            ))
          ), 
          "@type" => "dctypes:Image", 
          "height" => $canvas_info['RELS_INT_height_literal_s'], 
          "width" => $canvas_info['RELS_INT_height_literal_s'],
          ), 
          "on" => $id, 
        )
      );    
    }

  // And to make this legit we need a sequence with some canvases
 
  $sequence->canvases = $canvases;
  }

  return $sequence;
}

/**
* This is the actual manifest building function
*/ 
function islandora_iiif_sequence_callback(AbstractObject $object) {
  $sequence = islandora_iiif_sequence_generator($object);
  drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
  echo json_encode($sequence, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
  drupal_page_footer();

}

