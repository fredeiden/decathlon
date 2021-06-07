<?php

/**
 * Utility function to print 1st, 2nd, 3rd, etc.
 */
function decathlon_get_ordinal($number) {
  $ends = array('th','st','nd','rd','th','th','th','th','th','th');
  if (($number %100) >= 11 && ($number%100) <= 13) {
    return $number. 'th';
  } else {
    return $number. $ends[$number % 10];
  }
}

/**
 * Points are stored in the Drupal taxonomy structure and have taxonomy term IDs. 
 * Should probably clean this up and go to the database for these, but they never change.
 */
function decathlon_get_point_types() {
  return array(
    73 => 'Participation',
    74 => 'Besting',
    77 => 'Honor',
    76 => 'Skunk',
  );
}

/**
 * Query to fetch sum of points for each player in each game in the current event
 */
function decathlon_get_all_rankings() {
  $results = db_query("SELECT dc.gid, dp.pid, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dc.gid, dp.pid ORDER BY dc.gid, SUM(dp.points) DESC", array(':event' => decathlon_get_event()))->fetchAll();
  return $results;
}

/**
 * Query to fetch sum of points by point type and game in the current event
 */
function decathlon_get_points_games() {
  $results = db_query("SELECT dc.gid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dc.gid, dp.point_type ORDER BY dc.gid ASC", array(':event' => decathlon_get_event()));
  $points = array();
  foreach ($results as $row) {
    $points[$row->gid][$row->point_type] = $row->points;
  }
  return $points;
}

/**
 * Query to fetch sum of points by game, player, and point type in the current event
 */
function decathlon_get_all_typed_rankings() {
  $results = &drupal_static(__FUNCTION__);
  if(!isset($results)) {
    $results = db_query("SELECT dc.gid, dp.pid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dc.gid, dp.pid, dp.point_type ORDER BY dc.gid", array(':event' => decathlon_get_event()))->fetchAll();
  }
  return $results;
}

/**
 * Query to fetch sum of points by player and point type for a specific game in the current event
 * 
 * @param $gid
 *  The taxonomy term ID for the game (backgammon is a game whereas a specific game of backgammon is a contest)
 */
function decathlon_get_game_rankings($gid) {
  $results = db_query("SELECT dp.pid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event AND dc.gid = :gid GROUP BY dp.pid, dp.point_type ORDER BY dc.gid, SUM(dp.points) DESC", array(':event' => decathlon_get_event(), ':gid' => $gid))->fetchAll();
  return $results;
}

/**
 * Query to fetch avg, min, and max scores for a player in a game in the current event
 *
 * @param $pid
 *  The taxonomy term ID for a player
 *
 * @param $gid
 *  The taxonomy term ID for a game
 */
function decathlon_get_player_game_scores($pid, $gid) {
  $results = db_query("SELECT AVG(ds.score) avg, MIN(ds.score) min, MAX(ds.score) max FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.event = :event AND ds.pid = :pid AND dc.gid = :gid", array(':event' => decathlon_get_event(), ':pid' => $pid, ':gid' => $gid))->fetchObject();
  return $results;
}

/**
 * Query to fetch number of besters for a player in a game in the current event
 *
 * @param $pid
 *  The taxonomy term ID for a player
 *
 * @param $gid
 *  The taxonomy term ID for a game
 */
function decathlon_get_player_game_bested($pid, $gid) {
  $results = db_query("SELECT COUNT(bester) bested FROM {decathlon_besting} WHERE event = :event AND bestee = :pid AND game = :gid", array(':event' => decathlon_get_event(), ':pid' => $pid, ':gid' => $gid))->fetchObject();
  return $results;
}

/**
 * Query to fetch all contest, game, and player rows from the current event
 */
function decathlon_get_contests() {
  $results = &drupal_static(__FUNCTION__);
  if (! isset($results)) {
    $results = db_query(
      "SELECT dc.contest, dc.gid, ds.pid FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE dc.contest = ds.contest AND dc.event = :event ORDER BY dc.contest", 
      array(':event' => decathlon_get_event())
    )->fetchAll();
  }
  return $results;
}

/**
 * Helper function to return the name for a Drupal taxonomy term ID.
 * Heavily used, so cache all rows with one query. 
 *
 * @param $tid
 *  The taxonomy term ID for a player, game, etc.
 */
function decathlon_get_term_name($tid) {
  $my_map = &drupal_static(__FUNCTION__);
  if (!isset($my_map)) {
    $my_map = db_query("SELECT tid, name FROM taxonomy_term_data")->fetchAllKeyed();
  }
  if (isset($tid)) {
    return $my_map[$tid];
  }
}

