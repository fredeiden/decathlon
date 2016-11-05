<?php


/**
 *
 */
function decathlon_scoring_score_game($event, $contest, $game, $results) {

  $log_message = "<ul><li>" . date("h:i:s") . "</li>";

  //build a list of taxonomy ids to make display text clearer
  $id_list = array();
  $taxonomy = db_query('select tid, name from taxonomy_term_data');
  foreach ($taxonomy as $row) {
    $id_list[$row->tid] = $row->name;
  }

  //build a list indicating if game winner has highest or lowest score
  $high_score_wins = array();
  $high_score_yes_no = db_query('select entity_id, field_high_score_wins_value from field_data_field_high_score_wins');
  foreach ($high_score_yes_no as $row) {
    $high_score_wins[$row->entity_id] = $row->field_high_score_wins_value;
  }

  //build a list indicating if skunk scores extra point
  $skunk_bonus = array();
  $skunk_yes_no = db_query('select entity_id, field_skunk_point_value from field_data_field_skunk_point');
  foreach ($skunk_yes_no as $row) {
    $skunk_bonus[$row->entity_id] = $row->field_skunk_point_value;
  }

  //$log_message .= "<li>" . date("h:i:s") . " - built lists</li>";

  $game_results = array();
  $skunk = 0;
  foreach ($results as $contestant => $stats) {

    //use a variable to keep track of last score
    $set = 0;

    $score = $stats['score'];

    // record participation
    decathlon_scoring_add_points($contest, $contestant, 73, 1);
    $log_message .= "<li>Counted 1 participation point for $id_list[$contestant]</li>";

    //record honors
    if (isset($stats['honors'])) {
      decathlon_scoring_add_points($contest, $contestant, 77, $stats['honors']);
      $log_message .= "<li>Counted " . $stats['honors'] . " honors for $id_list[$contestant]</li>";
    }

    //number of records so far
    $num_results = count($game_results);

    //note skunk
    if ($score <= 0) { $skunk = 1; }

    if ($num_results > 0) {
      // Look through the existing results and insert new score
      for ($j = 1; $j <= $num_results; $j++) {

        if ($high_score_wins[$game] == 1) {  // High score wins

          if ($score > $game_results[$j]["score"]) {
            for ($k = $num_results + 1; $k > $j; $k--) {
              $game_results[$k]["score"] = $game_results[($k - 1)]["score"];
              $game_results[$k]["name"] = $game_results[($k - 1)]["name"];
            } // ends for
            $game_results[$j]["score"] = $score;
            $game_results[$j]["name"] = $contestant;
            $set = 1;
            break;
          } // ends if

        } else {  // Low score wins

          if ($score < $game_results[$j]["score"]) {
            for ($k = $num_results + 1; $k > $j; $k--) {
              $game_results[$k]["score"] = $game_results[($k - 1)]["score"];
              $game_results[$k]["name"] = $game_results[($k - 1)]["name"];
            } // ends for
            $game_results[$j]["score"] = $score;
            $game_results[$j]["name"] = $contestant;
            $set = 1;
            break;
          } // ends if
          
        } // ends if
      } // ends for

      if ($set == 0) {
        $game_results[$j]["score"] = $score;
        $game_results[$j]["name"] = $contestant;
      }
    } else {
      $game_results[1]["score"] = $score;
      $game_results[1]["name"] = $contestant;
    }
  }

  // Now tally skunks and bestings
  $players = count($results);
  $skunks = array();
  $bestings = array();
  for ($i = 1; $i < $players; $i++) {

    for ($j = ($i + 1); $j <= $players; $j++) {

      $winner = $game_results[$i]['name'];
      $loser = $game_results[$j]['name'];
      $winner_score = $game_results[$i]['score'];
      $loser_score = $game_results[$j]['score'];

      // if scores are equal, this is either a tie game or players are partners
      if ($winner_score == $loser_score) {
        continue;
      }
      
      // count skunk points
      if ($skunk == 1 && $skunk_bonus[$game] == 1) {
        $skunks[$winner] = 1; // this will only count 1 skunk point in doubles
      }

      // look for an existing record of this winner besting this loser in this game
      $result = db_query("SELECT game FROM {decathlon_besting} WHERE event = :event AND game = :game AND bester = :bester AND bestee = :bestee", 
          array(
            ':event' => $event, 
            ':game' => $game, 
            ':bester' => $winner, 
            ':bestee' => $loser
          )
        )->fetchAll();

      if (count($result) > 0) {
        $log_message .= "<li>$id_list[$winner] already bested $id_list[$loser] in a game of $id_list[$game] - no point recorded</li>";
      } else {
        // this is a new besting, so save new marker to prevent future counting
        decathlon_scoring_add_besting($event, $contest, $game, $winner, $loser);
        // and count besting point
        isset($bestings[$winner]) ? $bestings[$winner] += 1 : $bestings[$winner] = 1;
        $log_message .= "<li>$id_list[$winner] gets 1 point for besting $id_list[$loser] in this game of $id_list[$game]</li>";
      }
    }
  }
  // award skunk point
  foreach ($skunks as $winner => $count) {
    decathlon_scoring_add_skunk($contest, $winner);
    decathlon_scoring_add_points($contest, $winner, 76, $count); // 76 is skunk tid
    $log_message .= "<li>Counted 1 skunk point for $id_list[$winner]</li>";
  }
  // award besting points
  foreach ($bestings as $winner => $count) {
    decathlon_scoring_add_points($contest, $winner, 74, $count); // 74 is besting tid
  }

  $log_message .= "</ul>";
  return $log_message;
}

