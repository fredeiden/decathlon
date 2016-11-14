<?php

$plugin = array(
  'title' => t('Decathlon Player Standings'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_player_standings_render',
  'edit form' => 'decathlon_player_standings_edit_form',
  'admin info' => 'decathlon_player_standings_admin_info',
);

function decathlon_player_standings_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 0:
        $title = t('Player Participation');
        break;
      case 1:
        $title = t('Player Besting');
        break;
      case 2:
        $title = t('Player Honors');
        break;
      case 3:
        $title = t('Player Skunks');
        break;
      case 4:
        $title = t('Player Bested');
        break;
      case 5:
        $title = t('Player Game Summary');
        break;
      case 6:
        $title = t('Player Stats');
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

function decathlon_player_standings_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('Player Participation'),
      1 => t('Player Besting'),
      2 => t('Player Honors'),
      3 => t('Player Skunks'),
      4 => t('Player Bested'),
      5 => t('Player Game Summary'),
      6 => t('Player Stats'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Standings Table'),
  );

  return $form;
}

function decathlon_player_standings_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_player_standings_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');

  switch ($conf['type']) {
    case 0:
      $content = decathlon_player_standings_participation_table($args[0]);
      break;
    case 1:
      $content = decathlon_player_standings_besting_table($args[0], 'besting');
      break;
    case 2:
      $content = decathlon_player_standings_honors_table($args[0]);
      break;
    case 3:
      $content = decathlon_player_standings_skunks_table($args[0]);
      break;
    case 4:
      $content = decathlon_player_standings_besting_table($args[0], 'bested');
      break;
    case 5:
      $content = decathlon_player_standings_game_summary($args[0]);
      break;
    case 6:
      $content = decathlon_player_standings_stats_table();
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

/**
 *
 */
function decathlon_player_standings_participation_table($this_pid) {

  $event = decathlon_get_event();

  $header = array(
    array('data' =>'Game', 'field' => 'game'),
    array('data' => 'ID', 'field' => 'contest'),
    array('data' => 'Time', 'field' => 'time', 'sort' => 'asc'),
    array('data' => 'Players'),
  );

  $scores = db_query("SELECT dc.gid, dc.created, ds.contest FROM {decathlon_scores} ds, {decathlon_contests} dc  WHERE ds.contest = dc.contest AND dc.event = :event AND ds.pid = :pid", array(':event' => $event, ':pid' => $this_pid))->fetchAll();

  $table_data = array();

  foreach ($scores as $i => $data) {
    // get a list of players and scores for the contest
    $players = db_query("SELECT pid, score FROM {decathlon_scores} WHERE contest = :contest", array(':contest' => $data->contest))->fetchAllKeyed();

    // sort the list
    $high_score_wins = decathlon_high_score_wins($data->gid);
    if ($high_score_wins == 1) {
      arsort($players);
    } else {
      asort($players);
    }

    // write the list
    $chart = array();
    if (decathlon_doubles_game($data->gid, count($players))) {
      $i = 1;
      $team = array();
      foreach ($players as $pid => $score) {
        if ($pid == $this_pid) {
          $name = '<span class="this-player">' . decathlon_get_term_name($pid) . '</span>';
        } else {
          $name = decathlon_player_link($pid);
        }
        $team[] = $name;
        if ($i%2 == 0) {
          $chart[] = '<li><span class="player">' . implode(" / ", $team) . '</span><span class="score">' . $score . '</span></li>';
          $team = array();
        }
        $i++;
      }
    } else {
      foreach ($players as $pid => $score) {
        if ($pid == $this_pid) {
          $name = '<span class="this-player">' . decathlon_get_term_name($pid) . '</span>';
        } else {
          $name = decathlon_player_link($pid);
        }
        $chart[] = '<li><span class="player">' . $name . '</span><span class="score">' . $score . '</span></li>';
      }
    }

    $table_data[] = array(
      'game' => $data->gid,
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
          'data' => decathlon_game_link($row['game']),
          'style' => 'width: 12%',
        ),
        array(
          'data' => decathlon_contest_link($row['contest']),
          'style' => 'width: 5%',
        ),
        array(
          'data' => date('D g:i a', $row['time']),
          'style' => 'width: 10%',
        ),
        array(
          'data' => $row['players'],
          'style' => 'width: 65%',
        ),
      ),
    );
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'rows' => $rows,
      'header' => $header,
    ));
}