/**
 * Helper function to create HTML <a></a> link for a game
 *
 * @param $gid
 *  The taxonomy term ID for a game
 */
function decathlon_game_link($gid) {
  return l(decathlon_get_term_name($gid), 'scores/games/' . $gid);
}

/**
 * Helper function to create HTML <a></a> link for a contest
 *
 * @param $id
 *  The contest ID
 */
function decathlon_contest_link($id) {
  return l($id, 'scores/game/' . $id);
}

/**
 * Helper function to provide HTML <a></a> link to game based on game name
 */
function decathlon_game_link_from_name($name) {
  // Use Drupal function to get array of taxonomy term objects
  $terms = taxonomy_get_term_by_name($name);
  // Return the ID of the first object
  return decathlon_game_link(array_pop($terms)->tid);
}

/**
 * Helper function to create HTML <a></a> link for a player
 *
 * @param $pid
 *  The taxonomy term ID for a player
 * @param $text
 *  Optional text for link; player name when null
 */
function decathlon_player_link($pid, $text = NULL) {
  if (! $text) {
    $text = decathlon_get_term_name($pid);
  }
  return l($text, 'player/' . $pid . '/results');
}

/**
 * Helper function to provide HTML <a></a> link to player based on player name
 */
function decathlon_player_link_from_name($name) {
  // Use Drupal function to get array of taxonomy term objects
  $terms = taxonomy_get_term_by_name($name);
  // Return the ID of the first object
  return decathlon_player_link(array_pop($terms)->tid);
}

/**
 * Get the value of the high_score_wins variable for a game using some Drupal-ly stuff.
 * Games are stored as taxonomy terms in Drupal and have the associated overhead.
 */
function decathlon_high_score_wins($gid) {
  $term = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $term, 'field_high_score_wins');
  return $items[0]['value'];
}

/**
 * Get the value of the perfect_score variable for a game using some Drupal-ly stuff.
 * Games are stored as taxonomy terms in Drupal and have the associated overhead.
 */
function decathlon_perfect_score($gid) {
  $term = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $term, 'field_perfect_score');
  return $items[0]['value'];
}

/**
 * Get the type of game using some Drupal-ly stuff.
 * Games are stored as taxonomy terms in Drupal and have the associated overhead.
 */
function decathlon_game_type($gid) {
  $term = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $term, 'field_type');
  $value = field_view_value('taxonomy_term', $term, 'field_type', $items[0]);
  return strtolower($value['#title']);
}

/**
 * Decathlon games are separated into leisure games and parlor games.  All are decathlon. Some are exhibition.
 * This function will return a list of games for the respective grouping.
 * 
 * @param $athlon
 *  String assigned as 'decathlon', 'leisure', 'parlor', or 'exhibition'
 */
function decathlon_get_event_games($athlon = NULL) {

  $game_list = &drupal_static(__FUNCTION__);

  if (!isset($game_list)) {
 
    //leisure
    $leisure = views_get_view_result('event_games', 'panel_pane_1');
    foreach ($leisure as $row) {
      $game_list['leisure'][$row->field_data_field_leisure_games_field_leisure_games_target_id] = 
        $row->field_field_leisure_games[0]['raw']['entity']->name;
    }

    // parlor
    $parlor = views_get_view_result('event_games', 'panel_pane_2');
    foreach ($parlor as $row) {
      $game_list['parlor'][$row->field_data_field_parlor_games_field_parlor_games_target_id] = 
        $row->field_field_parlor_games[0]['raw']['entity']->name;
    }

    //exhibition
    $exhibition = views_get_view_result('event_games', 'panel_pane_3');
    foreach ($exhibition as $row) {
      if (!empty($row->field_field_exhibition_games)) {
        $game_list['exhibition'][$row->field_field_exhibition_games[0]['raw']['target_id']] = 
          $row->field_field_exhibition_games[0]['raw']['entity']->name;
      }
    }
  }

  if ($athlon == 'leisure') {
    return $game_list['leisure'];
  }
  if ($athlon == 'parlor') {
    return $game_list['parlor'];
  }
  if ($athlon == 'exhibition') {
    return !empty($game_list['exhibition']) ? $game_list['exhibition'] : array();
  }
  return $game_list['leisure'] + $game_list['parlor'];
}

