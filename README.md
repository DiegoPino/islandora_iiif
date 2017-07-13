# Islandora IIIF

Our super drop-in and forget kind-of IIIF Image server URL proxying and IIIF Presentation API 2.1.x types generator for Islandora
This is still experimental and I (Diego Pino) don't recommend running it in production. 
Still, it is very cool =)

## Requirements

This module requires the following modules/libraries:

* [Islandora](https://github.com/islandora/islandora)
* [Islandora Collection](https://github.com/islandora/islandora_solution_pack_collection)
* [Islandora Solr Search ](https://github.com/islandora/islandora_solr_search)
* [Islandora Solr Metadatata ](https://github.com/Islandora/islandora_solr_metadata)

This module also requires a working Adore-Djatoka installation, which is the default if you have a sane 
running and Extremely normal version of the Islandora stack and you are already running OpenSeadragon as viewer

This module does not require (kind-of really) but recommends having also installed
* [Islandora Paged ](https://github.com/islandora/islandora_paged)

## Installation

Install as usual, see [this](https://drupal.org/documentation/install/modules-themes/modules-7) for further information.


## Documentation

This module wraps around adore-djatoka image server and Islandora Datastream authentication/authorization translating well formed 
IIIF Image API calls internally to something that Adore understands, returning the transformed image. This allows any generic Islandora
Installation run and serve IIIF compliant image responses without any extra backend deployment with the added benefit of Drupal 
(and varnish if you have that!) Caching, datastream access secured via Islandora and even extra capabilities provided by image magick

@see http://iiif.io/api/presentation/2.1/

http://localhost:8000/islandora_iiif/islandora%3A95/full/max/0/default.jp2

Also, but not lastly, it provides responses for all IIIF Presentation API 2.1 types

In the form of:

http://localhost:8000/islandora_iiif/islandora%3A1/info.json
http://localhost:8000/islandora_iiif/islandora%3A1/manifest
http://localhost:8000/islandora_iiif/islandora%3A1/sequence/default
http://localhost:8000/islandora_iiif/islandora%3A1/canvas
http://localhost:8000/islandora_iiif/islandora%3A1/collection