<?php

$plugin = array(
  'title' => t('Decathlon Standard Scoring Form'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_scoring_form_standard_render',
);

function decathlon_scoring_form_standard_render($subtype, $conf, $args, $context) {

  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_scoring_form_standard_form', $args);
  return $block;
}

/**
 * Form
 */
function decathlon_scoring_form_standard_form($form, &$form_state, $arg) {

  form_load_include($form_state, 'inc', 'decathlon', 'modules/decathlon_scoring/decathlon_scoring');
  form_load_include($form_state, 'inc', 'decathlon', 'modules/decathlon_scoring/plugins/content_types/decathlon_scoring_form_standard');
  form_load_include($form_state, 'inc', 'decathlon', 'decathlon');

  $tid = $arg[0];
  $game = taxonomy_term_load($tid);
  $format = taxonomy_term_load($game->field_format['und'][0]['tid']);
  $scoring_format = taxonomy_term_load($game->field_scoring_format['und'][0]['tid']);
  $honors = views_get_view_result('honors_list', 'panel_pane_1', $tid);

  // Map to describe what choices to offer for number of players in the game.  The index number says
  // how many player-score-honors groups to present.  The value is simply the name and only shows up on the form item.
  // Should this be in the database?  This is the only place that it is used and
  // it is much simpler to just have it here.
  $layout_map = array(
    206 => array('2' => '2 Players'),
    235 => array('2' => '2 Players', '4' => '4 Players'),
    207 => array('3' => '3 Players', '4' => '4 Players'),
    209 => array('2' => '2 Players', '3' => '3 Players', '4' => '4 Players'),
    339 => array('4' => '4 Players'),
    204 => array('2' => 'Singles', '5' => 'Doubles'),
    205 => array('5' => 'Doubles'),
  );
  $layout = $layout_map[$format->tid];

  // Add some jquery stuff
  $form['#attached'] = array(
    'js' => array(
      drupal_get_path('module', 'decathlon_scoring') . '/decathlon_scoring.js',
    ),
    'library' => array(
      array('system', 'ui.button'),
    ),
  );

  // At the top of the form present layout options
  $form['layout'] = array(
    '#type' => 'radios',
    '#required' => TRUE,
    '#ajax' => array(
      'callback' => '_scoring_form_select_layout',
      'wrapper' => 'scores',
      'effect' => 'slide',
    ),
    '#prefix' => '<div class="layout buttonset">',
    '#suffix' => '</div>',
    '#options' => $layout,
    '#default_value' => key($layout),
  );

  // Store the game tid
  $form['game'] = array(
    '#type' => 'hidden',
    '#value' => $tid,
  );

  // On the rest of the form, render fields to store player, score, and honors and
  // render either 2, 3, 4, or 2 sets of 2.
  // Which one is determined either from $form_state value or default of first one in the list
  $layout_key = !empty($form_state['values']['layout']) ? $form_state['values']['layout'] : key($layout); // key returns current pointer key

  // *** AJAX **********************************************************
  // *** This section of the form is replaced by the ajax call above ***

  // outer wrapper
  $form['scores'] = array(
    '#type' => 'fieldset',
    '#prefix' => '<div id="scores">',
    '#suffix' => '</div>',
    '#tree' => TRUE,
    '#attributes' => array('class' => array('no-border')),
  );

  if ($layout_key == 5) { // placeholder for doubles (2 sets of 2)
    $limit = 2;
  } else {
    $limit = $layout_key;
  }
  for ($i = 1; $i <= $limit; $i++) {
    // wrapper around each set
    $form['scores'][$i] = array(
      '#type' => 'fieldset',
      '#prefix' => '<div class="scoring-group">',
      '#suffix' => '</div>',
    );

    // player
    $form['scores'][$i]['player'] = array(
      '#type' => 'fieldset',
      '#attributes' => array('class' => array('no-border', 'players')),
    );
    $form['scores'][$i]['player'][0] = _add_player();
    if ($layout_key == 5) { // placeholder for doubles (2 sets of 2)
      $form['scores'][$i]['player'][1] = _add_player();
    }

    // score
    // different formats depending on the game
    if ($scoring_format->name == 'Games Won') {
      // games won radios
      $form['scores'][$i]['score'] = _add_wins();
    } elseif ($scoring_format->name == 'Placing') {
      // placing radios
      $form['scores'][$i]['score'] = _add_placing($limit, $i);
    } elseif ($scoring_format->name == 'Tiles Remaining') {
      // score number input
      $form['scores'][$i]['score'] = _add_score($game->field_min_score['und'][0]['value'], $game->field_max_score['und'][0]['value'], 'Tiles Remaining');
    } else {
      // score number input
      $form['scores'][$i]['score'] = _add_score($game->field_min_score['und'][0]['value'], $game->field_max_score['und'][0]['value']);
    }

    // skunk notification
    if ($game->field_skunk_point['und'][0]['value'] == 1) {
      $form['scores'][$i]['skunk'] = array(
        '#type' => 'item',
        '#markup' => 'Skunk point awarded automatically at ' . $game->field_skunk_cutoff['und'][0]['value'] . '-0.',
        '#prefix' => '<div class="scoring-note">',
        '#suffix' => '</div>',
      );
    }

    // honors
    if (count($honors) > 0) {
      foreach($honors as $num => $row) {
        $honor = taxonomy_term_load($row->tid);
        if ($honor->field_auto['und'][0]['value'] == 1) {
          $form['scores'][$i]['honors'][$num] = _add_honors($honor->name, $honor->field_max_per_game['und'][0]['value'], $row->tid, TRUE);
        } else {
          $form['scores'][$i]['honors'][$num] = _add_honors($honor->name, $honor->field_max_per_game['und'][0]['value'], $row->tid);
        }
      }
    }
  }

  // *** End of ajax section ***
  // *** AJAX **********************************************************

  $form['actions'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('class' => array('no-border')),
  );
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
    '#prefix' => '<div class="custom-submit">',
    '#suffix' => '</div>',
  );

  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => 'Cancel',
    '#limit_validation_errors' => array(),       // No validation
    '#prefix' => '<div class="custom-submit">',
    '#suffix' => '</div>',
    '#submit' => array('decathlon_scoring_form_standard_form_cancel'),
  );

  return $form;
}