/**
 *
 */
function decathlon_player_standings_besting_table($pid, $type = 'besting') {

  $event = decathlon_get_event();

  if ($type == 'besting') {
    $records = db_query("SELECT game, bestee name FROM {decathlon_besting} WHERE event = :event AND bester = :pid", array(':event' => $event, ':pid' => $pid))->fetchAll();
  } else {
    $records = db_query("SELECT game, bester name FROM {decathlon_besting} WHERE event = :event AND bestee = :pid", array(':event' => $event, ':pid' => $pid))->fetchAll();
  }

  $rows = array();
  $groups = array();

  foreach ($records as $data) {
    $groups[$data->game][] = decathlon_get_term_name($data->name);
  }

  uksort($groups, function($a, $b) {
      return strcmp(decathlon_get_term_name($a), decathlon_get_term_name($b));
    });

  foreach ($groups as $game => $list) {
    sort($list);
    array_walk($list, function(&$val) {
        $val = decathlon_player_link_from_name($val);
      });
    $rows[] = array(
      array('data' => decathlon_game_link($game), 'class' => array('first')),
      implode(', ', $list),
    );
  }

  if (empty($rows)) {
    return 'No bestings';
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'rows' => $rows,
    ));
}

/**
 *
 */
function decathlon_player_standings_honors_table($pid) {

  $event = decathlon_get_event();

  $records = db_query("SELECT dc.gid, dh.contest, dh.honor, dh.count FROM {decathlon_honors} dh, {decathlon_contests} dc WHERE dh.contest = dc.contest AND dc.event = :event AND pid = :pid", array(':event' => $event, ':pid' => $pid))->fetchAll();

  $rows = array();

  $i = 1;
  foreach ($records as $data) {
    $rows[] = array(
      $i,
      decathlon_contest_link($data->contest),
      $data->count . " " . decathlon_get_term_name($data->honor),
    );
    $i++;
  }

  if (empty($rows)) {
    return 'No honors';
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'rows' => $rows,
    ));
}

/**
 *
 */
function decathlon_player_standings_skunks_table($pid) {

  $event = decathlon_get_event();

  $records = db_query("SELECT dc.gid, ds.contest FROM {decathlon_skunks} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.event = :event AND ds.pid = :pid", array(':event' => $event, ':pid' => $pid))->fetchAll();

  $rows = array();

  $i = 1;
  foreach ($records as $data) {
    $rows[] = array(
      $i,
      decathlon_contest_link($data->contest),
    );
    $i++;
  }

  if (empty($rows)) {
    return 'No skunks';
  }

  return theme('table', array(
      'sticky' => FALSE, 
      'rows' => $rows,
    ));
}

/**
 *
 */
