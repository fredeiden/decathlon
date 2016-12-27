<?php

/**
 *
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
 *
 */
function decathlon_get_point_types() {
  // could go to the database for these, but they never change
  return array(
    73 => 'Participation',
    74 => 'Besting',
    77 => 'Honor',
    76 => 'Skunk',
  );
}

/**
 *
 */
function decathlon_get_all_rankings() {
  $results = db_query("SELECT dc.gid, dp.pid, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dc.gid, dp.pid ORDER BY dc.gid, SUM(dp.points) DESC", array(':event' => decathlon_get_event()))->fetchAll();
  return $results;
}

/**
 *
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
 *
 */
function decathlon_get_all_typed_rankings() {
  $results = &drupal_static(__FUNCTION__);
  if(!isset($results)) {
    $results = db_query("SELECT dc.gid, dp.pid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dc.gid, dp.pid, dp.point_type ORDER BY dc.gid", array(':event' => decathlon_get_event()))->fetchAll();
  }
  return $results;
}

/**
 *
 */
function decathlon_get_game_rankings($gid) {
  $results = db_query("SELECT dp.pid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event AND dc.gid = :gid GROUP BY dp.pid, dp.point_type ORDER BY dc.gid, SUM(dp.points) DESC", array(':event' => decathlon_get_event(), ':gid' => $gid))->fetchAll();
  return $results;
}

/**
 *
 */
function decathlon_get_player_game_scores($pid, $gid) {
  $results = db_query("SELECT AVG(ds.score) avg, MIN(ds.score) min, MAX(ds.score) max FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.event = :event AND ds.pid = :pid AND dc.gid = :gid", array(':event' => decathlon_get_event(), ':pid' => $pid, ':gid' => $gid))->fetchObject();
  return $results;
}

/**
 *
 */
function decathlon_get_player_game_bested($pid, $gid) {
  $results = db_query("SELECT COUNT(bester) bested FROM {decathlon_besting} WHERE event = :event AND bestee = :pid AND game = :gid", array(':event' => decathlon_get_event(), ':pid' => $pid, ':gid' => $gid))->fetchObject();
  return $results;
}

/**
 *
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
 *
 */
function decathlon_get_term_name($tid) {
  $my_map = &drupal_static(__FUNCTION__);
  if (!isset($my_map)) {
    $my_map = db_query("SELECT tid, name FROM taxonomy_term_data")->fetchAllKeyed();
  }
  return $my_map[$tid];
}

/**
 *
 */
function decathlon_game_link($tid) {
  return l(decathlon_get_term_name($tid), 'scores/games/' . $tid);
}

/**
 *
 */
function decathlon_contest_link($id) {
  return l($id, 'scores/game/' . $id);
}

/**
 *
 */
function decathlon_game_link_from_name($name) {
  $terms = taxonomy_get_term_by_name($name);
  return decathlon_game_link(array_pop($terms)->tid);
}

/**
 *
 */
function decathlon_player_link($tid) {
  return l(decathlon_get_term_name($tid), 'players/player/' . $tid);
}

/**
 *
 */
function decathlon_player_link_from_name($name) {
  $terms = taxonomy_get_term_by_name($name);
  return decathlon_player_link(array_pop($terms)->tid);
}

/**
 *
 */
function decathlon_high_score_wins($gid) {
  $term = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $term, 'field_high_score_wins');
  return $items[0]['value'];
}

/**
 *
 */
function decathlon_game_type($gid) {
  $term = taxonomy_term_load($gid);
  $items = field_get_items('taxonomy_term', $term, 'field_type');
  $value = field_view_value('taxonomy_term', $term, 'field_type', $items[0]);
  return strtolower($value['#title']);
}