function _add_player() {
  $item = array(
    array(
      '#type' => 'item',
      '#markup' => '<div class="player-image"></div>',
    ),
    array(
      '#type' => 'hidden',
      '#element_validate' => array('_player_validate'),
    ),
  );
  return $item;
}

function _player_validate($element, &$form_state, $form) {
  if (empty($element['#value'])) {
    form_error($element, t('Fill in all player boxes.'));
  }
}

function _add_score($min = -1, $max = 500, $title = 'Score') {
  return array(
    '#type' => 'numberfield',
    '#title' => $title,
    '#attributes' => array('min' => $min, 'max' => $max, 'required' => ''),
    '#element_validate' => array('_score_validate'),
    '#prefix' => '<div class="scoring-column">',
    '#suffix' => '</div>',
  );
}

function _score_validate($element, &$form_state, $form) {
  if (!is_numeric($element['#value'])) {
    form_error($element, t('Please enter all numeric scores.'));
  }
}

function _add_placing($limit, $default) {
  $options = array(
    1 => 'First',
    2 => 'Second',
    3 => 'Third',
    4 => 'Fourth',
  );
  for ($i = 4; $i > $limit; $i--) {
    unset($options[$i]);
  }
  return array(
    '#type' => 'radios',
    '#title' => 'Placing',
    '#options' => $options,
    '#default_value' => $default,
    '#prefix' => '<div class="placing buttonset scoring-column">',
    '#suffix' => '</div>',
  );
}

function _add_wins() {
  $options = array(
    0 => 'None',
    1 => 'One',
    2 => 'Two',
  );
  return array(
    '#type' => 'radios',
    '#title' => 'Games Won',
    '#options' => $options,
    '#default_value' => 0,
    '#prefix' => '<div class="wins buttonset scoring-column">',
    '#suffix' => '</div>',
  );
}