/**
 * This is the super scary, gigantic, left-over, legacy procedural mess that I just can't seem to kill.
 * It is like the water sprouts that keep growing on top of my fruit tree.
 *
 * In brief, loops over all scoring records for the current event and returns
 * an array of arrays indexed by game and player.  It is used by the various reports
 * that show game rankings, player standings, and game recommendations.
 */
function decathlon_get_rankings() {

  // Game Results
  // query to fetch game, player, point type, points
  //
  $results = decathlon_get_all_typed_rankings();
  if (count($results) == 0) { return array(); }

  //
  // loop over results and calculate unweighted and weighted point totals
  //
  $totals = &drupal_static(__FUNCTION__);
  if (!isset($totals)) {

    foreach ($results as $row) {

      $gid = $row->gid;
      $athlon = decathlon_game_type($gid);
      $pid = $row->pid;
      $points = $row->points;
      $type = decathlon_get_term_name($row->point_type);

      // unweighted points
      $totals['games']['unweighted'][$gid][$pid]['player'] = $pid; // save for later
      $totals['games']['unweighted'][$gid][$pid][$type] = $points;
      if (isset($totals['games']['unweighted'][$gid][$pid]['Total'])) {
        $totals['games']['unweighted'][$gid][$pid]['Total'] += $points;
      } else {
        $totals['games']['unweighted'][$gid][$pid]['Total'] = $points;
      }

      // weighted points
      $totals['games']['weighted'][$gid][$pid]['player'] = $pid; // save for later
      $adjusted_points = $points * decathlon_dpi_get_participation_weighting($gid) * decathlon_dpi_get_multiplier($type);
      $totals['games']['weighted'][$gid][$pid][$type] = $adjusted_points;
      if (isset($totals['games']['weighted'][$gid][$pid]['Total'])) {
        $totals['games']['weighted'][$gid][$pid]['Total'] += $adjusted_points;
      } else {
        $totals['games']['weighted'][$gid][$pid]['Total'] = $adjusted_points;
      }

      // sums
      if (isset($totals['sums'][$pid]['play'][$athlon]['points'])) {
        $totals['sums'][$pid]['play'][$athlon]['points'] += $adjusted_points;
      } else {
        $totals['sums'][$pid]['play'][$athlon]['points'] = $adjusted_points;
      }
    }

    //
    // sort the weighted and unweighted lists by total points descending
    //
    foreach ($totals['games']['unweighted'] as $gid => $data) {
      // sort unweighted
      usort($data, function($a, $b) {
          return $b['Total'] - $a['Total'];
        });
      // reset array with ordered values
      $totals['games']['unweighted'][$gid] = $data;

      // sort weighted
      $new_weights = array();
      foreach ($totals['games']['unweighted'][$gid] as $index => $data) {
        $new_weights[$index] = $totals['games']['weighted'][$gid][$data['player']];
      }
      // reset array with ordered values
      $totals['games']['weighted'][$gid] = $new_weights;
    }

    //
    // add the ranking
    //
    $exhibition_games = decathlon_get_event_games('exhibition');
    $completed = array();
    $last_suggestion = array();

    foreach ($totals['games']['unweighted'] as $gid => $scores) {
      $rank = 1; // initialize to 1
      $counter = 0;
      $gain = array();
      $athlon = decathlon_game_type($gid);
      $game_weight = decathlon_dpi_get_participation_weighting($gid);

      foreach ($scores as $index => $points) {

        $pid = $points['player'];
          
        $current_total = $points['Total'];
        $previous_total = isset($scores[($index - 1)]['Total']) ? $scores[($index - 1)]['Total'] : 100000;
        $next_total = isset($scores[($index + 1)]['Total']) ? $scores[($index + 1)]['Total'] : 0;
          
        // track ties by comparing current total to totals before and after
        $tie = FALSE;
        if ($index < (count($scores) - 1)) { // not the last row
          if ($current_total == $next_total) { // total of this row and next row are equal
            $tie = TRUE; // this is a tie
          }
        } else {
          //dsm($scores);
        }
        if ($index > 0) { // past the first row
          if ($current_total == $previous_total) { // total of this row and previous row are equal
            $tie = TRUE; // this is a tie
          }
        }

        // adjust rank when not the first row and total has changed
        if ($index > 0 && ($current_total != $previous_total)) {
          $previous_rank = $index == 0 ? 1 : $rank; // save next best rank
          $previous_rank_total = $previous_total;
          $rank = ($index + 1); // move the rank up
          $counter++;
        }
        $gain[$counter]['gap'] = $current_total - $next_total;
        $gain[$counter]['rank'] = $rank;

        // write unweighted column
        if ($tie == TRUE) {
          $totals['games']['unweighted'][$gid][$index]['Rank'] = "T" . decathlon_get_ordinal($rank);
        } else {
          $totals['games']['unweighted'][$gid][$index]['Rank'] = decathlon_get_ordinal($rank);
        }

        //
        // write weighted columns
        //
        $adjusted_rank = decathlon_dpi_get_ranking($rank);          
        // game
        $totals['games']['weighted'][$gid][$index]['Rank'] = $adjusted_rank;
        // overall by athlon
        if (isset($totals['sums'][$pid]['play'][$athlon]['rank'])) {
          $totals['sums'][$pid]['play'][$athlon]['rank'] += $adjusted_rank;
        } else {
          $totals['sums'][$pid]['play'][$athlon]['rank'] = $adjusted_rank;
        }

        // Suggestion
//        if ($rank > 1 && $game_weight > 0) { // skip when already leading or game not weighted
        if ($game_weight > 0) { // skip when already leading or game not weighted
          $suggestion = decathlon_make_suggestion($athlon, $game_weight, $rank, $tie, $gid, $pid, $counter, $gain);
          decathlon_save_suggestion($totals, $suggestion, $pid, $gid);
        }

        // For eligibility
        // when not an exhibition game, add to completed game list
        empty($exhibition_games[$gid]) && $completed[$pid][$athlon][] = decathlon_get_term_name($gid);
      }

      // save last suggestion data for players who have not played
      $last_suggestion[$gid] = array(
        'rank' => $rank,
        'counter' => $counter,
        'gain' => $gain,
      );
    }

    // Bonus Results
    // view to fetch bonus points - store in array of player id => score
    //
    $view = views_get_view_result("overall_results_raw", "panel_pane_2", decathlon_get_event());

    foreach ($view as $row) {

      $recipient = $row->field_field_recipient[0]['raw']['target_id'];
      $bonus_count = $row->field_field_score[0]['raw']['value'];
      $bonus_multiplier = decathlon_dpi_get_bonus_multiplier($row->field_field_bonus[0]['rendered']['#markup']);

      // to weight the bonus, multiply count by dpi multiplier by time factor
      $weighted_bonus = $bonus_count * $bonus_multiplier;
      if (isset($row->field_field_service_length[0]['rendered']['#markup'])) {
        $weighted_bonus *= ($row->field_field_service_length[0]['rendered']['#markup'] / 60); // fraction of an hour
      }
      $weighted_bonus = round($weighted_bonus);

      // now assign the sum of weighted bonus values to the recipient
      if (isset($totals['sums'][$recipient]['bonus'])) {
        $totals['sums'][$recipient]['bonus'] += $weighted_bonus;
      } else {
        $totals['sums'][$recipient]['bonus'] = $weighted_bonus;
      }
    }

    // MostP
    foreach (array('decathlon', 'leisure', 'parlor') as $athlon) {
      // player opponents
      $all_played = decathlon_get_opponents($athlon);
      foreach ($all_played as $pid => $unique_opponents) {
        $totals['sums'][$pid]['mostp'][$athlon] = count($unique_opponents) * decathlon_dpi_get_multiplier('MostP');
      }
    }

    // Eligibility and secondary pass on suggestion (for uncompleted games)
    //
    $required_games = array(
      'leisure' => array_diff(decathlon_get_event_games('leisure'), $exhibition_games),
      'parlor' => array_diff(decathlon_get_event_games('parlor'), $exhibition_games),
    );

    foreach ($completed as $pid => $group) {
      foreach ($group as $athlon => $games) {

        // list of games that player has not played
        $unplayed = array_diff($required_games[$athlon], $completed[$pid][$athlon]);

        // eligibility is simply a count of games unplayed
        $totals['sums'][$pid]['play'][$athlon]['eligibility'] = count($unplayed);

        // also relevant is the possibility that playing an unplayed game is the best possible point gain
        // as a player does get a base number for playing at least once
        foreach ($unplayed as $gid => $name) {
          if (isset($last_suggestion[$gid])) {
            $game_weight = decathlon_dpi_get_participation_weighting($gid);
            $ls = $last_suggestion[$gid];
            $suggestion = decathlon_make_suggestion($athlon, $game_weight, $ls['rank'], FALSE, $gid, $pid, $ls['counter'], $ls['gain'], TRUE);
            decathlon_save_suggestion($totals, $suggestion, $pid, $gid);
          }
        }
      }
    }
  }

  return $totals;
}

