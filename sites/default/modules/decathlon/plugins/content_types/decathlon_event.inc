<?php

$plugin = array(
  'title' => t('Decathlon Event'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_event_render',
  'edit form' => 'decathlon_event_edit_form',
  'admin info' => 'decathlon_event_admin_info',
);

function decathlon_event_admin_info($subtype, $conf, $contexts) {
  switch ($conf['type']) {
    case 0:
      $title = t('Select Event Form');
      break;
    case 1: 
      $title = t('Add Event Form');
      break;
    case 2:
      $title = t('Delete Event Form');
      break;
  }

  $block = new stdClass;
  $block->title = $title;
  return $block;
}

function decathlon_event_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');

  switch ($conf['type']) {
    case 0:
      $content = drupal_get_form('decathlon_event_select_event_form');
      break;
    case 1:
      $content = drupal_get_form('decathlon_event_add_event_form');
      break;
    case 2:
      $content = drupal_get_form('decathlon_event_delete_event_form');
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

function decathlon_event_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('Select Event Form'),
      1 => t('Add Event Form'),
      2 => t('Delete Event Form'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Options'),
  );

  return $form;
}

function decathlon_event_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

/**
 * Form for setting session variable for event
 */
function decathlon_event_select_event_form($form, &$form_state) {

  $events = decathlon_event_get_events();

  $form['event'] = array(
    '#type' => 'select',
    '#options' => $events,
    '#default_value' => decathlon_get_event(),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Change'),
  );

  $form['status'] = array(
    '#type' => 'markup',
    '#prefix' => '<div id="status">',
    '#suffix' => '</div>',
  );

  return $form;
}

/**
 * Submit for form
 */
function decathlon_event_select_event_form_submit($form, &$form_state) {
  decathlon_set_event($form_state['values']['event']);
  $form_state['redirect'] = '';
}

/**
 *
 */
function decathlon_event_add_event_form($form, &$form_state) {

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

/**
 *
 */
function decathlon_event_add_event_form_submit($form, &$form_state) {
  $form_state['redirect'] = array(
    'admin/structure/taxonomy/event/add',
    array(
      'query' => array(
        'destination' => 'decathlon/manage-events'
      ),
    ),
  );
}

/**
 *
 */
function decathlon_event_delete_event_form($form, &$form_state) {

  $form['events'] = array(
    '#type' => 'select',
    '#options' => decathlon_event_get_events(),
    '#required' => TRUE,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Disabled'),
    '#disabled' => TRUE,
  );

  return $form;
}

/**
 *
 */
function decathlon_event_delete_event_form_submit($form, &$form_state) {

}

/**
 *
 */
function decathlon_event_get_events() {
  $v = taxonomy_vocabulary_machine_name_load('Event');
  $tree = taxonomy_get_tree($v->vid);

  $events = array();
  foreach ($tree as $term) {
    $events[$term->tid] = $term->name;
  }

  return $events;
}