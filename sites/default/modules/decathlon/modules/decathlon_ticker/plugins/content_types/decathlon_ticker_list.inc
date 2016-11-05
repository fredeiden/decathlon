<?php

$plugin = array(
  'title' => t('Decathlon Ticker List'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_ticker_list_render',
  'edit form' => 'decathlon_ticker_list_edit_form',
);

function decathlon_ticker_list_render($subtype, $conf, $args, $context) {

  $list = decathlon_ticker_get_permanent_tickers();

  $block = new stdClass();
  $block->content = theme('item_list', array('items' => $list, 'type' => 'ul'));
  return $block;
}

function decathlon_ticker_list_edit_form($form, &$form_state) {
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(0 => t('Permanent'), 1 => t('Temporary')),
    '#default_value' => 0,
    '#title' => t('List Type'),
  );

  return $form;
}

function decathlon_ticker_list_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}
