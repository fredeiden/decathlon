<?php

$plugin = array(
  'title' => t('Decathlon DPI'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'edit form' => 'decathlon_dpi_edit_form',
  'render callback' => 'decathlon_dpi_render',
  'admin form' => 'decathlon_dpi_admin_form',
);

function decathlon_dpi_admin_form($subtype, $conf, $contexts) {

  $title = t('Decathlon DPI Form');
  $block = new stdClass;
  $block->title = $title;
  return $block;
}

function decathlon_dpi_edit_form($form, &$form_state) {

  return $form;
}

function decathlon_dpi_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon_dpi');
  
  $content = drupal_get_form('decathlon_dpi_form', $args);
  $block = new stdClass();
  $block->content = $content;
  return $block;
}

function decathlon_dpi_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
} 