function _add_honors($title, $max, $tid, $auto = FALSE) {

  $item = array();

  // store this for use in submit
  $item['tid'] = array(
    '#type' => 'hidden',
    '#value' => $tid,
  );

  if ($auto == TRUE) {
    $item['auto_honor'] = array(
      '#type' => 'item',
      '#title' => t('Honor awarded automatically'),
    );
    $item['auto'] = array(
      '#type' => 'hidden',
      '#value' => TRUE,
    );
  } else {
    $options = array();
    for ($i = 0; $i <= $max; $i++) {
      $options[$i] = $i;
    }
    $item['count'] = array(
      '#type' => 'radios',
      '#title' => $title,
      '#options' => $options,
      '#default_value' => 0,
      '#prefix' => '<div class="honors buttonset scoring-column">',
      '#suffix' => '</div>',
    );
  }

  return $item;
}

function _scoring_form_select_layout($form, $form_state) {
  return $form['scores'];
}

function decathlon_scoring_form_standard_form_cancel() {
  $args = explode('/', current_path());
  drupal_goto($args[0]);
}

/**
 * Validate form
 */
function decathlon_scoring_form_standard_form_validate($form, &$form_state) {

  $game_tid = $form_state['values']['game'];
  $game = taxonomy_term_load($game_tid);

  // Backgammon (maybe create a taxonomy field to avoid this hard coding?)
  if ($game_tid == 10) {
    $check_list = array("02", "12", "20", "21");
    $check_string = "";
    foreach ($form_state['values']['scores'] as $set) {
      $check_string .= $set['score'];
    }
    if (! in_array($check_string, $check_list)) {
      form_set_error('scores', "Results should be 2-0 or 2-1.");
    }
  }

  // Skunks
  if ($game_tid == 31) {
    // Shuffleboard is too tricky to validate because of different table and floor rules
  } elseif ($game->field_skunk_point['und'][0]['value'] == 1) {
    $skunk_cutoff = $game->field_skunk_cutoff['und'][0]['value'];
    $skunk = FALSE;
    foreach ($form_state['values']['scores'] as $set) {
      if ($set['score'] == 0) {
        $skunk = TRUE;
      } else {
        $opponent_score = $set['score'];
      }
    }
    if ($skunk == TRUE) {
      if ($skunk_cutoff == 10) { // 10, used in horseshoes and shuffleboard, is a min
        if ($opponent_score < $skunk_cutoff) {
          form_set_error('scores', "Skunk should be >= " . $skunk_cutoff . "-0.");
        }
      } elseif ($opponent_score != $skunk_cutoff) {
        form_set_error('scores', "Skunk should be " . $skunk_cutoff . "-0.");
      }
    }
  }
}

/**
 * Submit form
 */
function decathlon_scoring_form_standard_form_submit($form, &$form_state) {

  // new entry in contests
  $format = $form_state['values']['layout'] == 5 ? 1 : 0; // doubles is 5
  $contest = decathlon_scoring_add_contest($form_state['build_info']['args'][0][0], $format);  // passing game tid

  // build array for point calculations
  $results = array();

  // scores are saved in form_state array
  foreach ($form_state['values']['scores'] as $set) {

    // loop over the player array and add scores and honors for player(s)
    // there will be two players for a doubles round and both are assigned the same results
    foreach ($set['player'] as $player) {

      // scores
      decathlon_scoring_add_score($contest, $player[1], $set['score']);
      $results[$player[1]]['score'] = $set['score'];

      // honors
      if (isset($set['honors'])) {
        foreach ($set['honors'] as $honor) {
          if (isset($honor['auto']) && $honor['auto'] == TRUE) {
            if ($set['score'] == 0) {
              decathlon_scoring_add_honor($contest, $player[1], $honor['tid'], 1);
              $results[$player[1]]['honors'] = 1;
            }
          } elseif (isset($honor['count']) && $honor['count'] > 0) {
            decathlon_scoring_add_honor($contest, $player[1], $honor['tid'], $honor['count']);
            $award = decathlon_get_honors_award($honor['tid'], $honor['count']);
            if ($award > 0) { $results[$player[1]]['honors'] = $award; }
          }
        }
      }
    }
  }

  // Run calc function
  $notes = decathlon_scoring_score_game(decathlon_get_event(), $contest, $form['game']['#value'], $results);
  decathlon_scoring_add_contest_notes($contest, $notes);

  $form_state['values']['contest'] = $contest;
  $form_state['redirect'] = 'scores/game/' . $contest;
}

