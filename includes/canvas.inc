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
  
 
 
    $qp->buildQuery(format_string('PID:"!pid"', array(
      '!pid' => $object->id,
    )));
    
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
  
  $canvas_info = islandora_iiif_islandora_canvas_solr_backend_callable($object);
  $height = isset($canvas_info[0]['solr_doc']['RELS_INT_height_literal_s']) ? $canvas_info[0]['solr_doc']['RELS_INT_height_literal_s'] : '0';
  $width = isset($canvas_info[0]['solr_doc']['RELS_INT_width_literal_s']) ? $canvas_info[0]['solr_doc']['RELS_INT_width_literal_s'] : '0';
  $canvas = new \stdClass;
  $canvas->{'@context'} = "http://iiif.io/api/presentation/2/context.json";
  $canvas->{'@id'} = urldecode(url(current_path(),
     array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    ))
  );
  
  $id = urldecode(url("islandora_iiif/{$object->id}/canvas/default", array(
    'absolute' => TRUE,
    'language' => (object)array('language' => FALSE),
    )));
  
  $canvas->{'@type'} = "sc:Canvas";
  $canvas->label = $object->label;
  $canvas->height = $height;
  $canvas->width = $width;
   $canvas->images = array (
        "@type" => "oa:Annotation", 
        "motivation" => "sc:painting", 
        "resource" => array(
          "@id" => urldecode(url("islandora_iiif/{$object->id}/full/max/default.jp2", array(
            'absolute' => TRUE,
            'language' => (object)array('language' => FALSE),
            ))
          ), 
          "@type" => "dctypes:Image", 
          "height" => $height,
          "width" => $width, 
          ), 
        "on" => $id, 
        "otherContent" => array(
            "@type" => "sc:AnnotationList",
            "@embed" => false,
          ),
      );

 // And to make this legit we need a sequence with some canvases

drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
echo json_encode($canvas, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE);
drupal_page_footer();
 
}