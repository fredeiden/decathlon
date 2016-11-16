<?php

$plugin = array(
  'title' => t('Decathlon Game Report'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_game_report_render',
  'edit form' => 'decathlon_game_report_edit_form',
  'admin info' => 'decathlon_game_report_admin_info',
);

function decathlon_game_report_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 1:
        $title = t('Game History');
        break;
      case 2:
        $title = t('Game Leaders');
        break;
      case 3:
        $title = t('Single Game Record');
        break;
      case 4:
        $title = t('Game Summary');
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

function decathlon_game_report_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      1 => t('Game History'),
      2 => t('Game Leaders'),
      3 => t('Single Game Record'),
      4 => t('Game Summary'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Options'),
  );

  return $form;
}

function decathlon_game_report_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_game_report_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');

  switch ($conf['type']) {
    case 1:
      $content = decathlon_game_report_game_history($args[0]);
      break;
    case 2:
      $content = decathlon_game_report_game_leaders($args[0]);
      break;
    case 3:
      $content = decathlon_game_report_single_game_record($args[0]);
      break;
    case 4:
      $content = decathlon_game_report_game_summary();
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

/**
 *
 */
function decathlon_game_report_game_history($gid) {

  $event = decathlon_get_event();

  $header = array(
    array('data' => 'Game ID'),
    array('data' => 'Time', 'field' => 'time', 'sort' => 'asc'),
    array('data' => 'Players'),
  );

  $scores = db_query("SELECT dc.created, dc.contest FROM {decathlon_contests} dc  WHERE dc.event = :event AND dc.gid = :gid", array(':event' => $event, ':gid' => $gid))->fetchAll();

  $table_data = array();

  foreach ($scores as $i => $data) {
    // get a list of players and scores for the contest
    $players = db_query("SELECT pid, score FROM {decathlon_scores} WHERE contest = :contest", array(':contest' => $data->contest))->fetchAllKeyed();
    
    // sort the list
    $high_score_wins = decathlon_high_score_wins($gid);
    if ($high_score_wins == 1) {
      arsort($players);
    } else {
      asort($players);
    }

    // write the list
    $chart = array();
    if (decathlon_doubles_game($gid, count($players))) {
      $i = 1;
      $team = array();
      foreach ($players as $pid => $score) {
        $team[] = decathlon_player_link($pid);
        if ($i%2 == 0) {
          $chart[] = '<li><span class="player">' . implode(" / ", $team) . '</span><span class="score">' . $score . '</span></li>';
          $team = array();
        }
        $i++;
      }
    } else {
      foreach ($players as $pid => $score) {
        $chart[] = '<li><span class="player">' . decathlon_player_link($pid) . '</span><span class="score">' . $score . '</span></li>';
      }
    }
    
    $table_data[] = array(
      'contest' => $data->contest,
      'time' => $data->created,
      'players' => '<ul class="score-list">' . implode(" ", $chart) . '</ul',
    );
  }

  $order = tablesort_get_order($header);
  $sort = tablesort_get_sort($header);
  $table_data = decathlon_table_sort($table_data, $order['sql'], $sort);

  $rows = array();

  foreach ($table_data as $row) {
    // write table rows and embed a list in each row
    $rows[] = array(
      'data' => array(
        array(
          'data' => l($row['contest'], 'scores/game/' . $row['contest']),
          'style' => 'width: 9%',
        ),
        array(
          'data' => date('D g:i a', $row['time']),
          'style' => 'width: 10%',
        ),
        array(
          'data' => $row['players'],
          'style' => 'width: 80%',
        ),
      ),
    );
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'rows' => $rows,
      'header' => $header,
      'caption' => 'History',
    ));
}

/**
 *
 */
