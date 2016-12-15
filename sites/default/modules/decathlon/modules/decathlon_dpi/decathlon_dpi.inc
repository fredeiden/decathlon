<?php

function decathlon_dpi_averages() {

  module_load_include('inc', 'decathlon');

  $query = db_query("SELECT gid, pid, AVG(points) avg, MAX(points) max FROM (SELECT contest AS contest, pid AS pid, SUM(points) AS points FROM decathlon_points GROUP BY pid, contest) AS sdp, decathlon_contests dc WHERE sdp.contest = dc.contest GROUP BY pid, gid", array())->fetchAll();

  // write query as associative array of game:player:average
  $players = array();
  foreach ($query as $row) {
    $data[$row->gid][$row->pid]['avg'] = $row->avg;
    $data[$row->gid][$row->pid]['max'] = $row->max;
    $players[decathlon_get_term_name($row->pid)] = $row->pid;
  }

  ksort($players);

  // write array of arrays for theme table
  $row = array();
  foreach ($players as $name => $pid) {
    $row = array();
    $row[] = decathlon_player_link($pid);
    foreach ($data as $gid => $totals) {
      $row[] = theme('item_list', array(
                 'type' => 'ul', 
                 'attributes' => array('class' => array('table-cell-set')),
                 'items' => array(
                   isset($totals[$pid]['avg']) ? round($totals[$pid]['avg'], 1) : '-', 
                   isset($totals[$pid]['max']) ? $totals[$pid]['max'] : '-', 
                 ),
               ));
    }
    $rows[] = $row;
  }
  
  // header array for theme table
  $header = array_merge(array(""), array_map(function($gid) {return decathlon_game_link($gid);}, array_keys($data)));

  // table array
  return theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'caption' => t('Player Historical Avg and Max Point'),
      'sticky' => TRUE,
    ));
}