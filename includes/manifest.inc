<?php
/**
* Implements hook_islandora_iiif_manifest_query_backends().
*/
function islandora_iiif_islandora_iiif_manifest_query_backends() {
  $module_path = drupal_get_path('module', 'islandora_solr');
  return array(
    'islandora_solr_query_backend' => array(
      'title' => t('Manifest via Solr'),
      'callable' => 'islandora_iiif_islandora_manifest_solr_backend_callable',
      'file' => "$module_path/includes/manifest.inc",
    ),
  );
}
/**
* Implements callback_islandora_iiif_manifest_query_backends()
*/
function islandora_iiif_islandora_manifest_solr_backend_callable($object, $page, $limit) {
  module_load_include('inc', 'islandora_solr', 'includes/db');
  $qp = new IslandoraSolrQueryProcessor();

  $qp->buildQuery(format_string('!member_field:("info:fedora/!pid" OR "!pid") OR !collection_member_field:("info:fedora/!pid" OR "!pid")', array(
    '!member_field' => variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms'),
    '!pid' => $object->id,
    '!collection_member_field' => variable_get('islandora_solr_member_of_collection_field', 'RELS_EXT_isMemberOfCollection_uri_ms'),
  )), drupal_get_query_parameters());

  $limit = variable_get('islandora_solr_num_of_results', '20');

  $qp->solrStart = $page * $limit;
  $qp->solrLimit = $limit;

  $qp->solrParams['sort'] = $qp->internalSolrParams['sort'];

  $qp->executeQuery();

  $map_to_pids = function ($result) {
    return $result['PID'];
  };
  return array(
    $qp->islandoraSolrResult['response']['numFound'],
    array_map($map_to_pids, $qp->islandoraSolrResult['response']['objects']),
  );
};
  
/**
* This is the actual manifest building function
*/ 
function islandora_iiif_manifest_callback(AbstractObject $object) {
  module_load_include('inc', 'islandora_solr_metadata', 'includes/db');
  module_load_include('inc', 'islandora_solr_metadata', 'theme/theme');
  $manifest = new \stdClass;
  $meta = '"@context": "http://iiif.io/api/presentation/2/context.json",
  "@id": "http://example.org/iiif/book1/manifest",
  "@type": "sc:Manifest",';
  $manifest->label = $object->label;
   
  $associations = islandora_solr_metadata_get_associations_by_cmodels($object->models);
  dpm($associations);

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
    dpm($field);
    $db_fields = array_merge($db_fields, $field);
    $description = islandora_solr_metadata_retrieve_description($configuration_id['configuration_id']);
    if ($description['description_field'] !== NULL) {
      $descriptions[] = $description;
    }
  }
  if (count($descriptions)) {
    $finaldescription = implode($separator, $descriptions);
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
      $metadata[] = array("label" => $key, "value" =>$value);
    }
  }
  $manifest->metadata = $metadata;
  $manifest->description = $finaldescription;
  // The reason i went for current_path() is because someone could 
  // wants to serve this via an alias.
  // The use of url() also allows for further altering via outgoing and incomming
  // alterhooks.
  
  $manifest->description = array(
    "@id" => url(current_path(),
    array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    )
  ),
  "thumbnail" => array(
    "@id" => url("islandora_iiif/{$object->id}/full/50,/0/default.jp2", array(
      'absolute' => TRUE,
      'language' => (object)array('language' => FALSE),
    )
  ),
  "service" => array(
    "@context" => "http://iiif.io/api/image/2/context.json",
    "@id" => "http://example.org/images/book1-page1",
    "profile" => "http://iiif.io/api/image/2/level1.json",
  ),
),
);
drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
echo json_encode($manifest, JSON_PRETTY_PRINT);
drupal_page_footer();
 
}