/**
 * Suggestion algorithm
 *
 * This is based on the idea that when you move up in the rankings you increase your overall number of DPI points.
 * For each player look at what they can achieve by scoring 1, 2, 3, 4, or 5 points.  Higher scores are less likely
 * and are multiplied by a factor calculated from either the player's historical scores or, lacking that, historical scores
 * of all players.
 *
 * This list of params is ridiculous, I know.  It is the side effect of extracting this from the clutches
 * of decathlon_get_rankings().
 *
 * Stored for each athlon and overall
 *
 * @param $athlon
 * 
 * @param $game_weight
 * 
 * @param $rank
 *
 * @param $tie
 *
 * @param $pid
 *
 * @param $counter
 *
 * @param $gain
 *
 * @param $new_player
 *  flag to mark this player as new and therefore currently having no rank or points
 *
 */
function decathlon_make_suggestion($athlon, $game_weight, $rank, $tie, $gid, $pid, $counter, $gain, $new_player = FALSE) {

  $suggestion = array();
  $limit = 8; // you can only score so many points in each game, 5 is probably the max, but check 8 to be safe

  // array of the point totals that will reach a new rank
  $sp = array();
  $deficit = 0;
  for ($i = ($counter - 1); $i >= 0; $i--) { // walk up the $gain list
    $deficit += $gain[$i]['gap']; // add on the point gap

    // stop looking after n points (this could be tailored to each game and player?)
    if ($deficit > $limit) {
      break;
    }
              
    // spread is the gain in dpi points for moving to a new rank
    $sp[$deficit]['rank'] = $gain[$i]['rank'];
    $sp[$deficit]['lower_rank'] = $gain[($i + 1)]['rank'];
  }

  $spread = 0;
  $bump = 0;
  $new_rank = $rank;
  for ($points = 1; $points <= $limit; $points++) {
    // points available to next ranking
    if (isset($sp[$points]['rank'])) {

      $new_rank = $sp[$points]['rank'];

      // moving into a tie for this total, current rank holder stays and player(s) behind will drop 1 rank relative to their current rank
      $bump = decathlon_dpi_get_ranking($sp[$points]['lower_rank']) - decathlon_dpi_get_ranking(($sp[$points]['lower_rank'] + 1));
      if ($spread == 0 && $tie == FALSE) { // there is no bump here
        $bump = 0;
      }

      // spread is the point gain from new rank to current rank
      if ($new_player) {
        $spread = decathlon_dpi_get_ranking($new_rank); // new player has 0 ranking points
      } else {
        $spread = decathlon_dpi_get_ranking($new_rank) - decathlon_dpi_get_ranking($rank);
      }

    } else {
      if ($spread == 0) {
        // not enough to reach next rank, but would break a tie
        if ($tie == TRUE) {
          $bump = decathlon_dpi_get_ranking(($rank)) - decathlon_dpi_get_ranking(($rank + 1));
        }
      } else {
        // only player with this total, so current holder of this rank gets bumped from this to next
        $bump = decathlon_dpi_get_ranking(($new_rank)) - decathlon_dpi_get_ranking(($new_rank + 1));
      }
    }
            
    // estimate of the likelihood of getting this number of points
    $multiplier = decathlon_dpi_get_player_multipliers($gid, $pid, $points);

    // the magic number
    $next_best_rank = (($spread + $bump) * $multiplier['value']) / $game_weight;

    $suggestion[] = array(
      'points' => $points,
      'athlon' => $athlon,
      'spread' => $spread,
      'bump' => $bump,
      'multiplier' => $multiplier,
      'game_weight' => $game_weight,
      'result' => $next_best_rank,
    );
  }

  return $suggestion;
}


