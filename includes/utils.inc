<?php
/**
 * Cleans a fake filename for IIIF´s Server API.
 * @param string $filename
 *  A File name as IIIF Server API would expect
 *
 * @return array()
 *  Containing basename and its extension 
 *
 * @author Diego Pino Navarro
 */

function islandora_iiif_safefile($filename, $allowedExtensions = array('jpg')) {
  // @TODO: this should be defined per backend image implementation.
    
  list($filename) = explode('?', $filename);
  list($filename) = explode('#', $filename);
  var_dump($filename);
  $filename = rawurldecode(drupal_basename($filename));
  // Remove white space variations.
  $filename = trim($filename, " \t\n\r\0\x0B.");

  if (strpos($filename, '.') === FALSE) {
    $extension = FALSE;
    var_dump($filename);
  }
  else {
    $dotpos = strrpos($filename, '.');
    $extension = drupal_strtolower(substr($filename, $dotpos + 1));
    $basename = drupal_strtolower(substr($filename, 0, $dotpos));
  }

  if (!$extension || !in_array($extension, $allowedExtensions, TRUE)) {
    throw new RuntimeException(t('The file @file has an invalid extension.', array('@file' => $filename)));
  }

  $basename = file_munge_filename($basename, implode(" ",$allowedExtensions), FALSE);
  return array($basename, $extension);
}

function islandora_iiif_adoredjatoka_info_json($image_info) {
  $info = array(
    "@context" => "http://iiif.io/api/image/2/context.json",
    "@embed" => "true",
    "protocol" => "http://iiif.io/api/image",
    "width" => (int)$image_info["width"],
    "height" => (int)$image_info["height"],
    "tiles"=> array(
      array("width" => 512, "scaleFactors" => array(1,2,4,8,16))
    ),
    "profile" => array("http=>//iiif.io/api/image/2/level2.json"),
  );
  return $info;
}