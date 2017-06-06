<?php
/**
 * Cleans a fake filename for IIIF´s Server API.
 * @param string $filename
 *  A File name as IIIF Server API would expect
 * @param array $allowedExtensionse
 *  An array with allowed extensions
 *
 * @return array
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

  // Note: Adore-djatoka tiles work on 96px multipliers
  $info = array(
    "@context" => "http://iiif.io/api/image/2/context.json",
    "@embed" => "true",
    "protocol" => "http://iiif.io/api/image",
    "width" => (int)$image_info["width"],
    "height" => (int)$image_info["height"],
    "tiles"=> array(
      array("width" => 256, "scaleFactors" => range(1,$image_info['dwtLevels']))
    ),
    "profile" => array("http://iiif.io/api/image/2/level2.json"),
  );
  return $info;
}


function islandora_iiif_djatoka_region($image_info, $iiif_params) {
  // We need to translate basically everything to zoom level + region + rotate.
  $offsets = array();
  $region_dimensions = array();
  

  $match = array();
  $h = (int)$image_info["height"];
  $w = (int)$image_info["width"];
  //Set this by default in case nothing matches
  $region_dimensions = array($w, $h);
  $djatoka_params_translated = array(
    'region' => implode(",", $region_dimensions),
    'rotate' => 0,
    'level' => 0,
  );
  //preventive in case region does not fit anything else:
  // I will precalculate some of the non string options here

  //First Region:

  preg_match('/^([pct:]*)([\d\.]+),([\d\.]+),([\d\.]+),([\d\.]+)$/', $iiif_params['region'], $match);
  if (count($match) == 6) {
    if ($match[1] == "pct:") {
      $x = round(($match[2] / 100) * $w, 0, PHP_ROUND_HALF_DOWN);
      $y = round(($match[3] / 100) * $h, 0, PHP_ROUND_HALF_DOWN);
      $dw = round(($match[4] / 100) * $w, 0, PHP_ROUND_HALF_DOWN);
      $dh = round(($match[5] / 100) * $h, 0, PHP_ROUND_HALF_DOWN);
    } 
    else { 
      $x = (int)$match[2];
      $y = (int)$match[3];
      $dw = (int)$match[4];
      $dh = (int)$match[5];
    }
    $offsets = array($x, $y);
    $region_dimensions = array($dw, $dh); 
  }
  else {
    switch ($iiif_params['region']) {
      case 'full':
        break;
      case 'square':
        $min = min($h, $w);
        $max = max($h, $w);
        $offset = round(($max - $min) / 2, 0, PHP_ROUND_HALF_DOWN);
        $offsets = array($offset, 0);
        $region_dimensions = array($min, $min);
        if ($h >= $w) {
          $offsets = reverse($offsets);
        }    
        break;
    default:
    // Hell breaks loose, nothing in our Level 2 Profile matches
    // @TODO return something i can catch in a parent call
    // Failing argument or format should return a 400

    }
  }
  $match = array();
  //Second Size:
  switch(true) {
    case $iiif_params['size'] == "full" || $iiif_params['size'] == "max":
    break;
    case 1 === preg_match('/^(\d+),$/', $iiif_params['size'], $match):
    $djatoka_params_translated['scale'] = (int)$match[1].",0";
    $level = (int)($region_dimensions[0] || $w) / (int)$match[1];
    $djatoka_params_translated['level'] = $level;
    break;
    case 1 === preg_match('/^,(\d+)$/', $iiif_params['size'], $match):
    $djatoka_params_translated['scale'] = "0,".(int)$match[1];
    $level = (int)($region_dimensions[1] || $h) / (int)$match[1];
    $djatoka_params_translated['level'] = $level;
    break;
    case 1 === preg_match('/^pct:(\d*\.?\d*)$/i', $iiif_params['size'], $match):
    $server_ratio = round($match[1]/100, 1);
    $level = $image_info['dwtLevels'] - round(log($server_ratio, 2),0);
    $level =  $level < 1 ? 1 : $level;
    $reduction = $image_info['dwtLevels'] - $level;
    //Would love to use ** but was introduced in php 5.6
    $djatoka_params_translated['scale'] = ($server_ratio * pow($reduction, 2));
    $djatoka_params_translated['level'] = $level;

    break;
    case 1 === preg_match('/^(\d+),(\d+)$/', $iiif_params['size'], $match):
    $djatoka_params_translated['scale'] = $match[0];
    $level = (int)($region_dimensions[0] || $w) / (int)$match[1];    
    $djatoka_params_translated['level'] = $level;
    break;
    case 1 === preg_match('/^!(\d+),(\d+)$/', $iiif_params['size'], $match):
    $best_scale = min($match[1]/$region_dimensions[0], $match[2]/$region_dimensions[1]);
    $dw = round(ceil($region_dimensions[0] * $best_scale),0);
    $dh = round(ceil($region_dimensions[1] * $best_scale),0);    
    $djatoka_params_translated['scale'] = $dw . "," . $dh;
    $server_ratio = ($region_dimensions[0] / $dw);
    $level = $image_info['dwtLevels'] - round(log($server_ratio, 2),0);
    $level =  $level < 1 ? 1 : $level;
    $djatoka_params_translated['level'] = $level;
    break; 
  }
  // Offset offsets things...jiji
  if (count($offsets) > 0) {
    $exp_reduction = pow($image_info['dwtLevels'] - $djatoka_params_translated['level'], 2);
    $powpow = function($v) use ($exp_reduction) {
      return ($v/(int)$exp_reduction);
    };
    list($dw, $dh) = array_map($powpow, $region_dimensions);
    list($dx, $dy) = $offsets;
    $djatoka_params_translated['region'] = $dy . "," . $dx . "," . $dh . "," . $dw ; 
  }
  
  // Third Rotation
  $rotmod = (int)$iiif_params['rotation'] % 90;
  // This basically converts any degree in chunks of 90 between 0 and 360. 
  // Can't remember last time i had to do this.
  $djatoka_params_translated['rotation'] = ((int)$iiif_params['rotation'] - $rotmod) % 360;
  dpm($djatoka_params_translated);
  return $djatoka_params_translated;
}

function islandora_iiif_djatoka_metadata_request($pid) {
  module_load_include('inc', 'islandora', 'includes/authtokens');
  $uri_params = array(
    'absolute' => TRUE,
    'query' => array( 
      'token' => islandora_get_object_token($pid, "OBJ", 2),
    ),
  );

  $djatoka_metadata_params = array(
    'url_ver' => 'Z39.88-2004',
    'rft_id' =>  url("islandora/object/{$pid}/datastream/OBJ/view", $uri_params),
    'svc_id' => 'info:lanl-repo/svc/getMetadata'
  );
 
   //@TODO Don't like it but will have to do it: Make openseadragon a dependency.
   $settings = islandora_openseadragon_get_settings();
   $djatoka_url = url($settings['djatokaServerBaseURL'], array(
     'absolute' => TRUE,
     'language' => (object)array('language' => FALSE),
     'query' => $djatoka_metadata_params,
   ));
   dpm($djatoka_url);
   return drupal_http_request($djatoka_url);
}