function decathlon_player_standings_game_summary($pid) {

  // header array
  // includes keys (data, field, sort) for tablesort
  // also includes keys (lookup) for this function
  $header = array(
    'game' => array('data' => 'Game', 'convert' => 'decathlon_game_link_from_name'),
    'plays' => array('data' => 'Part', 'type' => 'lookup'),
    'minutes' => array('data' => 'Time', 'type' => 'lookup', 'convert' => 'decathlon_hours_and_minutes'),
    'players' => array('data' => 'Opponents', 'type' => 'lookup'),
    'avg' => array('data' => 'Avg Score', 'type' => 'lookup'),
    'besting' => array('data' => 'Besting', 'type' => 'lookup'),
    'bested' => array('data' => 'Bested', 'type' => 'lookup'),
    'honors' => array('data' => 'Honors', 'type' => 'lookup'),
    'bgpar' => array('data' => 'BgPar'),
    'bdpar' => array('data' => 'BdPar'),
    'hopar' => array('data' => 'HoPar'),
  );
  
  // copy header fields of type lookup to an array to capture values as we go
  $fields = array();
  foreach ($header as $key => $row) {
    if (isset($row['type']) && $row['type'] == 'lookup') {
      $fields[$key] = 0; // initialize
    }
  }
  $athlon_data = array(
    'leisure' => $fields,
    'parlor' => $fields,
    'decathlon' => $fields,
  );
  
  // query
  $game_data = array();
  $results = db_query("SELECT dc.gid gid, COUNT(dc.gid) AS total FROM {decathlon_contests} dc, {decathlon_scores} ds WHERE dc.contest = ds.contest AND dc.event = :event AND ds.pid = :pid GROUP BY dc.gid ORDER BY COUNT(dc.gid) DESC", array(':event' => decathlon_get_event(), ':pid' => $pid))->fetchAllKeyed();
  
  foreach ($results as $id => $plays) {
    
    // estimate of time played
    $minutes = ($plays * decathlon_dpi_get_participation_weighting($id));

    // unique players
    $opponents = db_query("SELECT DISTINCT scores.pid player FROM (SELECT dc.contest contest FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.gid = :gid AND dc.event = :event AND ds.pid = :pid) contests, {decathlon_scores} scores WHERE contests.contest = scores.contest AND scores.pid != :pid", array(':gid' => $id, ':event' => decathlon_get_event(), ':pid' => $pid))->fetchAll();
    //dsm($opponents);
    $players = count($opponents);
    
    // scores
    $avg = db_query("SELECT AVG(ds.score) average FROM {decathlon_scores} ds, {decathlon_contests} dc WHERE ds.contest = dc.contest AND dc.gid = :gid AND dc.event = :event AND ds.pid = :pid", array(':gid' => $id, ':event' => decathlon_get_event(), ':pid' => $pid))->fetchField();
    $avg = round($avg, 1);
    
    // honors
    $honors = db_query("SELECT SUM(dp.points) FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.gid = :gid AND dc.event = :event AND dp.point_type = :type AND dp.pid = :pid", array(':gid' => $id, ':event' => decathlon_get_event(), ':type' => 77, ':pid' => $pid))->fetchField();
    if (empty($honors)) {
      $honors = 0;
    }
    
    //besting
    $besting = db_query("SELECT count(bester) total FROM {decathlon_besting} WHERE event = :event AND game = :gid AND bester = :pid", array(':event' => decathlon_get_event(), ':gid' => $id, ':pid' => $pid))->fetchField();
    
    //bested
    $bested = db_query("SELECT count(bestee) total FROM {decathlon_besting} WHERE event = :event AND game = :gid AND bestee = :pid", array(':event' => decathlon_get_event(), ':gid' => $id, ':pid' => $pid))->fetchField();
    
    // assign data values to fields and write rows
    $set = array();
    $type = decathlon_game_type($id);
    foreach (array_keys($fields) as $field) {
      $value = ${$field};  // oh so ugly
      $set[$field] = $value;  // for table row              
      $athlon_data[$type][$field] += $value;  // for athlon subtotal
      $athlon_data['decathlon'][$field] += $value; // for overall subtotal
    }
    
    $game_data[] = array_merge(
      array('game' => decathlon_get_term_name($id)),
      $set,
      array(
        'bgpar' => round(($besting/$plays), 1),
        'bdpar' => round(($bested/$plays), 1),
        'hopar' => round(($honors/$plays), 1),
      ));
  }

  $content = decathlon_table($header, $game_data);

  // athlon table
  $rows = array();
  foreach ($athlon_data as $athlon => $data) {
    $rows[] = array(
      'data' => array(
        strtoupper($athlon),
        $data['plays'],
        decathlon_hours_and_minutes($data['minutes']),
        '<a href="/leaderboard/mostp">MostP</a>',
        '-',
        $data['besting'],
        $data['bested'],
        $data['honors'],
        $data['plays'] > 0 ? round(($data['besting']/$data['plays']), 1) : '-',
        $data['plays'] > 0 ? round(($data['bested']/$data['plays']), 1) : '-',
        $data['plays'] > 0 ? round(($data['honors']/$data['plays']), 1) : '-',
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

/**
 *
 */
function decathlon_player_standings_stats_table() {

  $header = array(
    'player' => array('data' => 'Player', 'convert' => 'decathlon_player_link_from_name'),
    'decathlon' => array('data' => 'Decathlon', 'sort' => 'desc'),
    'leisure' => array('data' => 'Leisure'),
    'parlor' => array('data' => 'Parlor'),
    'time' => array('data' => 'Time', 'convert' => 'decathlon_hours_and_minutes'),
    'opponents' => array('data' => 'MostP'),
    'play' => array('data' => 'Part'), 
    'besting' => array('data' => 'Besting'),
    'bested' => array('data' => 'Bested'),
    'honor' => array('data' => 'Honor'),
    'bgpar' => array('data' => 'BgPar'),
    'bdpar' => array('data' => 'BdPar'),
    'hopar' => array('data' => 'HoPar'),
  );

  $dpi = decathlon_get_ranking_points();
  $points = decathlon_player_standings_get_points();
  $opponents = decathlon_get_opponents('decathlon');
  $table_data = array();

  foreach (array_keys($dpi['decathlon']) as $pid) {

    $bested = db_query(
      "SELECT count(bestee) total FROM {decathlon_besting} WHERE event = :event AND bestee = :pid", 
      array(':event' => decathlon_get_event(), ':pid' => $pid)
    )->fetchField();

    $table_data[] = array(
      'player' => decathlon_get_term_name($pid),
      'decathlon' => $dpi['decathlon'][$pid]['dpi'],
      'leisure' => isset($dpi['leisure'][$pid]['dpi']) ? $dpi['leisure'][$pid]['dpi'] : 0,
      'parlor' => isset($dpi['parlor'][$pid]['dpi']) ? $dpi['parlor'][$pid]['dpi'] : 0,
      'time' => isset($points[$pid]['time']) ? $points[$pid]['time'] : 0,
      'opponents' => isset($opponents[$pid]) ? count($opponents[$pid]) : 0,
      'play' => isset($points[$pid]['play']) ? $points[$pid]['play'] : 0,
      'besting' => isset($points[$pid]['besting']) ? $points[$pid]['besting'] : 0,
      'bested' => isset($bested) ? $bested : 0,
      'honor' => isset($points[$pid]['honor']) ? $points[$pid]['honor'] : 0,
      'bgpar' => isset($points[$pid]['besting']) ? round(($points[$pid]['besting']/$points[$pid]['play']), 1) : 0,
      'bdpar' => isset($bested) ? round(($bested/$points[$pid]['play']), 1) : 0,
      'hopar' => isset($points[$pid]['honor']) ? round(($points[$pid]['honor']/$points[$pid]['play']), 1) : 0,
    );
  }

  return decathlon_table($header, $table_data);
}

/**
 * Query for player, game, and points
 */
function decathlon_player_standings_get_points() {
  $results = db_query("SELECT dp.pid, dc.gid, dp.point_type, SUM(dp.points) points FROM {decathlon_points} dp, {decathlon_contests} dc WHERE dp.contest = dc.contest AND dc.event = :event GROUP BY dp.pid, dc.gid, dp.point_type", array(':event' => decathlon_get_event()));
  $points = array();
  foreach ($results as $row) {
    // point types
    $type = strtolower(decathlon_get_term_name($row->point_type));
    if (isset($points[$row->pid][$type])) {
      $points[$row->pid][$type] += $row->points;
    } else {
      $points[$row->pid][$type] = $row->points;
    }
    // time estimate from participation points
    if ($type == 'play') {
      $time = ($row->points * decathlon_dpi_get_participation_weighting($row->gid));
      if (isset($points[$row->pid]['time'])) {
        $points[$row->pid]['time'] += $time;
      } else {
        $points[$row->pid]['time'] = $time;
      }
    }
  }
  return $points;
}

