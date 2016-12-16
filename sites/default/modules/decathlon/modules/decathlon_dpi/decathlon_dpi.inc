<?php

function decathlon_dpi_get_player_points($gid, $pid) {
  $my_map = &drupal_static(__FUNCTION__);

  $limit = 6;  // max number of points

  if (!isset($my_map)) {
    $data = decathlon_dpi_get_points();
    foreach ($data as $gid => $players) {
      foreach ($players as $pid => $points) {
        $total = 0;
        foreach ($points as $amount) {
          $total += $amount;
        }
        for ($i = 1; $i <= $limit; $i++) {
          $count = 0;
          for ($j = $i; $j <= $limit; $j++) {
            $count += isset($points[$j]) ? $points[$j] : 0;
          }
          $my_map[$gid][$pid][$i] = $count == 0 ? '-' : round (($count/$total) * 100, 0) . '%';
          //$my_map[$gid][$pid][$i] = isset($points[$i]) ? $points[$i] : 0;
        }
      }
    }
  }

  if (!isset($my_map[$gid][$pid])) {
    return NULL;
  }
  return $my_map[$gid][$pid];
}

function decathlon_dpi_get_points() {

  $query = db_query("SELECT gid, pid, points, COUNT(sdp.contest) count FROM (SELECT contest AS contest, pid AS pid, SUM(points) AS points FROM decathlon_points GROUP BY pid, contest) AS sdp, decathlon_contests dc WHERE sdp.contest = dc.contest GROUP BY pid, gid, points", array())->fetchAll();

  // write query as associative array of game:player:points = number of times scored
  $data = array();
  foreach ($query as $row) {
    $data[$row->gid][$row->pid][$row->points] = $row->count;
  }

  return $data;
}

function decathlon_dpi_averages() {

  module_load_include('inc', 'decathlon');

  $players = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("players")->vid);
  $games = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("games")->vid);

  // write array of arrays for theme table
  $row = array();
  foreach ($players as $p) {
    $pid = $p->tid;
    $row = array();
    $row[] = decathlon_player_link($pid);
    foreach ($games as $g) {
      $gid = $g->tid;
      $row[] = theme('item_list', array(
                 'type' => 'ul', 
                 'attributes' => array('class' => array('table-cell-set')),
                 'items' => decathlon_dpi_get_player_points($gid, $pid),
               ));
    }
    $rows[] = $row;
  }
  
  // header array for theme table
  $header = array("");
  foreach ($games as $g) {
    $header[] = $g->name;
  }

  // table array
  return theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'sticky' => TRUE,
    ));
}