function decathlon_game_report_single_game_record($contest) {

  $scores = db_query("SELECT pid, score FROM {decathlon_scores} WHERE contest = :contest", array(':contest' => $contest))->fetchAllKeyed();
  $points = db_query("SELECT pid, points, point_type FROM {decathlon_points} WHERE contest = :contest", array(':contest' => $contest))->fetchAll();
  $types = decathlon_get_point_types();

  // populate assoc array as array[player][point type]
  $data = array();
  foreach ($points as $record) {
    $data[$record->pid][$record->point_type] = $record->points;
  }

  // build table rows
  $rows = array();
  foreach ($scores as $pid => $score) {
    // the first two columns: player, score
    $row = array(
      l(decathlon_get_term_name($pid), 'players/player/' . $pid),
      $score,
    );
    // the next four columns: play, besting, skunk, honor
    foreach ($types as $i => $name) {
      if (isset($data[$pid][$i])) {
        $row[] = $data[$pid][$i];
      } else {
        $row[] = 0;
      }
    }
    // add new row
    $rows[] = $row;
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'header' => array_merge(array('', 'Score'), array_values($types)),
      'rows' => $rows,
    ));
}

/**
 *
 */
function decathlon_game_report_game_leaders($gid = NULL) {

  if ($gid == NULL) { return '<div class="choose-value">Select a game</div>'; }

  $games = decathlon_get_game_rankings($gid); 
  $types = decathlon_get_point_types();
  $results = array();
  $totals = array(
    'bested' => 0,
    'avg' => 0,
    'max' => 0,
    'min' => 1000000,
  );

  $header = array_merge(
    array(
      array('data' => 'Player'), 
      array('data' => 'Rank', 'field' => 'rank', 'sort' => 'asc'),
      array('data' => 'Time', 'field' => 'time'),
    ), 
    array_map(
      function($type) {return array('data' => $type == 'Participation' ? substr($type, 0, 4) : $type, 'field' => strtolower($type));},
      array_values($types)
    ),
    array(
      array('data' => 'Total', 'field' => 'total'),
      array('data' => 'Bested', 'field' => 'bested'),
      array('data' => 'Avg Score', 'field' => 'avg'),
      array('data' => 'Max Score', 'field' => 'max'),
      array('data' => 'Min Score', 'field' => 'min'),
      array('data' => 'BgPaR', 'field' => 'bgpar', 'title' => 'Besting-to-Participation Ratio'),
      array('data' => 'BdPaR', 'field' => 'bdpar', 'title' => 'Bested-to-Participation Ratio'),
      array('data' => 'HoPaR', 'field' => 'hopar', 'title' => 'Honors-to-Participation Ratio'),
    ));

  $table_data = array();
  foreach ($games as $row) {
    $tid = $row->pid;
    $type = decathlon_get_term_name($row->point_type);
    $score = $row->points;
    
    $results[$tid]['tid'] = $tid;
    $results[$tid]['name'] = decathlon_get_term_name($tid);
    
    // scores by player by type
    if (isset($results[$tid][$row->point_type])) {
      $results[$tid][$row->point_type] += $score;
    } else {
      $results[$tid][$row->point_type] = $score;
    }
    
    // totals by player
    if (isset($results[$tid]['total'])) {
      $results[$tid]['total'] += $score;
    } else {
      $results[$tid]['total'] = $score;
    }
    
    // totals by  type
    if (empty($totals[$row->point_type])) {
      $totals[$row->point_type] = $score;
    } else {
      $totals[$row->point_type] += $score;
    }
  }

  // sort by total score descending
  usort($results, function($a, $b) { return $b['total'] - $a['total']; });

  // write rank
  $rank = 1;
  foreach ($results as $index => $row) {
    // watch for ties
    if (isset($previous_points) && $previous_points != $row['total']) {
      $rank = $index + 1; // next rank, otherwise a tie
    }
    // write rank
    $results[$index]['rank'] = $rank;
    // compare scores
    $previous_points = $row['total'];
  }

  // write table data
  foreach ($results as $index => $row) {

    // point columns
    $points = array();
    foreach ($types as $tid => $name) {
      $points[strtolower($name)] = (isset($row[$tid]) ? $row[$tid] : 0);
    }

    $bested = decathlon_get_player_game_bested($row['tid'], $gid)->bested;
    $totals['bested'] += $bested;

    $scores = decathlon_get_player_game_scores($row['tid'], $gid);
    $totals['avg'] += round($scores->avg);
    $scores->max > $totals['max'] && $totals['max'] = $scores->max;
    $scores->min < $totals['min'] && $totals['min'] = $scores->min;

    $minutes = ($points['participation'] * decathlon_dpi_get_participation_weighting($gid));

    if (isset($row['name'])) {
      $table_data[] = array_merge(
        array(
          'name' => decathlon_player_link($row['tid']),
          'rank' => $row['rank'],
          'time' => $minutes,
        ),
        $points,
        array(
          'total' => $row['total'],
          'bested' => $bested,
          'avg' => $cols[] = round($scores->avg),
          'max' => $cols[] = $scores->max,
          'min' => $cols[] = $scores->min,
          // ratios (cheating on these indexes)
          'bgpar' => isset($row[74]) ? round($row[74]/$row[73], 1) : 0,
          'bdpar' => round($bested/$row[73], 1),
          'hopar' => isset($row[77]) ? round($row[77]/$row[73], 1) : 0,
        ));
    }
  }

  $rows = array();

  // totals row
  $cols = array();
  $cols[] = 'TOTALS';
  $cols[] = count($table_data);
  $cols[] = '';
  foreach ($types as $tid => $name) {
    $cols[] = (isset($totals[$tid]) ? $totals[$tid] : 0);
  }
  $cols[] = '-';
  $cols[] = $totals['bested'];
  $cols[] = !empty($table_data) ? round(($totals['avg']/count($table_data))) : 0;
  $cols[] = $totals['max'];
  $cols[] = $totals['min'];
  $cols[] = '';
  $cols[] = '';
  $cols[] = '';
  $rows[] = array('data' => $cols, 'class' => array('totals-row'));

  // sort
  $order = tablesort_get_order($header);
  $sort = tablesort_get_sort($header);
  $table_data = decathlon_table_sort($table_data, $order['sql'], $sort);

  // write table rows
  foreach ($table_data as $row) {
    $rows[] = array(
      $row['name'],
      $row['rank'],
      decathlon_hours_and_minutes($row['time']),
      $row['participation'],
      $row['besting'],
      $row['honor'],
      $row['skunk'],
      $row['total'],
      $row['bested'],
      $row['avg'],
      $row['max'],
      $row['min'],
      $row['bgpar'],
      $row['bdpar'],
      $row['hopar'],
    );
  }

  $html = theme('table', array('caption' => decathlon_get_term_name($gid), 'sticky' => FALSE, 'header' => $header, 'rows' => $rows));

  return $html;
}