/**
 *
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

    //decathlon
    $game_list['decathlon'] = array_merge($game_list['leisure'], $game_list['parlor']);

    //exhibition
    $exhibition = views_get_view_result('event_games', 'panel_pane_3');
    foreach ($exhibition as $row) {
      if (!empty($row->field_field_exhibition_games)) {
        $game_list['exhibition'][$row->field_field_exhibition_games[0]['raw']['target_id']] = 
          $row->field_field_exhibition_games[0]['rendered']['#markup'];
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
  return $game_list['decathlon'];
}

/**
 *
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

    foreach ($totals['games']['unweighted'] as $gid => $scores) {
      $rank = 1; // initialize to 1
      $counter = 0;
      $gain = array();
      $athlon = decathlon_game_type($gid);
      $game_weight = decathlon_dpi_get_participation_weighting($gid);
      $gm = decathlon_dpi_get_game_multipliers($gid);

      foreach ($scores as $index => $points) {
          
        $current_total = $points['Total'];
        $previous_total = isset($scores[($index - 1)]['Total']) ? $scores[($index - 1)]['Total'] : 100000;
        $next_total = isset($scores[($index + 1)]['Total']) ? $scores[($index + 1)]['Total'] : 0;
          
        // track ties by comparing current total to totals before and after
        $tie = FALSE;
        if ($index < (count($scores) - 1)) { // not the last row
          if ($current_total == $next_total) { // total of this row and next row are equal
            $tie = TRUE; // this is a tie
          }
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
        $pid = $totals['games']['weighted'][$gid][$index]['player'];
        // game
        $totals['games']['weighted'][$gid][$index]['Rank'] = $adjusted_rank;
        // overall by athlon
        if (isset($totals['sums'][$pid]['play'][$athlon]['rank'])) {
          $totals['sums'][$pid]['play'][$athlon]['rank'] += $adjusted_rank;
        } else {
          $totals['sums'][$pid]['play'][$athlon]['rank'] = $adjusted_rank;
        }

        //
        // Suggestion algorithm
        //
        // This is based on the idea that when you move up in the rankings you increase your overall number of DPI points.
        // For each player look at what they can achieve by scoring 1, 2, 3, 4, or 5 points.  Higher scores are less likely
        // and are multiplied by a factor calculated from either the player's historical scores or, lacking that, historical scores
        // of all players.
        //
        // Stored for each athlon and overall
        //
        if ($rank > 1 && $game_weight > 0) { // skip when already leading or game not weighted

          // an array of multipliers for this game and player at each point level, i.e.
          // 2 points in Boggle has a multiplier of .5, meaning this player gets 2 points half the time
          $pm = decathlon_dpi_get_player_multipliers($gid, $pid);

          // a array of the point totals that will reach a new rank
          $sp = array();
          $deficit = 0;
          for ($i = ($counter - 1); $i >= 0; $i--) { // walk up the $gain list
            $deficit += $gain[$i]['gap']; // add on the point gap

            // stop looking after 5 points (this could be tailored to each game (and player)
            if ($deficit > 5) {
              break;
            }
              
            // spread is the gain in dpi points for moving to a new rank
            $sp[$deficit]['rank'] = $gain[$i]['rank'];
            $sp[$deficit]['lower_rank'] = $gain[($i + 1)]['rank'];
          }

          $spread = 0;
          $bump = 0;
          $this_bump = 0;
          $new_rank = $rank;
          for ($points = 1; $points <= 5; $points++) {

            // points available to next ranking
            if (isset($sp[$points]['rank'])) {
              $new_rank = $sp[$points]['rank'];

              // moving into a tie for this total, current rank holder stays and player(s) behind will drop 1 rank relative to their current rank
              $bump = decathlon_dpi_get_ranking($sp[$points]['lower_rank']) - decathlon_dpi_get_ranking(($sp[$points]['lower_rank'] + 1));
              if ($spread == 0 && $tie == FALSE) { // there is no bump here
                $bump = 0;
              }

              // spread is the point gain from new rank to current rank
              $spread = decathlon_dpi_get_ranking($new_rank) - decathlon_dpi_get_ranking($rank);

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
            
            // estimate the likelihood of getting this number of points
            $multiplier = 0;
            $multiplier_type = "";
            if (isset($pm[$points])) {
              $multiplier = $pm[$points];
            } elseif($points < 5 && isset($gm[$points])) {
              $multiplier = $gm[$points];
              $multiplier_type = "(g)";
            }

            // the magic number
            $next_best_rank = (($spread + $bump) * $multiplier) / $game_weight;

            // log factors
            $log = array($spread, $bump, $multiplier . $multiplier_type, $game_weight, round($next_best_rank, 2));
            $totals['sums'][$pid]['log'][$gid][$points] = $log;

            if ($next_best_rank > 0) {
              // separate athlons
              if (isset($totals['sums'][$pid][$points][$athlon]['value'])) {
                if ($next_best_rank > $totals['sums'][$pid][$points][$athlon]['value']) {
                  $totals['sums'][$pid][$points][$athlon]['value'] = $next_best_rank;          
                  $totals['sums'][$pid][$points][$athlon]['game'] = $gid;          
                }
              } else {
                $totals['sums'][$pid][$points][$athlon]['value'] = $next_best_rank;
                $totals['sums'][$pid][$points][$athlon]['game'] = $gid;          
              }
              // decathlon
              if (isset($totals['sums'][$pid][$points]['decathlon']['value'])) {
                if ($next_best_rank > $totals['sums'][$pid][$points]['decathlon']['value']) {
                  $totals['sums'][$pid][$points]['decathlon']['value'] = $next_best_rank;          
                  $totals['sums'][$pid][$points]['decathlon']['game'] = $gid;          
                }
              } else {
                $totals['sums'][$pid][$points]['decathlon']['value'] = $next_best_rank;
                $totals['sums'][$pid][$points]['decathlon']['game'] = $gid;          
              }
            }
          }
        }

        // for eligibility
        // when not an exhibition game, add to completed game list
        empty($exhibition_games[$gid]) && $completed[$pid][$athlon][] = decathlon_get_term_name($gid);
      }
    }

    // Bonus Results
    // view to fetch bonus points - store in array of player id => score
    //
    $view = views_get_view_result("overall_results_raw", "panel_pane_1", decathlon_get_event());
    foreach ($view as $row) {
      $totals['sums'][$row->field_field_contestant[0]['raw']['tid']]['bonus'] = $row->field_field_score[0]['raw']['value'] * decathlon_dpi_get_multiplier('Bonus');
    }

    // Eligibility
    //
    $required_games = array(
      'leisure' => array_diff(decathlon_get_event_games('leisure'), $exhibition_games),
      'parlor' => array_diff(decathlon_get_event_games('parlor'), $exhibition_games),
    );

    foreach ($completed as $pid => $group) {
      foreach ($group as $athlon => $games) {
        $totals['sums'][$pid]['play'][$athlon]['eligibility'] = count(array_diff($required_games[$athlon], $completed[$pid][$athlon]));
      }
    }
  }
  return $totals;
}

/**
 *
 */
