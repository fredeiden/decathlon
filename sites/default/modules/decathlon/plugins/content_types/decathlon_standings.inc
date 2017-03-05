<?php

$plugin = array(
  'title' => t('Decathlon Standings'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_standings_render',
  'edit form' => 'decathlon_standings_edit_form',
  'admin info' => 'decathlon_standings_admin_info',
);

function decathlon_standings_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 0:
        $title = t('MostP');
        break;
      case 1:
        $title = t('Unused');
        break;
      case 2:
        $title = t('Honors Standings');
        break;
      case 3:
        $title = t('Bonus Standings');
        break;
      case 4:
        $title = t('All Game Standings');
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

function decathlon_standings_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('MostP'),
      1 => t('Not in use'),
      2 => t('Honors Standings'),
      3 => t('Bonus Standings'),
      4 => t('All Game Standings'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Standings Table'),
  );

  return $form;
}

function decathlon_standings_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_standings_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');

  switch ($conf['type']) {
    case 0:
      $content = decathlon_standings_most_opponents();
      break;
    case 1:
      $content = '';
      break;
    case 2:
      $content = decathlon_standings_honors_leaders();
      break;
    case 3:
      $content = decathlon_standings_bonus_leaders();
      break;
    case 4:
      $content = decathlon_standings_game_leaders();
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

/**
 *
 */
function decathlon_standings_most_opponents() {

  $html = '';
  $groups = array('decathlon', 'leisure', 'parlor');
  foreach ($groups as $athlon) {

    // player opponents
    $opponents = decathlon_get_opponents($athlon);

    // sort descending
    uasort($opponents, function($a, $b) {
        return count($b) - count($a);
      });

    // build list
    $rows = array();
    foreach ($opponents as $index => $played) {
      $rows[] = 
        '<div class="name">' . strtoupper(decathlon_player_link($index)) . '</div>' . 
        '<div class = "count">' . (count($played)) . '</div>';
    }

    $html .= '<div class="leaderboard-mostp-standings">';
    $html .= '<h3>' . ucfirst($athlon) . '</h3>';
    $html .= theme('item_list', array(
              'items' => $rows, 
              'type' => 'ul', 
            ));
    $html .= '</div>';
  }    

  return $html;
}

/**
 *
 */
function decathlon_standings_honors_leaders() {

  // honors list for data handling and table header
  $tree = decathlon_standings_get_honors_tree();

  // honors table
  $records = db_query(
    "SELECT dc.format, dh.pid, dh.honor, dh.count FROM decathlon_honors dh, decathlon_contests dc WHERE dh.contest = dc.contest AND dc.event = :event", 
    array(
      ':event' => decathlon_get_event(),
    )
  )->fetchAll();

  // write honors totals to arrays
  $results = array();
  $totals = array();
  foreach ($records as $row) {

    $pid = $row->pid;
    $count = decathlon_count_honors($row->honor, $row->count);

    // count by player by type
    if (isset($results[$pid][$row->honor])) {
      $results[$pid][$row->honor] += $count;
    } else {
      $results[$pid][$row->honor] = $count;
    }

    // total by player
    if (isset($results[$pid]['total'])) {
      $results[$pid]['total'] += $count;
    } else {
      $results[$pid]['total'] = $count;
    }

    // total
    // when the format is 1 (doubles), only count 1/2 honor since it was a team effort
    if (isset($totals[$row->honor])) {
      $totals[$row->honor] += $row->format == 1 ? $count : $count;
    } else {
      $totals[$row->honor] = $row->format == 1 ? ($count / 2) : $count;
    }
  }

  return decathlon_standings_table($tree, $results, $totals);
}

/**
 *
 */
function decathlon_standings_bonus_leaders() {

  // honors list for data handling and table header
  $tree = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('honor')->vid); // bonus is called honor due to historical blunder

  // data
  // uses the Drupal node system, so use Views to pull rows
  $views_result = views_get_view_result("bonuses", "panel_pane_4", decathlon_get_event());

  $results = array();
  $totals = array();

  foreach ($views_result as $row) {

    $tid = $row->field_data_field_bonus_field_bonus_tid;           // bonus id
    $pid = $row->field_data_field_contestant_field_contestant_tid; // player id
    $points = $row->field_data_field_score_field_score_value;      // bonus points

    // player
    $results[$pid][$tid] = $points;
    
    // sum by player
    if (isset($results[$pid]['total'])) {
      $results[$pid]['total'] += $points;
    } else {
      $results[$pid]['total'] = $points;
    }

    // sum by bonus
    if (isset($totals[$tid])) {
      $totals[$tid] += $points;
    } else {
      $totals[$tid] = $points;
    }
  }

  return decathlon_standings_table($tree, $results, $totals, TRUE);
}

