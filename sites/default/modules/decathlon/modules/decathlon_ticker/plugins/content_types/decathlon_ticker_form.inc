<?php

$plugin = array(
  'title' => t('Decathlon Ticker Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_ticker_form_render',
  'edit form' => 'decathlon_ticker_form_edit_form',
);

function decathlon_ticker_form_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_ticker_form');
  return $block;
}

function decathlon_ticker_form_edit_form($form, &$form_state) {

  // no new fields

  return $form;
}

