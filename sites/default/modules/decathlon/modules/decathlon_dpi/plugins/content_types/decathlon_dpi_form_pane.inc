<?php

$plugin = array(
  'title' => t('Decathlon DPI Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'edit form' => 'decathlon_dpi_edit_form',
  'render callback' => 'decathlon_dpi_render',
);

function decathlon_dpi_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_dpi_form', $args);
  return $block;
}

function decathlon_dpi_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  return $form;
}

function decathlon_dpi_edit_form_submit($form, &$form_state) {
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
} 