function decathlon_get_rankings_by_type($type) {
  $results = decathlon_get_rankings();
  return $results['games'][$type];  
}

/**
 *
 */
function decathlon_get_rankings_player_sums($pid) {
  $results = decathlon_get_rankings();
  return $results['sums'][$pid];  
}

/**
 *
 */
function decathlon_get_rankings_all_sums() {
  $results = decathlon_get_rankings();
  if (isset($results['sums'])) { return $results['sums']; }
  return array();
}

/**
 *
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
    }
    // bonus points for decathlon
    // this would be double counted inside the previous loop
    if (!empty($data['bonus'])) {
      $sums['decathlon'][$pid]['dpi'] += $data['bonus'];
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
 *
 */
function decathlon_get_opponents($athlon) {

  $results = decathlon_get_contests(); // get games and players
  $html = '';
  $opponents = array();
  $players = array();

  foreach ($results as $row) {
    if ($athlon != 'decathlon' && decathlon_game_type($row->gid) != $athlon) { continue; }
    if (isset($this_contest) && $row->contest != $this_contest) {
      foreach ($players as $p) {
        $opponents[$p] = isset($opponents[$p]) ? array_merge($opponents[$p], $players) : $players;
      }
      $players = array(); // reset
    }
    $players[] = $row->pid;
    $this_contest = $row->contest;
  }
  foreach ($players as $p) {
    $opponents[$p] = isset($opponents[$p]) ? array_merge($opponents[$p], $players) : $players;
  }

  // remove duplicates
  foreach ($opponents as $index => $played) {
    $opponents[$index] = array_unique($played);
  }

  return $opponents;
}

/**
 *
 */
function decathlon_player_opponents($pid) {
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

