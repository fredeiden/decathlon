<?php

$plugin = array(
  'title' => t('Decathlon Players Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_players_form_render',
  'admin info' => 'decathlon_players_form_admin_info',
);

function decathlon_players_form_admin_info($subtype, $conf, $contexts) {
  $block = new stdClass;
  $block->title = 'Manage Players';
  return $block;
}

function decathlon_players_form_render($subtype, $conf, $args, $context) {
  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_players_form', $args);
  return $block;
}

/**
 * Custom form for players taxonomy fields
 */
function decathlon_players_form($form, &$form_state, $arg) {

  $form = array();

  $event = decathlon_get_event();
  $players = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("players")->vid);
  $events = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("event")->vid);
  $options = array();
  foreach ($events as $row) {
    $options[$row->tid] = $row->name;
  }

  $form['players'] = array(
    '#tree' => TRUE,
  );

  for ($i = 0; $i < count($players); $i++) {
//  for ($i = 0; $i < 5; $i++) {

    // need the taxonomy data
    $term = taxonomy_term_load($players[$i]->tid);
    $active = FALSE;

    // look through field_attandance field for current event
    // if it is set, this player is registered for this decathlon
    $active_defaults = array();
    if (!empty($term->field_active['und'])) {
      foreach ($term->field_active['und'] as $row) {
        $active_defaults[] = $row['tid'];
        if ($row['tid'] == $event) {
          $active = TRUE;
        }
      }
    }

    // fieldset wrapper for this player
    if ($active == TRUE) {
      $class = 'player-details active';
    } else {
      $class = 'player-details inactive';
    }
    $form['players'][$i] = array(
      '#type' => 'fieldset',
      '#attributes' => array('class' => array($class)),
    );

    // carry the taxonomy term id for submit function
    $form['players'][$i]['tid'] = array(
      '#type' => 'hidden',
      '#value' => $players[$i]->tid,
    );

    // show player image
    $form['players'][$i]['image'] = array(
//      '#type' => 'managed_file',
//      '#default_value' => !empty($term->field_picture['und'][0]['fid']) ? $term->field_picture['und'][0]['fid'] : 0,
      '#prefix' => '<h3>' . $term->name . '</h3>',
      '#markup' => theme('image_style', array(
                   'style_name' => 'player_small', 
                   'path' => !empty($term->field_picture['und'][0]['uri']) ? $term->field_picture['und'][0]['uri'] : 'public://default_images/yoda.jpg',
                 )),
    );

    // display active/inactive status and allow it to be reset
    $form['players'][$i]['status'] = array(
      '#type' => 'select',
      '#options' => array(
        0 => 'Inactive',
        1 => 'Active',
      ),
      '#default_value' => $active,
    );

  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

/**
 *  Validate for form
 */
function decathlon_players_form_validate($form, &$form_state) {
}

/**
 * Submit for form
 */
function decathlon_players_form_submit($form, &$form_state) {

  foreach (element_children($form['players']) as $index) {

    $status = $form['players'][$index]['status']['#default_value'];
    $tid = $form_state['values']['players'][$index]['tid'];
    $event = decathlon_get_event();

    if ($status != $form_state['values']['players'][$index]['status']) {
      // load players taxonomy term
      $term = taxonomy_term_load($tid);

      // update
      if ($status == 1) { // is set to active; change to inactive
        $remove = array();
        foreach ($term->field_active['und'] as $i => $row) {
          if ($row['tid'] == $event) {
            $remove[] = $i;
          }
        }
        foreach ($remove as $j) {
          unset($term->field_active['und'][$j]);
        }
        // reset array keys
        $term->field_active['und'] = array_values($term->field_active['und']);
      } else { // is set to inactive; change to active
        $term->field_active['und'][]['tid'] = $event;
      }

      // save modified term
      taxonomy_term_save($term);
    }
  }
}

