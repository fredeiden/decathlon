<?php

function decathlon_drush_command() {

    $items = array();	 

    $items['cache-decathlon-results'] = array(
        'description' => t('Write results cache'),
        'aliases' => array('dcr'),
    );

    return $items;
}

function drush_decathlon_cache_decathlon_results() {
  drush_print("Reset cache.");
}