/**
 * Save a suggestion array to a totals array.  What the hell does that mean?
 * This is called in two places in decathlon_get_rankings() and is really just a convenience.
 * Good luck finding another use for this!
 */
function decathlon_save_suggestion(&$totals, $suggestion, $pid, $gid) {

  foreach ($suggestion as $row) {
    if ($row['multiplier']['value'] > 0) { // some chance of getting this
      $totals['sums'][$pid]['log'][$gid][$row['points']] = array(
        'athlon' => $row['athlon'],
        'spread' => $row['spread'], 
        'bump' => $row['bump'], 
        'multiplier' => $row['multiplier']['value'],
        'multiplier_suffix' => $row['multiplier']['string'], 
        'game_weight' => $row['game_weight'], 
        'result' => round($row['result'], 2),
      );
    }
  }
}

function decathlon_easy_suggestion($log, $athlon) {
  $max = 0;
  $option = "";

  foreach ($log as $gid => $levels) {
    if ($athlon == 'decathlon' || $athlon == $levels[1]['athlon']) {
      if ($levels[1]['result'] > $max) { // array indexed starting from 1
        $max = $levels[1]['result'];
        $option = $gid;
      }
    }
  }

  return empty($option) ? "-" : decathlon_game_icon($option);
}

function decathlon_medium_suggestion($log, $athlon) {
  $max = 0;
  $option = "";

  foreach ($log as $gid => $levels) {
    if ($athlon == 'decathlon' || $athlon == $levels[1]['athlon']) {
      // sum totals for rows other than low and high
      $start = 2; // array indexed starting from 1; start at second row
      $end = (count($levels) - 1); // end at penultimate row
      $total = 0;
      $num_rows = 0;
      for ($i = $start; $i <= $end; $i++) {
        $total += $levels[$i]['result'];
        $num_rows++;
      }
      if ($num_rows > 0) {
        $avg = ($total / $num_rows);
        if ($avg > $max) {
          $max = $avg;
          $option = $gid;
        }
      }
    }
  }

  return empty($option) ? "-" : decathlon_game_icon($option);
}

