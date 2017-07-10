<?php
/**
* Implements hook_islandora_iiif_manifest_query_backends().
*/
function islandora_iiif_islandora_iiif_canvas_query_backends() {
  $module_path = drupal_get_path('module', 'islandora_solr');
  return array(
    'islandora_solr_query_backend' => array(
      'title' => t('Manifest via Solr'),
      'callable' => 'islandora_iiif_islandora_canvas_solr_backend_callable',
      'file' => "$module_path/includes/canvas.inc",
    ),
  );
}
/**
* Implements callback_islandora_iiif_canvas_query_backends()
*/
function islandora_iiif_islandora_canvas_solr_backend_callable($object) {
  module_load_include('inc', 'islandora_solr', 'includes/db');
  module_load_include('inc', 'islandora_solr', 'includes/db');
    $qp = new IslandoraSolrQueryProcessor();
  
 
 
    $qp->buildQuery(format_string('"PID:!pid"'), array(
      '!pid' => $object->id,
    ));
    
    $limit = variable_get('islandora_solr_num_of_results', '20');

    $qp->solrStart = 0;
    $qp->solrLimit = 1;

    // We should have a default sort here for sequences.



    $qp->executeQuery();
  
  
    return array(
      $qp->islandoraSolrResult['response']['numFound'],
      $qp->islandoraSolrResult['response']['objects'],
    );
}
  
/**
* This is the actual canvas building function
*/ 
function islandora_iiif_canvas_callback(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  module_load_include('inc', 'islandora_solr_metadata', 'theme/theme');
 
   
  $associations = islandora_solr_metadata_get_associations_by_cmodels($object->models);


  $separator = variable_get('islandora_solr_metadata_field_value_separator', "\n");

  if (empty($separator)) {
    $separator = "\n";
  }
  
  $descriptions = array();
  $db_fields = array();
  $solr_fields = array();
  $metadata = array();
  $finaldescription = '';
  
  foreach ($associations as $configuration_id) {
    $field = islandora_solr_metadata_get_fields($configuration_id['configuration_id']);
    $db_fields = array_merge($db_fields, $field);
    $description = islandora_solr_metadata_retrieve_description($configuration_id['configuration_id']);
    if ($description['description_field'] !== NULL) {
      dpm($db_fields);
      dpm( $description);
      //$db_fields  = array_merge($db_fields, $description);
    }
  }
  
  foreach ($db_fields as $solr_field => $value) {
    if (isset($solr_fields[$solr_field])) {
      continue;
    }
    // Make an array for use later on.
    $solr_fields[$solr_field] = $value + array(
      'value' => array(),
    );
  }
  $separator = variable_get('islandora_solr_metadata_field_value_separator', "\n");
  if (islandora_solr_metadata_query_fields($object, $solr_fields)) {
    foreach ($solr_fields as $key => $value) {
      dpm($value);
      $metadata[] = array("label" => $value['display_label'], "value" =>$value['value']);
    }
  }
  
  $manifest = new \stdClass;
  $manifest->{'@context'} = "http://iiif.io/api/presentation/2/context.json";
  $manifest->{'@id'} = urldecode(url(current_path(),
     array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))
  );
  $manifest->{'@type'} = "sc:Manifest";
  
  $manifest->label = $object->label;
  
  
  
  $manifest->metadata = $metadata;
  $manifest->description = $finaldescription;
  // The reason i went for current_path() is because someone could 
  // wants to serve this via an alias.
  // The use of url() also allows for further altering via outgoing and incomming
  // alterhooks.
  
  $manifest->thumbnail = array(
    "@id" => urldecode(url("islandora_iiif/{$object->id}/full/50,/0/default.jp2", array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))
  ),
  "service" => array(
    "@context" => "http://iiif.io/api/image/2/context.json",
    "@id" => urldecode(url("islandora_iiif/{$object->id}/info.json", array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))),
    "profile" => "http://iiif.io/api/image/2/level2.json",
  ),
);

 // And to make this legit we need a sequence with some canvases
 
 $manifest->sequences = array(

        "@id" =>"http://example.org/iiif/book1/sequence/normal",
        "@type"  => "sc:Sequence",
        "label" => "Current Page Order"
        // sequence's page order should be included here, see below...
      // Any additional sequences can be referenced here...
    );




drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
echo json_encode($manifest, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
drupal_page_footer();
 
}