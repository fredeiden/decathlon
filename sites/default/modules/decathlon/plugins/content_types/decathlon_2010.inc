<?php

$plugin = array(
  'title' => t('Decathlon 2010'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_2010_render',
  'admin info' => 'decathlon_2010_admin_info',
);

function decathlon_2010_admin_info($subtype, $conf, $contexts) {
  $block = new stdClass;
  $block->title = 'Decathlon 2010 Upload';
  return $block;
}

function decathlon_2010_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');
  module_load_include('inc', 'decathlon_scoring');

  $block = new stdClass();
  $block->content = drupal_get_form('decathlon_2010_upload_form');
  return $block;
}

function decathlon_2010_upload_form($form, &$form_state) {

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Add'),
  );

  return $form;
}

function decathlon_2010_upload_form_submit($form, &$form_state) {
  //decathlon_2010_record_scores();
  decathlon_2010_record_bonuses();
}


function decathlon_2010_record_scores() {

  db_set_active('extra');
  $results = db_query("SELECT match_id, player_id, score, bonus_points FROM decathlon_individualmatchresult")->fetchAll();
  $match_map = db_query("SELECT id, event_id FROM decathlon_match")->fetchAllKeyed();
  db_set_active();

  $scores = array();
  foreach ($results as $row) {
    $pid = decathlon_2010_user_map($row->player_id);
    $scores[$row->match_id][$pid]['score'] = $row->score;
    $scores[$row->match_id][$pid]['honor'] = $row->bonus_points;
  }


  foreach ($scores as $match_id => $match_data) {

    $results = array();
    $gid = decathlon_2010_game_map($match_map[$match_id]);
    $contest = decathlon_scoring_add_contest($gid);
    
    foreach ($match_data as $pid => $set) {

      // scores
      decathlon_scoring_add_score($contest, $pid, $set['score']);
      $results[$pid]['score'] = $set['score'];

      // honors
      if ($set['honor'] > 0) {
        if ($gid == 30) { // Foosball
          // skunk point, no action
        } else {
          if ($gid == 27) { // Horseshoes = 5 ringers to score
            $count = 5;
          } else {
            $count = $set['honor'];
          }
          decathlon_scoring_add_honor($contest, $pid, decathlon_2010_honor_map($match_map[$match_id]), $count);
          $results[$pid]['honors'] = $set['honor'];
        }
      }
    }

    // Run calc function
    $notes = decathlon_scoring_score_game(decathlon_get_event(), $contest, $gid, $results);
    decathlon_scoring_add_contest_notes($contest, $notes);
  }
}

function decathlon_2010_record_bonuses() {

  db_set_active('extra');
  $results = db_query("SELECT id, player_id, bonus_type_id, points, notes FROM decathlon_bonus")->fetchAll();
  db_set_active();

  $nodes = array();

  foreach ($results as $row) {
    $node = new stdClass();
    $node->type = 'bonus';
    $node->language = LANGUAGE_NONE;
    node_object_prepare($node);

    $node->uid = 1;
    $node->field_decathlon[LANGUAGE_NONE][0]['tid'] = decathlon_get_event();
    $node->field_contestant[LANGUAGE_NONE][0]['tid'] = decathlon_2010_user_map($row->player_id);
    $node->field_bonus[LANGUAGE_NONE][0]['tid'] = decathlon_2010_bonus_map($row->bonus_type_id);
    $node->field_score[LANGUAGE_NONE][0]['value'] = $row->points;
    $node->field_comments[LANGUAGE_NONE][0]['value'] = $row->notes;

    node_save($node);

    $nodes[] = $node->nid;
  }

  drupal_set_message("Added bonus nodes: " . implode(",", $nodes));
}


function decathlon_2010_user_map($id) {
  $user_map = array(
    1 => 106,
    2 => 105,
    3 => 94,
    4 => 93,
    5 => 170,
    6 => 169,
    7 => 95,
    8 => 192,
    9 => 117,
    10 => 118,
    11 => 85,
    12 => 86,
    13 => 115,
    14 => 116,
    15 => 78,
    16 => 81,
    17 => 82,
    18 => 187,
    19 => 325,
    20 => 326,
    21 => 327,
    22 => 329,
    23 => 330,
    24 => 331,
    25 => 332,
    26 => 96,
    27 => 178,
    28 => 102,
    29 => 333,
    30 => 334,
    31 => 335,
  );
  return $user_map[$id];
}

function decathlon_2010_game_map($id) {
  $game_map = array(
    1 => 30,  // Foosball
    2 => 29,  // Bocce 
    3 => 28,  // Disc
    4 => 27,  // Horseshoes
    5 => 31,  // Shuffleboard
    6 => 11,  // Hearts
    7 => 12,  // Scrabble
    8 => 8,   // Blokus 
    9 => 10,  // Backgammon
    10 => 308,// Boggle
  );
  return $game_map[$id];
}

function decathlon_2010_honor_map($id) {
  $honor_map = array(
    2 => 61,  // Bocce 
    3 => 60,  // Disc
    4 => 62,  // Horseshoes
    5 => 65,  // Shuffleboard
    6 => 68,  // Hearts
    7 => 66,  // Scrabble
    8 => 71,  // Blokus 
    9 => 69,  // Backgammon
    10 => 312,// Boggle
  );
  return $honor_map[$id];
}

function decathlon_2010_bonus_map($id) {
  $bonus_map = array(
    1 => 58,
    2 => 59,
    3 => 233,
    4 => 233,
    5 => 57,
    6 => 57,
  );
  return $bonus_map[$id];
}