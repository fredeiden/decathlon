<?php

function decathlon_dpi_get_player_multipliers($gid, $pid, $p) {
  $my_map = &drupal_static(__FUNCTION__);

  if (!isset($my_map)) {

    $data = decathlon_dpi_get_points();

    foreach ($data as $gid => $players) {
      foreach ($players as $pid => $points) {

        // sum the total points earned by this player
        $total = 0;
        foreach ($points as $amount) {
          $total += $amount;
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

  // return a multiplier
  // if the player has not played or has only played a little, use a global average
  $multiplier = array(
    'value' => 0,
    'string' => "",
    'global' => 0,
  );
  if (isset($my_map[$gid][$pid][$p])) {
    $multiplier['value'] = $my_map[$gid][$pid][$p];
  } else {
    if ($p <= 3) { // only use globals for 3 and under
      if (!isset($my_map[$gid][$pid]['total']) || $my_map[$gid][$pid]['total'] < 5) {
        $gm = decathlon_dpi_get_global_multipliers($gid); // substitute these when player has none
        if (isset($gm[$p])) {
          $multiplier['value'] = $gm[$p];
          $multiplier['string'] = "(g)";
          $multiplier['global'] = 1;
        }
      }
    }
  }
  return $multiplier;
}

function decathlon_dpi_get_global_multipliers($gid) {
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

    foreach ($new as $gid => $points) {

      // sum the total points
      $total = 0;
      foreach ($points as $amount) {
        $total += $amount;
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

function decathlon_dpi_averages($pid) {

  $games = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("games")->vid);
  $rows = array();

  // write array of arrays for theme table
  foreach ($games as $g) {
    $row = array();
    $row[] = decathlon_game_link($g->tid);
    for ($i = 1; $i <= 8; $i++) {
      $multiplier = decathlon_dpi_get_player_multipliers($g->tid, $pid, $i);
      $row[] = $multiplier['value'] . $multiplier['string']; 
    }
    $rows[] = $row;
  }
  
  // header array for theme table
  $header = array("Game");
  for ($i = 1; $i <= 8; $i++) {
    $header[] = $i;
  }

  // table array
  return theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'sticky' => FALSE,
    ));
}