/**
 *
 */
function decathlon_game_report_game_summary() {

  // define a header that can be used by tablesort (data, field, sort) and add a value for local use (type)
  $header = array(
    'game' => array('data' => 'Game', 'sort' => 'asc', 'convert' => 'decathlon_game_link_from_name'),
    'games' => array('data' => 'Games', 'type' => 'lookup'),
    'players' => array('data' => 'Players', 'type' => 'lookup'),
    'minutes' => array('data' => 'Time', 'type' => 'lookup'),
    'plays' => array('data' => 'Part', 'type' => 'lookup'),
    'besting' => array('data' => 'Besting', 'type' => 'lookup'),
    'honors' => array('data' => 'Honors', 'type' => 'lookup'),
    'skunks' => array('data' => 'Skunks', 'type' => 'lookup'),
    'average' => array('data' => 'Avg Score', 'type' => 'lookup', 'convert' => 'decathlon_round'),
    'max' => array('data' => 'Max Score', 'type' => 'lookup'),
    'min' => array('data' => 'Min Score', 'type' => 'lookup'),
    'bggam' => array('data' => 'BgGam'),
    'hogam' => array('data' => 'HoGam'),
  );
  // copy header fields of type lookup to an array to capture values as we go
  $fields = array();
  foreach ($header as $key => $row) {
    if (isset($row['type']) && $row['type'] == 'lookup') {
      $fields[$key] = 0;
    }
  }
  $athlon_data = array(
    'leisure' => $fields,
    'parlor' => $fields,
    'decathlon' => $fields,
  );

  // collect point totals per game
  $points = decathlon_get_points_games();

  // loop over query results and write/tabulate rows
  $game_data = array();
  $results = db_query("SELECT gid, COUNT(gid) AS total FROM {decathlon_contests} WHERE event = :event GROUP BY gid ORDER BY COUNT(gid) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();
  foreach ($results as $id => $games) {

    // estimate of time played
    $minutes = ($games * decathlon_dpi_get_participation_weighting($id));

    // distinct players
    $scores = db_query("SELECT COUNT(DISTINCT ds.pid) players, AVG(ds.score) average, MAX(ds.score) max, MIN(ds.score) min FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.gid = :gid AND dc.event = :event", array(':gid' => $id, ':event' => decathlon_get_event()))->fetchAll();
    $players = $scores[0]->players;
    $average = $scores[0]->average;
    $max = $scores[0]->max;
    $min = $scores[0]->min;

    // points
    $plays = isset($points[$id][73]) ? $points[$id][73] : 0; 
    $besting = isset($points[$id][74]) ? $points[$id][74] : 0; 
    $honors = isset($points[$id][77]) ? $points[$id][77] : 0; 
    $skunks = isset($points[$id][76]) ? $points[$id][76] : 0; 

    // assign data values to fields and write rows
    $set = array();
    $type = decathlon_game_type($id);
    foreach (array_keys($fields) as $field) {
      $value = ${$field};  // oh so ugly
      $set[$field] = $value;  // for table row              
      $athlon_data[$type][$field] += $value;  // for athlon subtotal
      $athlon_data['decathlon'][$field] += $value; // for overall subtotal
    }
    // write table rows
    $game_data[$id] = array_merge(
      array(
        'game' => decathlon_get_term_name($id)
      ),
      $set,
      array(
        'bggam' => round(($besting/$games), 1),
        'hogam' => round(($honors/$games), 1),
      )
    );
  }

  $content = decathlon_table($header, $game_data);

  //
  // athlon table
  //

  // distinct players athlons
  $unique_players = array();
  $query = db_query("SELECT COUNT(DISTINCT ds.pid) players, field_type_tid athlon FROM {decathlon_scores} ds, {decathlon_contests} dc, {field_data_field_type} ft WHERE ds.contest = dc.contest AND dc.gid = ft.entity_id AND dc.event = :event GROUP BY ft.field_type_tid", array(':event' => decathlon_get_event()))->fetchAllKeyed(1,0);
  foreach ($query as $athlon => $count) {
    $unique_players[strtolower(decathlon_get_term_name($athlon))] = $count;
  }
  // distinct players decathlon
  $unique_players['decathlon'] = db_query("SELECT COUNT(DISTINCT ds.pid) players FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.event = :event", array(':event' => decathlon_get_event()))->fetchField();

  $rows = array();
  foreach ($athlon_data as $athlon => $data) {
    $rows[] = array(
      'data' => array(
        strtoupper($athlon),
        $data['games'],
        isset($unique_players[$athlon]) ? $unique_players[$athlon] : 0,
        decathlon_hours_and_minutes($data['minutes']),
        $data['plays'],
        $data['besting'],
        $data['honors'],
        $data['skunks'],
        '-',
        '-',
        '-',
        $data['games'] > 0 ? round(($data['besting']/$data['games']), 1) : 0,
        $data['games'] > 0 ? round(($data['honors']/$data['games']), 1) : 0,
      ),
    'class' => array('totals-row'),
    );
  }

  $content .= theme('table', array(
            'sticky' => FALSE, 
            'rows' => $rows,
          ));

  return $content;
  
}


