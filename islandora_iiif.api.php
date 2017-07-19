<?php

/**
 * @file
 * Hooks provided by Islandora IIIF.
 */

/**
 * @addtogroup hooks
 * @{
 */

/**
 * Describe "backends" which might be selected between.
 *
 * We have a number of different mechanisms which might be used to generate
 * lists of PIDs and basic metadata belonging to given collections.
 *
 * @return array
 *   Should return an associative array mapping unique (module-prefixed,
 *   preferably) keys to associative arrays containing:
 *   - title: A human-readable title for the backend.
 *   - callable: A PHP callable to call for this backend, implementing
 *     callback_islandora_iiif_manifest_query_backends().
 *   - file: An optional file to load before attempting to call the callable.
 */
function hook_islandora_iiif_collection_query_backends() {
  $a_callable = function ($object, $page, $limit) {
    // Do something to get the total number of objects and this page return
    // them.
    $iiif = array('islandora:1' => 'Object one', 'islandora:2' => 'Second object');
    // Could be more of course.
    $total = count($iiif);
    return array($total, $iiif);
  };
  return array(
    'awesome_backend' => array(
      'title' => t('Awesome IIIF Backend'),
      'callable' => $a_callable,
    ),
  );
}

/**
 * Generate a total count and paged list of PIDs.
 *
 * Callback for hook_islandora_basic_collection_query_backends().
 *
 * @param AbstractObject $object
 *   A collection object for which to obtain members.
 * @param int $page
 *   The page of PIDs for which to query.
 * @param int $limit
 *   The number of PIDs for which to query.
 *
 * @return array
 *   An array containing:
 *   - An integer representing the total number of objects in the collection.
 *   - An associative array containing up to $limit strings with keys containing object PIDs
 *     belonging to the given collection and label as value.
 */
function callback_iiif_collection_query_backends(AbstractObject $object, $page, $limit) {
  // Do something to get the total number of objects and this page return
  // them.
  $iiif = array('islandora:1' => 'Object one', 'islandora:2' => 'Second object');
  return array($total, $iiif);
}

/**
 * Describe "backends" which might be selected between.
 *
 * We have a number of different mechanisms which might be used to generate
 * lists of PIDs and basic metadata belonging to given collections.
 *
 * @return array
 *   Should return an associative array mapping unique (module-prefixed,
 *   preferably) keys to associative arrays containing:
 *   - title: A human-readable title for the backend.
 *   - callable: A PHP callable to call for this backend, implementing
 *     callback_islandora_iiif_manifest_query_backends().
 *   - file: An optional file to load before attempting to call the callable.
 */
function hook_islandora_iiif_manifest_query_backends() {
  $a_callable = function ($object, $page, $limit) {
    // Do something to get the total number of objects and this page return
    // them.
    $iiif = array('islandora:1' => 'Object one', 'islandora:2' => 'Second object');
    // Could be more of course.
    $total = count($iiif);
    return array($total, $iiif);
  };
  return array(
    'awesome_backend' => array(
      'title' => t('Awesome IIIF Backend'),
      'callable' => $a_callable,
    ),
  );
}

/**
 * Generate a total count and paged list of PIDs.
 *
 * Callback for hook_islandora_basic_collection_query_backends().
 *
 * @param AbstractObject $object
 *   A collection object for which to obtain members.
 * @param int $page
 *   The page of PIDs for which to query.
 * @param int $limit
 *   The number of PIDs for which to query.
 *
 * @return array
 *   An array containing:
 *   - An integer representing the total number of objects in the collection.
 *   - An associative array containing up to $limit strings with keys containing object PIDs
 *     belonging to the given collection and label as value.
 */
function callback_iiif_manifest_query_backends(AbstractObject $object, $page, $limit) {
  // Do something to get the total number of objects and this page return
  // them.
  $iiif = array('islandora:1' => 'Object one', 'islandora:2' => 'Second object');
  return array($total, $iiif);
}