function decathlon_scoring_get_honor_data($tid) {
  $result = views_get_view_result('honor_data', 'panel_pane_1', $tid);
  return array(
    $result[0]->field_field_value[0]['raw']['value'],
    $result[0]->field_field_min_to_score[0]['raw']['value'],
  );
}

function decathlon_scoring_add_contest($gid) {

  global $user;

  $contest = db_insert('decathlon_contests')
    ->fields(array(
        'event' => decathlon_get_event(),
        'gid' => $gid,
        'umpire' => $user->uid,
        'created' => REQUEST_TIME,
       ))
    ->execute();
  
  return $contest;
}

function decathlon_scoring_add_contest_notes($contest, $notes) {

  db_merge('decathlon_contests')
    ->key(array(
        'contest' => $contest,
      ))
    ->fields(array(
        'log' => $notes,
      ))
    ->execute();
}

function decathlon_scoring_add_score($contest, $player, $score) {

  db_insert('decathlon_scores')
    ->fields(array(
        'contest' => $contest,
        'pid' => $player,
        'score' => $score,
       ))
    ->execute();
}

function decathlon_scoring_add_honor($contest, $player, $honor, $count) {

  db_insert('decathlon_honors')
    ->fields(array(
        'contest' => $contest,
        'pid' => $player,
        'honor' => $honor,
        'count' => $count,
       ))
    ->execute();
}

function decathlon_scoring_add_skunk($contest, $player) {

  db_insert('decathlon_skunks')
    ->fields(array(
        'contest' => $contest,
        'pid' => $player,
       ))
    ->execute();
}

function decathlon_scoring_add_points($contest, $player, $point_type, $points) {

  db_insert('decathlon_points')
    ->fields(array(
        'contest' => $contest,
        'pid' => $player,
        'point_type' => $point_type,
        'points' => $points,
       ))
    ->execute();
}

function decathlon_scoring_add_besting($event, $contest, $game, $winner, $loser) {

  db_insert('decathlon_besting')
    ->fields(array(
        'event' => $event,
        'contest' => $contest,
        'game' => $game, // tid
        'bester' => $winner,
        'bestee' => $loser,
       ))
    ->execute();
}

function decathlon_scoring_honor_automatic($tid) {
  $term = taxonomy_term_load($tid);
  $items = field_get_items('taxonomy_term', $term, 'field_auto');
  $value = field_view_value('taxonomy_term', $term, 'field_auto', $items[0]);
  return $value['#markup'];
}

/**
 * Caller needs to set up a batch to process returned operations array
 */
function decathlon_scoring_reset_scores() {

  $operations = array();

  // this function is rewriting these tables
  $operations[] = array('decathlon_scoring_reset_truncate_table', array('decathlon_besting'));
  $operations[] = array('decathlon_scoring_reset_truncate_table', array('decathlon_points'));
  $operations[] = array('decathlon_scoring_reset_truncate_table', array('decathlon_skunks'));

  // operate on all events
  $terms = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('event')->vid);

  foreach ($terms as $event) {

    // load scores
    $scores = db_query("SELECT dc.gid, dc.created, ds.contest contest, ds.pid, ds.score FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.event = :event", array(':event' => $event->tid))->fetchAll();
    
    // load honors
    $honors = db_query("SELECT dh.pid, dh.contest, dh.honor, dh.count FROM {decathlon_honors} dh, {decathlon_contests} dc WHERE dh.contest = dc.contest AND dc.event = :event", array(':event' => $event->tid))->fetchAll();

    // build scores and honors into an array to pass to decathlon_scoring_score_game
    $results = array();
    foreach ($scores as $row) {
      $results[$row->contest]['game'] = $row->gid;
      $results[$row->contest]['players'][$row->pid]['score'] = $row->score;
    }

    foreach ($honors as $row) {
      $results[$row->contest]['players'][$row->pid]['honors'] = $row->count;
    }

    // rewrite
    foreach ($results as $contest => $data) {
      $operations[] = array('decathlon_scoring_reset_score_game', array($event->tid, $contest, $data['game'], $data['players']));
    }
  }

  return $operations;
}

/**
 *
 */
function decathlon_scoring_reset_truncate_table($table, &$context) {
  db_truncate($table)->execute();
  $text = "Truncated table " . $table;
  $context['message'] = $text;
  $context['results'][] = $text;
}

/**
 *
 */
function decathlon_scoring_reset_score_game($event, $contest, $game, $players, &$context) {
  decathlon_scoring_score_game($event, $contest, $game, $players);
  $text = "Scored " . $contest;
  $context['message'] = $text;
  $context['results'][] = $text;
}