function decathlon_hard_suggestion($log, $athlon) {
  $max = 0;
  $option = "";

  foreach ($log as $gid => $levels) {
    if ($athlon == 'decathlon' || $athlon == $levels[1]['athlon']) {
      $last = count($levels); // log array indexed starting from 1
      if (isset($levels[$last]['result'])) {
        if ($levels[$last]['result'] > $max) {
          $max = $levels[$last]['result'];
          $option = $gid;
        }
      }
    }
  }

  return empty($option) ? "-" : decathlon_game_icon($option);
}

/**
 * Return a portion of the decathlon_get_rankings() array.
 */
function decathlon_get_rankings_by_type($type) {
  $results = decathlon_get_rankings();
  if (isset($results['games'][$type])) {
    return $results['games'][$type];  
  }
  return array();
}

/**
 * Return a portion of the decathlon_get_rankings() array.
 */
function decathlon_get_rankings_player_sums($pid) {
  $results = decathlon_get_rankings();
  if (isset($results['sums'][$pid])) {
    return $results['sums'][$pid];
  }  
}

/**
 * Return a portion of the decathlon_get_rankings() array.
 */
function decathlon_get_rankings_all_sums() {
  $results = decathlon_get_rankings();
  if (isset($results['sums'])) { return $results['sums']; }
  return array();
}

/**
 * Set overall DPI totals for each player and apply podium house rule.
 */
function decathlon_get_ranking_points() {

  $results = decathlon_get_rankings_all_sums();

  if (empty($results)) { return array(); }
  $sums = array();

  foreach ($results as $pid => $data) {
    // initialize dpi and player for decathlon
    $sums['decathlon'][$pid]['dpi'] = 0;
    $sums['decathlon'][$pid]['player'] = $pid;
    foreach (array('leisure', 'parlor') as $group) {
      // initialize dpi and player for athlons
      $sums[$group][$pid]['dpi'] = 0;
      $sums[$group][$pid]['player'] = $pid;
      if (!empty($data['play'][$group])) {
        // athlon points
        $sums[$group][$pid]['dpi'] = $data['play'][$group]['points'] + $data['play'][$group]['rank'];
        // decathlon points
        $sums['decathlon'][$pid]['dpi'] += $data['play'][$group]['points'] + $data['play'][$group]['rank'];
      } 
      // bonus points for athlons
      if (!empty($data['bonus'])) {
        $sums[$group][$pid]['dpi'] += $data['bonus'];
      }
      // mostp points for athlons
      if (!empty($data['mostp'][$group])) {
        $sums[$group][$pid]['dpi'] += $data['mostp'][$group];
      }
    }
    // bonus points for decathlon
    // this would be double counted inside the previous loop
    if (!empty($data['bonus'])) {
      $sums['decathlon'][$pid]['dpi'] += $data['bonus'];
    }
    // mostp points for decathlon
    if (!empty($data['mostp']['decathlon'])) {
      $sums['decathlon'][$pid]['dpi'] += $data['mostp']['decathlon'];
    }
  }

  // sort each array group by points
  foreach (array('leisure', 'parlor', 'decathlon') as $group) {
    if (isset($sums[$group])) {
      uasort($sums[$group], function($a, $b) {
          return $b['dpi'] - $a['dpi'];
        });
    }
  }

  // apply house rule for podium positions
  //
  // a player can only hold 1 podium position across the 3 events
  // 1st in decathlon takes precedence
  // 1st in athlon follows
  // 2nd in decathlon is next
  // etc.
  //
  // this routine does not account for a tiebreaker between leisure and parlor, but simply gives
  // leisure first dibs
  if (!empty($sums)) {
    $group = array(
      'decathlon' => array(
        'index' => 0,
        'keys' => array_keys($sums['decathlon']),
        'other' => array('leisure', 'parlor'),
      ),
      'leisure' => array(
        'index' => 0,
        'keys' => array_keys($sums['leisure']),
        'other' => array('decathlon', 'parlor'),
      ),
      'parlor' => array(
        'index' => 0,
        'keys' => array_keys($sums['parlor']),
        'other' => array('decathlon', 'leisure'),
      ),
    );
    for ($i = 0; $i < count($results); $i++) { // 3 passes
      foreach ($group as $name => $settings) {
        $keys = $settings['keys'];
        $index = $settings['index'];
        
        if (isset($keys[$index])) {
          $pid = $keys[$index];
        } else {
          break;
        }
      
        // find eligible player not currently on a podium
        while (!empty($sums[$name][$pid]['podium']) || (decathlon_get_leaderboard_eligibility($results[$pid], $name) != 0)) { 
          $index++;
          if (isset($keys[$index])) {
            $pid = $keys[$index];
          } else {
            break;
          }
        }

        // in the categories other than this, mark player as on podium
        foreach ($settings['other'] as $other) { 
          $sums[$other][$pid]['podium'] = $name;
        }
        
        $index++; // increment for next pass
        $group[$name]['index'] = $index;  // save new index
      }
    }
  }

  return $sums;
}

