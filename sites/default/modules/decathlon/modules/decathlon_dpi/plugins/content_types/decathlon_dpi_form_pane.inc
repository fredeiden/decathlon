<?php

$plugin = array(
  'title' => t('Decathlon DPI'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'edit form' => 'decathlon_dpi_edit_form',
  'render callback' => 'decathlon_dpi_render',
  'admin info' => 'decathlon_dpi_admin_info',
);

/**
 * Display title of selected block on panel page
 */
function decathlon_dpi_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 0:
        $title = 'DPI Admin Form';
        break;
      case 1:
        $title = 'DPI Weight Table';
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

/**
 * Option list for panel pane edit window
 */
function decathlon_dpi_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('DPI Admin Form'),
      1 => t('DPI Weight Table'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('DPI Form or Table'),
  );

  return $form;
}

/**
 * Submit for edit_form
 */
function decathlon_dpi_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
} 

/**
 * Router for selected option
 */
function decathlon_dpi_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon_dpi');

  switch ($conf['type']) {
    case 0:
      $content = drupal_get_form('decathlon_dpi_form', $args);
      break;
    case 1:
      $content = decathlon_dpi_weight_table();
      break;
  }
  
  $block = new stdClass();
  $block->content = $content;
  return $block;
}

/**
 * Form to modify game weights
 */
function decathlon_dpi_form($form, &$form_state) {

  module_load_include('inc', 'decathlon');
  $event = decathlon_get_event();

  // Point multipliers
  $form['multipliers'] = array(
    '#prefix' => '<div class="dpi-form-table dpi-form-multipliers">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#title' => decathlon_get_term_name($event) . ' Point Multipliers',
    '#theme' => 'decathlon_dpi_weights'
  );

  $point_multipliers = decathlon_dpi_get_multipliers('point');
  foreach ($point_multipliers as $tid => $row) {
    $form['multipliers'][$tid] = array(
      '#type' => 'textfield',
      '#title' => $row['name'],
      '#title_display' => 'invisible',
      '#size' => 5,
      '#default_value' => $row['value'],
    );
  }

  // Bonus multipliers
  $form['bonuses'] = array(
    '#prefix' => '<div class="dpi-form-table dpi-form-multipliers">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#title' => decathlon_get_term_name($event) . ' Bonus Multipliers',
    '#theme' => 'decathlon_dpi_weights'
  );

  $bonus_multipliers = decathlon_dpi_get_multipliers('bonus');
  foreach ($bonus_multipliers as $tid => $row) {
    $form['bonuses'][$tid] = array(
      '#type' => 'textfield',
      '#title' => $row['name'],
      '#title_display' => 'invisible',
      '#size' => 5,
      '#default_value' => $row['value'],
    );
  }

  // Participaton weights
  $form['weights'] = array(
    '#prefix' => '<div class="dpi-form-table dpi-form-chart">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#title' =>  decathlon_get_term_name($event) . ' Game Weights',
    '#theme' => 'decathlon_dpi_weights'
  );

  $game_multipliers = decathlon_dpi_get_multipliers('game');
  foreach ($game_multipliers as $tid => $row) {
    $form['weights'][$tid] = array(
      '#type' => 'textfield',
      '#title' => $row['name'],
      '#title_display' => 'invisible',
      '#default_value' => $row['value'],
      '#size' => 4,
    );
  }

  // DPI Chart
  $form['chart'] = array(
    '#prefix' => '<div class="dpi-form-table dpi-form-chart">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#title' => decathlon_get_term_name($event) . ' Ranking Weights',
    '#theme' => 'decathlon_dpi_weights'
  );

  $ranking_multipliers = decathlon_dpi_get_multipliers('ranking');
  foreach ($ranking_multipliers as $tid => $row) {
    $form['chart'][$tid] = array(
      '#type' => 'textfield',
      '#title' => $row['name'],
      '#title_display' => 'invisible',
      '#default_value' => $row['value'],
      '#size' => 4,
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Update',
  );

  return $form;
}

function decathlon_dpi_form_submit($form, &$form_state) {

  // Multipliers
  foreach (element_children($form['multipliers']) as $tid) {
    if ($form['multipliers'][$tid]['#default_value'] != $form_state['values']['multipliers'][$tid]) {
      decathlon_dpi_set_multiplier($tid, $form_state['values']['multipliers'][$tid]);
    }
  }

  // Bonuses
  foreach (element_children($form['bonuses']) as $tid) {
    if ($form['bonuses'][$tid]['#default_value'] != $form_state['values']['bonuses'][$tid]) {
      decathlon_dpi_set_multiplier($tid, $form_state['values']['bonuses'][$tid]);
    }
  }

  // Participation weights
  foreach (element_children($form['weights']) as $tid) {
    if ($form['weights'][$tid]['#default_value'] != $form_state['values']['weights'][$tid]) {
      decathlon_dpi_set_multiplier($tid, $form_state['values']['weights'][$tid]);
    }
  }

  // Ranking weights
  foreach (element_children($form['chart']) as $tid) {
    if ($form['chart'][$tid]['#default_value'] != $form_state['values']['chart'][$tid]) {
      decathlon_dpi_set_multiplier($tid, $form_state['values']['chart'][$tid]);
    }
  }

  drupal_set_message("Changes saved.");
}

function decathlon_dpi_theme($existing, $type, $theme, $path) {
  return array(
    'decathlon_dpi_multipliers' => array(
      'render element' => 'element'
    ),
    'decathlon_dpi_weights' => array(
      'render element' => 'element'
    ),
    'decathlon_dpi_tiers' => array(
      'render element' => 'element'
    ),
  );
}

function theme_decathlon_dpi_weights($vars) {

  $element = $vars['element'];

  $header = array();
  $row = array();
  foreach (element_children($element) as $key) {
    $header[] = $element[$key]['#title'];
    $row[] = array('data' => drupal_render($element[$key]));
  }

  return theme('table', array(
      'header' => $header,
      'rows' => array($row),
      'caption' => $element['#title'],
      'sticky' => FALSE,
    ));
}

/**
 *  Table of game weights specific to the current event
 */
function decathlon_dpi_weight_table() {

  module_load_include('inc', 'decathlon');

  // list of games for this event
  $games = decathlon_get_event_games();
  asort($games);

  // table rows
  $rows = array();

  // row columns: name and weight
  foreach ($games as $tid => $name) {
    $cols = array(
      $name,
      decathlon_dpi_get_participation_weighting($tid),
    );
    $rows[] = $cols;
  }

  // table array
  return theme('table', array(
      'rows' => $rows,
      'sticky' => FALSE,
    ));
}

