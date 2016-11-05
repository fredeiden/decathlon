<?php

$plugin = array(
  'title' => t('Decathlon Scoring Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'edit form' => 'decathlon_scoring_pane_edit_form',
  'render callback' => 'decathlon_scoring_pane_content_type_render',
);

function decathlon_scoring_pane_content_type_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_scoring_form', $args);
  return $block;
}

function decathlon_scoring_pane_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  return $form;
}

function decathlon_scoring_pane_edit_form_submit($form, &$form_state) {
  foreach (array_keys($form_state['plugin']['defaults']) as $key) {
    if (isset($form_state['values'][$key])) {
      $form_state['conf'][$key] = $form_state['values'][$key];
    }
  }
} 