/**
 * Build a list of players that each player has played
 *
 * @param $athlon
 *  Set as either decathlon, leisure, or parlor
 */
function decathlon_get_opponents($athlon) {

  $results = decathlon_get_contests(); // get games and players
  $opponents = array();
  $players = array();

  // loop over query from decathlon_get_contests
  // query is grouped by contest
  foreach ($results as $row) {

    // only look at relevant athlon
    if ($athlon != 'decathlon' && decathlon_game_type($row->gid) != $athlon) { continue; }

    // each time contest ID changes, store player set and loop some more
    if (isset($this_contest) && $row->contest != $this_contest) {
      foreach ($players as $p) {
        $opponents[$p] = isset($opponents[$p]) ? array_merge($opponents[$p], $players) : $players;
      }
      $players = array(); // reset
    }
    $players[] = $row->pid;
    $this_contest = $row->contest;
  }

  // grab the last set
  foreach ($players as $p) {
    $opponents[$p] = isset($opponents[$p]) ? array_merge($opponents[$p], $players) : $players;
  }

  // remove duplicates
  foreach ($opponents as $index => $played) {
    $opponents[$index] = array_unique($played);
  }

  // remove self
  foreach ($opponents as $index => $played) {
    $self = array_search($index, $played);
    unset($opponents[$index][$self]);
  }

  return $opponents;
}

/**
 *
 */
function decathlon_get_decathlon_eligibility($play) {
  $target = count(decathlon_get_event_games());
  if (isset($play['leisure']['eligibility'])) {
    $target -= (count(decathlon_get_event_games('leisure')) - $play['leisure']['eligibility']);
  }
  if (isset($play['parlor']['eligibility'])) {
    $target -= (count(decathlon_get_event_games('parlor')) - $play['parlor']['eligibility']);
  }
  return $target;
}

/**
 *
 */
function decathlon_get_leaderboard_eligibility($data, $athlon) {
  $eligibility = array(
    'leisure' => 0,
    'parlor' => 0,
  );
  foreach ($eligibility as $name => $count) {
    if (isset($data['play'][$name]['eligibility'])) {
      $eligibility[$name] = $data['play'][$name]['eligibility'];
    } else {
      $eligibility[$name] = count(array_diff(decathlon_get_event_games($name), decathlon_get_event_games('exhibition')));
    }
  }

  if ($athlon == 'decathlon') {
    return ($eligibility['leisure'] + $eligibility['parlor']);
  }

  return $eligibility[$athlon];
}

/**
 *
 */
function decathlon_get_eligible_string($num, $text) {
  if ($num == 0) {
    $string = '<div class="rank other"><span>' . $text . '</span></div>';
  } else {
    $string = '<div class="rank not-eligible"><span>' . $num . ' to play' . '</span></div>';
  }
  return $string;
}

/**
 *
 */
