<?php

$plugin = array(
  'title' => t('Decathlon Ticker Ticker'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_ticker_ticker_render',
);

function decathlon_ticker_ticker_render($subtype, $conf, $args, $context) {

  libraries_load('liscroll');
  drupal_add_js(drupal_get_path('module', 'decathlon_ticker') . '/decathlon_ticker.js');

  $plist = decathlon_ticker_get_permanent_tickers();
  shuffle($plist);
  //$tlist = decathlon_ticker_get_temporary_tickers();

  $ul = theme('item_list', array('items' => $plist, 'type' => 'ul', 'attributes' => array('id' => 'ticker')));

  $block = new stdClass();
  $block->content = $ul;
  return $block;
}
