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
  switch ($conf['type']) {
    case 0:
      $title = t('Decathlon DPI Form');
      break;
    case 1: 
      $title = t('Not In Use');
      break;
  }

  $block = new stdClass;
  $block->title = $title;
  return $block;
}

function decathlon_dpi_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('DPI Form'),
      1 => t('Not In Use'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Option'),
  );

  return $form;
}

function decathlon_dpi_render($subtype, $conf, $args, $context) {
  module_load_include('inc', 'decathlon_dpi');
  
  switch ($conf['type']) {
    case 0:
      $content = drupal_get_form('decathlon_dpi_form', $args);
      break;
    case 1:
      $content = "";
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

function decathlon_dpi_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
} 

