<?php

$plugin = array(
  'title' => t('Decathlon Special Scoring Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_scoring_form_special_render',
);

function decathlon_scoring_form_special_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon_scoring');
  module_load_include('inc', 'decathlon');

  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_scoring_form_special_form', $args);
  return $block;
}

function decathlon_scoring_form_special_form($form, &$form_state, $args) {

  $form['#attached'] = array(
    'js' => array(
      drupal_get_path('module', 'decathlon_scoring') . '/plugins/content_types/decathlon_scoring_form_special.js',
    ),
    'library' => array(
      array('system', 'ui.button'),
    ),
  );

  // We add 2 keys to form_state ("step" and "state") to keep track
  // of state in a multistep form.
  $step = isset($form_state['step'])? ($form_state['step']) : (1);
  $form['heading'] = array(
    '#type' => 'markup',
        '#markup' => "<h3>Step #${step}</h3>"
  );
    
  switch ($step) {
    case 2:
      // Render form elements for step #2
      
      $selected_players = $form_state['state']['players']; /* retreive value from step #1 */
      $options = array(
        1 => 'First',
        2 => 'Second',
        3 => 'Third',
        4 => 'Fourth',
        5 => 'Fifth',
        6 => 'Sixth',
        7 => 'Seventh',
        8 => 'Eighth',
        9 => 'Ninth',
        10 => 'Finisher',
      );

      $form['competitor'] = array(
        '#type' => 'container',
        '#tree' => TRUE,
      );

      foreach ($selected_players as $tid) {
        $form['competitor'][$tid] = array(
          '#type' => 'radios',
          '#title' => decathlon_get_term_name($tid),
          '#options' => $options,
          '#default_value' => 10,
          '#prefix' => '<div class="buttonset scoring-column">',
          '#suffix' => '</div>',
        );
      }

      $form['submit'] = array('#type' => 'submit', '#value' => 'Submit');
      break;
  
    case 1:
    default:
      $view = views_get_view_result('players', 'panel_pane_2');
      $players = array();
      foreach ($view as $row) {
        $players[$row->tid] = $row->taxonomy_term_data_name;
      }
      $form['players'] = array(
        '#type' => 'checkboxes',
        '#options' => $players,
      );
      $form['next'] = array('#type' => 'submit', '#value' => 'Next');
      break;
  }

  return $form;
}

function decathlon_scoring_form_special_form_submit($form, &$form_state) {

  $step = isset($form_state['step'])? ($form_state['step']) : (1);
    
  $values =& $form_state['values'];
  switch ($step) {
    case 2:
    {
      $gid = $form_state['build_info']['args'][0][0]; // game term id

      // new entry in contests
      $contest = decathlon_scoring_add_contest($gid);  

      $results = array();
      foreach ($values['competitor'] as $player => $placing) { // loop over saved scores
        decathlon_scoring_add_score($contest, $player, $placing); // save score
        $results[$player]['score'] = $placing; // populate results array
      }
    }

    // Run calc function
    $notes = decathlon_scoring_score_game(decathlon_get_event(), $contest, $gid, $results);
    decathlon_scoring_add_contest_notes($contest, $notes);
    
    $form_state['values']['contest'] = $contest;
    $form_state['redirect'] = 'scores/game/' . $contest;

    break;

    case 1:
    default:
    {
      // Prepare for next step 
      $form_state['step'] = 2;
      $players = array();
      foreach ($values['players'] as $tid) {
        if ($tid != 0) {
          $players[] = $tid;
        }
      }
      $form_state['state'] = array('players' => $players);
      $form_state['rebuild'] = true; /* required in order to rebuild form in each step!! */
    }
    break;
  }
}
