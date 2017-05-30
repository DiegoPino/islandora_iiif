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

function islandora_iiif_safefile($filename) {
  // @TODO: this should be defined per backend image implementation.
  $allowedExtensions = array('png', 'jpg', 'jp2');
    
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