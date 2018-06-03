<?php

/**
 * @file
 */

/**
 * Implements hook_islandora_iiif_collection_query_backends().
 */
function islandora_iiif_islandora_iiif_collection_query_backends() {
  $module_path = drupal_get_path('module', 'islandora_solr');
  return array(
    'islandora_solr_query_backend' => array(
      'title' => t('Collection via Solr'),
      'callable' => 'islandora_iiif_islandora_collection_solr_backend_callable',
      'file' => "$module_path/includes/collection.inc",
    ),
  );
}

/**
 * Implements callback_islandora_iiif_collection_query_backends()
 */
function islandora_iiif_islandora_collection_solr_backend_callable($object, $index) {
  module_load_include('inc', 'islandora_solr', 'includes/db');
  $qp = new IslandoraSolrQueryProcessor();

  $qp->buildQuery(format_string('!member_field:("info:fedora/!pid" OR "!pid") OR !collection_member_field:("info:fedora/!pid" OR "!pid")', array(
    '!member_field' => variable_get('islandora_solr_member_of_field', 'RELS_EXT_isMemberOf_uri_ms'),
    '!pid' => $object->id,
    '!collection_member_field' => variable_get('islandora_solr_member_of_collection_field', 'RELS_EXT_isMemberOfCollection_uri_ms'),
  )), drupal_get_query_parameters());

  $limit = variable_get('islandora_solr_num_of_results', '20');

  $qp->solrStart = $index;
  $qp->solrLimit = $limit;

  if (isset($qp->internalSolrParams['sort'])) {
    $qp->solrParams['sort'] = $qp->internalSolrParams['sort'];
  }

  $qp->executeQuery(FALSE);

  // dpm($qp->islandoraSolrResult['response']);

  return array(
    $qp->islandoraSolrResult['response']['numFound'],
    $qp->islandoraSolrResult['response']['objects'],
  );
};

/**
 * This is the actual collection building function.
 */
function islandora_iiif_collection_callback(AbstractObject $object, $index = 1) {

  module_load_include('inc', 'islandora_solr', 'includes/breadcrumbs');
  if (!in_array('islandora:collectionCModel', $object->models)) {
    drupal_not_found();
  }

  $limit = variable_get('islandora_solr_num_of_results', '20');

  $children_list = islandora_iiif_islandora_collection_solr_backend_callable($object, $index);
  $total_children = $children_list[0];
  $current_response = $children_list[1];
  $collection_members = array();
  $collection_manifests = array();
  $collection_collections = array();
  $istop = variable_get('islandora_repository_pid', 'islandora:root') == $object->id;
  if ($children_list[0] != 0) {
    foreach ($current_response as $children_info) {
      $label = "";

      $label = $children_info['object_label'];
      $thumb = array(
        "@id" => urldecode(url("islandora_iiif/{$object->id}/full/50,/0/default.jp2", array(
          'absolute' => TRUE,
          'language' => (object) array('language' => FALSE),
        ))),
        "service" => array(
          "@context" => "http://iiif.io/api/image/2/context.json",
          "@id" => urldecode(url("islandora_iiif/{$object->id}/info.json", array(
            'absolute' => TRUE,
            'language' => (object) array('language' => FALSE),
          ))),
          "profile" => "http://iiif.io/api/image/2/level2.json",
        ),
      );
      if (in_array('info:fedora/islandora:collectionCModel', $children_info['content_models'])) {
        $id = urldecode(url("islandora_iiif/collection/{$children_info['PID']}", array(
          'absolute' => TRUE,
          'language' => (object) array('language' => FALSE),
        )));
        $type = "sc:Collection";
        $collection_collections[] = array(
          "@id" => $id,
          "@type" => $type,
          "label" => $label,
          "thumbnail" => $thumb,
        );
      }
      else {
        $id = urldecode(url("islandora_iiif/{$children_info['PID']}/manifest", array(
          'absolute' => TRUE,
          'language' => (object) array('language' => FALSE),
        )));
        $type = "sc:Manifest";
        $collection_manifests[] = array(
          "@id" => $id,
          "@type" => $type,
          "label" => $label,
          "thumbnail" => $thumb,
        );
      }
      // I now it looks weird, but API 3.o will use members and i want to respect
      // Islandora's ordering versus what 2.1 imposes, manifest v/s collection separation.
      $collection_members[] = array(
        "@id" => $id,
        "@type" => $type,
        "label" => $label,
        "thumbnail" => $thumb,
      );
    }
  }
  $collection = new \stdClass();
  $collection->{'@context'} = "http://iiif.io/api/presentation/2/context.json";
  $collection->{'@id'} = urldecode(url(current_path(),
      array(
        'absolute' => TRUE,
        'language' => (object) array('language' => FALSE),
      )
    )
  );
  $collection->{'@type'} = "sc:Collection";
  // Yes, this is ours.
  $collection->viewingHint = "multi-part";

  $collection->label = $object->label;
  $collection->startIndex = $index;

  $next_url_str = NULL;
  $prev_url_str = NULL;

  // Few assumptiomns
  // A. If total results is less than $limit, then no $next,
  // B. if $index + $limit is more than total, then no $next
  // C. if $index - $limit <= 0 then $prev is parent top
  // if $index = 1 then no $prev.
  $collection->total = (int) $total_children;

  if (($total_children > $index + $limit) && ($total_children > 0)) {
    $nextid = $index+$limit+1;
    $next_url_str = "islandora_iiif/collection/{$object->id}/{$nextid}";
  }
  if (($index > $limit) && ($total_children > 0)) {
    $nextid = ($index-$limit+1);
    $prev_url_str = "islandora_iiif/collection/{$object->id}/{$nextid}";
  }

  if ($next_url_str) {
    $collection->next = array(
      "@id" => urldecode(url($next_url_str, array(
        'absolute' => TRUE,
        'language' => (object) array('language' => FALSE),
      ))),
    );
  }
  if ($prev_url_str) {
    $collection->prev = array(
      "@id" => urldecode(url($prev_url_str, array(
        'absolute' => TRUE,
        'language' => (object) array('language' => FALSE),
      ))),
    );
  }
  if (!$istop) {
    $parent = islandora_solr_get_breadcrumb_parent($object->id);
    if (!empty($parent)) {
      $collection->within = array(
        "@id" => urldecode(url("islandora_iiif/collection/{$parent['PID']}", array(
          'absolute' => TRUE,
          'language' => (object) array('language' => FALSE),
        ))
        ),
      );
    }
  }
  if (!empty($collection_manifests)) {
    $collection->manifests = $collection_manifests;
  }
  if (!empty($collection_collections)) {
    $collection->collections = $collection_collections;
  }
  if (!empty($collection_members)) {
    $collection->members = $collection_members;
  }

  // And to make this legit we need a sequence with some canvases.
  drupal_add_http_header('Content-Type', 'application/json; charset=utf-8');
  drupal_add_http_header('access-control-allow-origin', '*');
  echo json_encode($collection, JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES | JSON_UNESCAPED_UNICODE | JSON_NUMERIC_CHECK);
  drupal_page_footer();

}
