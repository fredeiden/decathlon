<?php

/**
 * Get weights for a specific grouping: point, game, or ranking
 */ 
function decathlon_dpi_get_multipliers($group) {

  $list = array();

  // initialize list with all types
  switch($group) {
    case 'point':
      $terms = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('dpi_multiplier')->vid);
      break;
    case 'game':
      $terms = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('games')->vid);
      break;
    case 'ranking':
      $terms = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('ranking')->vid);
      break;
  }

  foreach ($terms as $row) {
    $term = taxonomy_term_load($row->tid);
    $weight = db_query("SELECT multiplier FROM decathlon_dpi WHERE event = :event AND tid = :tid", array(':event' => decathlon_get_event(), ':tid' => $term->tid))->fetchField();
    $list[$term->tid] = array(
      'name' => $term->name,
      'value' => $weight ? $weight : 0,
    );
  }

  return $list;
}

/**
 * Return weight for point type (i.e. Bonus)
 */ 
function decathlon_dpi_get_multiplier($option) {
  $my_map = &drupal_static(__FUNCTION__);
  if (!isset($my_map)) {
    $list = decathlon_dpi_get_multipliers('point');
    foreach ($list as $row) {
      $my_map[$row['name']] = $row['value'];
    }
  }

  return $my_map[$option];
}

/**
 * Return weight for game participation
 */ 
function decathlon_dpi_get_participation_weighting($tid) {

  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map)) {
    $my_map = decathlon_dpi_get_multipliers('game');
  }

  return $my_map[$tid]['value'];
}

/**
 * Return weight for ranking
 */ 
function decathlon_dpi_get_ranking($rank) {

  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map)) {
    $list = decathlon_dpi_get_multipliers('ranking');
    $my_map = array_merge($list);
  }

  if ($rank >= count($my_map)) { // Rankings above n all have the same value
    $index = count($my_map) - 1;
  } else {
    $index = $rank - 1;
  }

  return $my_map[$index]['value'];
}

/**
 * Set or update value in database
 */
function decathlon_dpi_set_multiplier($tid, $value) {

  db_merge('decathlon_dpi')
    ->key(array(
        'event' => decathlon_get_event(),
        'tid' => $tid,
      ))
    ->fields(array(
        'multiplier' => $value,
      ))
    ->execute();
}

/**
 * an estimate of the likelihood of a player getting points 1 through n 
 */
function decathlon_dpi_get_player_multipliers($gid, $pid, $p) {

  // multipliers for player
  $pm = decathlon_dpi_load_player_multipliers();

  // return a multiplier array
  $multiplier = array(
    'value' => 0,
    'string' => "",
    'global' => 0,
  );

  // if the player has not played or has only played a little, use a global average
  // playing "a little" is determined by comparing the player's aggregate scoring in the game
  // to the perfect score for that game, which is simply an integer stored as a field in game taxonomy
  if (!isset($pm[$gid][$pid]['total']) || $pm[$gid][$pid]['total'] < decathlon_perfect_score($gid)) {

    // global multipliers: substitute these when player has none
    $gm = decathlon_dpi_load_global_multipliers($gid); 

    if (isset($gm[$p])) {
      $multiplier['value'] = $gm[$p];
      $multiplier['string'] = "(g)";
      $multiplier['global'] = 1;
    }

  } elseif (isset($pm[$gid][$pid][$p])) {

    // if the player has played more than a little then use the player's own data record
    $multiplier['value'] = $pm[$gid][$pid][$p];
  }

  return $multiplier;
}

function decathlon_dpi_load_player_multipliers() {
  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map)) {

    $data = decathlon_dpi_get_points();

    foreach ($data as $gid => $players) {
      foreach ($players as $pid => $points) {

        // sum the total count of point levels earned by this player
        $total = 0;
        foreach ($points as $value => $count) {
          $total += $value * $count;
        }
        $my_map[$gid][$pid]['total'] = $total;

        // walk through the list and calculate the average number of times
        // player earns the points at each level

        // get last key
        end($points);
        $limit = key($points);
        reset($points);

        // at each level count the points at and above and then divide by the total
        // for instance, you score 4 points when you score 4, 5, or 6 points
        for ($i = 1; $i <= $limit; $i++) {
          $count = 0;
          for ($j = $i; $j <= $limit; $j++) {
            $count += isset($points[$j]) ? $points[$j] : 0;
          }
          $my_map[$gid][$pid][$i] = round(($count/$total), 2);
        }
      }
    }
  }

  return $my_map;
}

/**
 * when there is not enough data to calculate the likelihood for a player, build one from all players
 */
function decathlon_dpi_load_global_multipliers($gid) {
  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map)) {

    $data = decathlon_dpi_get_points();
    $new = array();
    foreach ($data as $gid => $players) {
      foreach ($players as $pid => $points) {
        foreach ($points as $value => $count) {
          if (!isset($new[$gid][$value])) {
            $new[$gid][$value] = 0; // initialize
          }
          $new[$gid][$value] += $count; // add
        }
      }
    }

    // sort the keys of each of the subsets so they start with 1 and end with the highest number
    foreach ($new as $gid => $set) {
      ksort($set);
      $new[$gid] = $set;
    }

    foreach ($new as $gid => $points) {

      // sum the total points
      $total = 0;
      foreach ($points as $value => $count) {
        $total += $count;
      }

      // get last key
      end($points);
      $limit = key($points);
      reset($points);

      // at each level count the points at and above and then divide by the total
      // for instance, you score 4 points when you score 4, 5, or 6 points
      for ($i = 1; $i <= $limit; $i++) {
        $count = 0;
        for ($j = $i; $j <= $limit; $j++) {
          $count += isset($points[$j]) ? $points[$j] : 0;
        }

        $my_map[$gid][$i] = $count == 0 ? 0 : round (($count/$total), 2);
      }
    }
  }

  if (isset($my_map[$gid])) {
    return $my_map[$gid];
  }
}

/**
 * Helper function to load point totals from database
 */
function decathlon_dpi_get_points() {

  $data = &drupal_static(__FUNCTION__);

  if (!isset($data)) {
    $query = db_query("SELECT gid, pid, points, COUNT(sdp.contest) count FROM (SELECT contest AS contest, pid AS pid, SUM(points) AS points FROM decathlon_points GROUP BY pid, contest) AS sdp, decathlon_contests dc WHERE sdp.contest = dc.contest GROUP BY pid, gid, points", array())->fetchAll();

    // write query as associative array of game:player:points = number of times scored
    $data = array();
    foreach ($query as $row) {
      $data[$row->gid][$row->pid][$row->points] = $row->count;
    }

  }

  return $data;
}