function decathlon_get_eligible_cell($num) {
  if ($num == 0) {
    $cell = array('class' => 'eligible');
  } else {
    $cell = array(
      'data' => $num . ' to go',
      'class' => 'not-eligible',
    );
  }
  return $cell;
}

/**
 *
 */
function decathlon_game_icon($gid) {
  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map[$gid])) {
    $term = taxonomy_term_load($gid);
    $my_map[$gid] = l(theme('image_style', array(
                        'style_name' => 'mini', 
                        'path' => $term->field_picture['und'][0]['uri'],
                      )), 'scores/games/' . $gid, array('html' => TRUE));
  }

  return $my_map[$gid];
}

/**
 *
 */
function decathlon_player_athlon_dpi($results, $athlon) {
  $dpi = 0;
  if (isset($results['play'][$athlon]['points'])) {
    $dpi += $results['play'][$athlon]['points'];
  }
  if (isset($results['play'][$athlon]['rank'])) {
    $dpi += $results['play'][$athlon]['rank'];
  }
  if (isset($results['bonus'])) {
    $dpi += $results['bonus'];
  }
  return $dpi;
}

/**
 *
 */
function decathlon_player_decathlon_dpi($results) {
  $dpi = 0;
  $dpi += decathlon_player_athlon_dpi($results, 'leisure');
  $dpi += decathlon_player_athlon_dpi($results, 'parlor');
  // bonus is double counted above
  if (isset($results['bonus'])) {
    $dpi -= $results['bonus'];
  }
  return $dpi;
}

/**
 *
 */
function decathlon_doubles_game($gid, $num) {
  $game = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $game, 'field_format');
  $format = taxonomy_term_load($items[0]['tid']);

  if (($format->tid == 204 || $format->tid == 205) && $num == 4) {
    return TRUE;
  }
  return FALSE;
}

/**
 *
 */
function decathlon_hours_and_minutes($total_minutes) {
  $hours = floor($total_minutes / 60);
  $minutes = ($total_minutes % 60);
  return sprintf('%02d:%02d', $hours, $minutes);
}

/**
 *
 */
function decathlon_round($num) {
  return round($num, 1);
}

/**
 *
 */
function decathlon_table_sort($data, $index, $sort) {
  if (empty($index)) {return $data;}
  uasort($data, function($a, $b) use ($index, $sort) {
      if ($sort == 'asc') {
        if (is_numeric($a[$index])) {
          if ($a[$index] == $b[$index]) {return 0;}
          return ($a[$index] > $b[$index]) ? 1 : -1;
        } else {
          return strcmp($a[$index], $b[$index]);
        }
      } else {
        if (is_numeric($a[$index])) {
          if ($a[$index] == $b[$index]) {return 0;}
          return ($a[$index] < $b[$index]) ? 1 : -1;
        } else {
          return strcmp($b[$index], $a[$index]);
        }
      }
    });
  
  return $data;
}

/**
 *
 */
function decathlon_table($header, $table_data) {

  // add 'field' column to header
  foreach (array_keys($header) as $key) {
    $header[$key]['field'] = $key;
  }

  // sort
  $order = tablesort_get_order($header);
  $sort = tablesort_get_sort($header);
  $table_data = decathlon_table_sort($table_data, $order['sql'], $sort);

  // table rows
  $rows = array();
  foreach ($table_data as $row) {
    array_walk($row, 'decathlon_table_row', $header);
    $rows[] = $row;
  }

  // theme table
  return theme('table', array(
      'sticky' => FALSE, 
      'header' => $header, 
      'rows' => $rows,
    ));
}

/**
 *
 */
function decathlon_table_row(&$value, $key, $header) {
  if (isset($header[$key]['convert'])) {
    $callback = $header[$key]['convert'];
    $value = $callback($value);
  } else {
    $value = $value;
  }
}

/**
 *
 */
function decathlon_count_honors($tid, $count) {
  
  // get honors minimum to score (i.e. need 3 ringers to score the honor)
  $min_to_score = decathlon_taxonomy_get_override($tid, 'field_min_to_score');

  // check for min to score and then multiply count by value
  if (isset($min_to_score)) {
    if ($min_to_score > 1) {
      $count = ($count - ($min_to_score - 1));
      if ($count < 0) { $count = 0; }
    }
  }

  return $count;
}

/**
 *
 */
function decathlon_get_honors_award($tid, $count) {
  // get honors value
  $term = taxonomy_term_load($tid);
  $value = $term->field_value['und'][0]['value'];

  $adjusted_for_min = decathlon_count_honors($tid, $count);

  return ($adjusted_for_min * $value);
}