/**
 *
 */
function decathlon_standings_game_leaders() {

  $rankings = decathlon_get_all_rankings();
  $results = array();
  $html = ''; // string for theme output

  // rewrite rankings in new associative array
  foreach ($rankings as $row) {
    $game = decathlon_get_term_name($row->gid);
    $results[$game][$row->pid]['total'] = $row->points;
    $results[$game][$row->pid]['tid'] = $row->pid;
    $results[$game][$row->pid]['name'] = decathlon_get_term_name($row->pid);
  }

  // sort by game name ascending
  ksort($results);

    // add ranking
  foreach ($results as $game => $data) {
      
    // sort by total score descending
    usort($data, function($a, $b) { return $b['total'] - $a['total']; });
      
    // write rank
    $rank = 1;
    foreach ($data as $index => $row) {

      // watch for ties
      if (isset($previous_points) && $previous_points != $row['total']) {
        $rank = $index + 1; // next rank, otherwise a tie
      }
      // write rank
      $results[$game][$row['tid']]['rank'] = $rank;
      // compare scores
      $previous_points = $row['total'];
    }
  }

  foreach ($results as $game => $data) {

    // sort by total score descending
    usort($data, function($a, $b) { return $b['total'] - $a['total']; });
      
    $rows = array();

    foreach ($data as $index => $row) {
      if (isset($row['name'])) {
        // rank and name
        $rows[] =
          '<div class="rank">' . $row['rank'] . '</div>' .
          '<div class="name">' . decathlon_player_link($row['tid'], strtoupper($row['name'])) . '</div>';
      }
    }

    $html .= '<div class="leaderboard-game-standings">';
    $html .= '<h3>' . $game . '</h3>';
    $html .= theme(
      'item_list', 
      array(

        'items' => $rows, 
        'type' => 'ul', 
        'attributes' => array(),
      )
    );
    $html .= '</div>';
  }

  return $html;
}

/**
 *
 */
function decathlon_standings_table($tree, $results, $totals, $weighted = FALSE) {

  module_load_include('inc', 'decathlon_dpi');

  // build header and columns
  $groups = array();
  $header = array();
  $header[] = ''; // player name
  foreach ($tree as $row) {
    $groups[$row->tid] = $row->name;
    $header[] = array('data' => $row->name, 'field' => $row->tid);
  }
  $header[] = array('data' => 'Total', 'field' => 'total', 'sort' => 'desc');
  if ($weighted == TRUE) {
    $header[] = array('data' => 'Weighted');
  }

  // initialize empty results
  foreach (array_keys($results) as $pid) {
    foreach (array_keys($groups) as $tid) {
      if (!isset($results[$pid][$tid])) {
        $results[$pid][$tid] = 0;
      }
    }
  }

  // sort results array
  $order = tablesort_get_order($header);
  $sort = tablesort_get_sort($header);
  $results = decathlon_table_sort($results, $order['sql'], $sort);

  // totals row for table
  $rows = array();
  $cols = array();
  $sum = 0;
  $cols[] = 'TOTALS';
  foreach (array_keys($groups) as $tid) {
    $cols[] = isset($totals[$tid]) ? $totals[$tid] : 0;
    $sum += isset($totals[$tid]) ? $totals[$tid] : 0;
  }
  $cols[] = $sum;
  if ($weighted == TRUE) {
    $cols[] = '-';
  }
  $rows[] = $cols;

  // data rows for table
  if (isset($results)) {
    foreach ($results as $pid => $row) {
      $cols = array();
      $cols[] = decathlon_player_link($pid); // player name
      foreach (array_keys($groups) as $tid) {     // honors columns
        $cols[] = $row[$tid];
      }
      $cols[] = $row['total'];
      if ($weighted == TRUE) {
        $cols[] = ($row['total'] * decathlon_dpi_get_multiplier('Bonus'));
      }
      $rows[] = $cols;
    }
  }

  return theme('table', array('sticky' => TRUE, 'header' => $header, 'rows' => $rows));
}

/**
 *
 */
function decathlon_standings_get_honors_tree() {
  $tree = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load('bonus')->vid); // honor is called bonus due to historical blunder
  $games = decathlon_get_event_games();
  foreach ($tree as $index => $row) {
    $term = taxonomy_term_load($row->tid);
    $items = field_get_items('taxonomy_term', $term, 'field_game');
    $value = field_view_value('taxonomy_term', $term, 'field_game', $items[0]);
    if (!in_array($value['#title'], $games)) {
      unset($tree[$index]);
    }
  }
  return $tree;
}

