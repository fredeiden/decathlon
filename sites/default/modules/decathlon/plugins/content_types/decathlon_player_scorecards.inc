<?php

$plugin = array(
  'title' => t('Decathlon Player Scorecards'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_player_scorecards_render',
  'edit form' => 'decathlon_player_scorecards_edit_form',
  'admin info' => 'decathlon_player_scorecards_admin_info',
);

function decathlon_player_scorecards_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 0:
        $title = t('Weighted');
        break;
      case 1:
        $title = t('Unweighted');
        break;
      case 2:
        $title = t('Single Player Totals');
        break;
      case 3:
        $title = t('Decathlon Player Totals');
        break;
      case 4:
        $title = t('Leisure Player Totals');
        break;
      case 5:
        $title = t('Parlor Player Totals');
        break;
      case 6:
        $title = t('Leisure Suggestion Log');
        break;
      case 7:
        $title = t('Parlor Suggestion Log');
        break;
      case 8:
        $title = t('Player Multipliers');
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

function decathlon_player_scorecards_edit_form($form, &$form_state) {

  $conf = $form_state['conf'];

  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('Weighted'),
      1 => t('Unweighted'),
      2 => t('One Player Totals'),
      3 => t('Decathlon Player Totals'),
      4 => t('Leisure Player Totals'),
      5 => t('Parlor Player Totals'),
      6 => t('Leisure Suggestion Log'),
      7 => t('Parlor Suggestion Log'),
      8 => t('Player Multipliers'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Point Type'),
  );

  return $form;
}

function decathlon_player_scorecards_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_player_scorecards_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');
  module_load_include('inc', 'decathlon_dpi');

  switch ($conf['type']) {
    case 0:
      $content = decathlon_player_scorecards_point_totals($args[0], 'weighted');
      break;
    case 1:
      $content = decathlon_player_scorecards_point_totals($args[0], 'unweighted');
      break;
    case 2:
      $content = decathlon_player_scorecards_player_totals($args[0]);
      break;
    case 3:
      $content = decathlon_player_scorecards_all_totals();
      break;
    case 4:
      $content = decathlon_player_scorecards_all_totals('leisure');
      break;
    case 5:
      $content = decathlon_player_scorecards_all_totals('parlor');
      break;
    case 6:
      $content = decathlon_player_scorecards_suggestion_log($args[0], 'leisure');
      break;
    case 7:
      $content = decathlon_player_scorecards_suggestion_log($args[0], 'parlor');
      break;
    case 8:
      $content = decathlon_player_scorecards_dpi_averages($args[0]);
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

/**
 *
 */
function decathlon_player_scorecards_point_totals($this_pid, $type) {
  $options = array(
    'leisure',
    'parlor',
  );
  
  $content = "";
  foreach ($options as $athlon) {
    $content .= decathlon_player_scorecards_point_table(decathlon_get_rankings_by_type($type), $this_pid, $athlon, $type);
  }

  return $content;
}

/**
 *
 */
function decathlon_player_scorecards_point_table($report, $this_pid, $athlon, $type) {

  $header = array(
    '', // no title for first column
    'Part',
    'Best',
    'Honor',
    'Skunk',
    'Points',
    'Rank',
  );

  $filter = decathlon_get_event_games($athlon); // only working on these games
  $exhibition = decathlon_get_event_games('exhibition'); // flag these games

  $rows = array();
  $sum = array(
    'Play' => 0,
    'Besting' => 0,
    'Honor' => 0,
    'Skunk' => 0,
    'Total' => 0,
    'Rank' => 0,
  );

  foreach ($filter as $gid => $name) {

    array_key_exists($gid, $exhibition) ? $suffix = "*" : $suffix = "";

    $cols = array();
    foreach ($report[$gid] as $index => $data) {
      if ($data['player'] == $this_pid) {
        $cols = array(
          l(strtoupper(substr(decathlon_get_term_name($gid), 0, 4)) . $suffix, 'scores/games/' . $gid),
          isset($data['Play']) ? $data['Play'] : 0,
          isset($data['Besting']) ? $data['Besting'] : 0,
          isset($data['Honor']) ? $data['Honor'] : 0,
          isset($data['Skunk']) ? $data['Skunk'] : 0,
          isset($data['Total']) ? $data['Total'] : 0,
          isset($data['Rank']) ? $data['Rank'] : 0,
        );
        $rows[] = array('data' => $cols);
        isset($data['Play']) && $sum['Play'] += $data['Play'];
        isset($data['Besting']) && $sum['Besting'] += $data['Besting'];
        isset($data['Honor']) && $sum['Honor'] += $data['Honor'];
        isset($data['Skunk']) && $sum['Skunk'] += $data['Skunk'];
        isset($data['Total']) && $sum['Total'] += $data['Total'];
        isset($data['Rank']) && $sum['Rank'] += $data['Rank'];
      }
    }
    if (count($cols) == 0) { // this game has not been played
      $rows[] = array(
        l(strtoupper(substr(decathlon_get_term_name($gid), 0, 4)) . $suffix, 'scores/games/' . $gid),
        0,
        0,
        0,
        0,
        0,
        '-',
      );
    }
  }

  $cols = array(
    "TOTAL",
    $sum['Play'],
    $sum['Besting'],
    $sum['Honor'],
    $sum['Skunk'],
    $sum['Total'],
    $type == 'weighted' ? $sum['Rank'] : "",
  );
  $rows[] = array('data' => $cols);

  $content = theme('table', array('sticky' => FALSE, 'caption' => strtoupper($athlon), 'header' => $header, 'rows' => $rows, 'attributes' => array('class' => array('athlon-table'))));

  return $content;
}

/**
 *
 */
function decathlon_player_scorecards_all_totals($athlon = NULL) {

  $results = decathlon_get_rankings_all_sums();
  if ($athlon == NULL) {
    uasort($results, function($a, $b) {
        return (decathlon_player_decathlon_dpi($b) - decathlon_player_decathlon_dpi($a)); 
      });
  } else {
    uasort($results, function($a, $b) use ($athlon) {
        return (decathlon_player_athlon_dpi($b, $athlon) - decathlon_player_athlon_dpi($a, $athlon)); 
      });
  }

  $content = "";
  foreach ($results as $pid => $totals) {
    $content .= decathlon_player_scorecards_player_totals($pid, $athlon);
  }
  return $content;
}

/**
 *
 */
function decathlon_player_scorecards_player_totals($pid, $athlon = NULL) {
  $content = "";
  $content .= '<div class="player-scorecard">';
  $content .= decathlon_player_scorecards_scorecard($pid, $athlon);
  $content .= '</div>';

  return $content;
}

/**
 *
 */
function decathlon_player_scorecards_scorecard($this_pid, $athlon_only = NULL) {

  $sums = decathlon_get_ranking_points();
  $results = decathlon_get_rankings_player_sums($this_pid);
  module_load_include('inc', 'decathlon', 'plugins/content_types/decathlon_leaderboard');

  $ranking = array();
  foreach ($sums as $group => $scores) {
    $i = 1;
    foreach ($scores as $player => $data) {
      if ($player == $this_pid) {
        $ranking[$group] = $i;
        break;
      }
      $i++;
    }
  }

  isset($results['bonus']) ? $bonus = $results['bonus'] : $bonus = 0; 

  $totals = array(
    'points' => 0,
    'rank' => 0,
  );

  $rows = array();

  if (isset($results['play'])) {
    foreach ($results['play'] as $athlon => $type) {

      if ($athlon_only != NULL && $athlon != $athlon_only) {
        continue;
      }

      // eligibility
      $eligibility = decathlon_get_leaderboard_eligibility($results, $athlon);
      
      $cols = array(
        ucfirst($athlon),
        decathlon_get_ordinal($ranking[$athlon]),
        $type['points'] + $type['rank'] + $bonus,
        decathlon_get_eligible_string($eligibility, 'qualified'),
        isset($results['log']) ? decathlon_easy_suggestion($results['log'], $athlon) : "-",
        isset($results['log']) ? decathlon_medium_suggestion($results['log'], $athlon) : "-",
        isset($results['log']) ? decathlon_hard_suggestion($results['log'], $athlon) : "-",
      );
      $rows[] = array(
        'data' => $cols,
        'class' => array(),
      );

      $totals['points'] += $type['points'];
      $totals['rank'] += $type['rank'];
    }
  }

  if ($athlon_only == NULL) {

    // eligibility
    $eligibility = decathlon_get_leaderboard_eligibility($results, 'decathlon');

    $rows[] = array(
      'data' => array(
        'Decathlon',
        isset($ranking['decathlon']) ? decathlon_get_ordinal($ranking['decathlon']) : "-",
        $totals['points'] + $totals['rank'] + $bonus,
        decathlon_get_eligible_string($eligibility, 'qualified'),
        isset($results['log']) ? decathlon_easy_suggestion($results['log'], 'decathlon') : "-",
        isset($results['log']) ? decathlon_medium_suggestion($results['log'], 'decathlon') : "-",
        isset($results['log']) ? decathlon_hard_suggestion($results['log'], 'decathlon') : "-",
      ),
      'class' => array(),
    );
  }

  $header = array(
    'Athlon',
    'Rank',
    'DPI',
    'Eligibility',
    '',
    '',
    '',
  );

  $html = theme('table', array('sticky' => FALSE, 'header' => $header, 'rows' => $rows, 'attributes' => array('class' => 'dpi-table')));
  $html .= '<div class="dpi-table table-footnote">' . $bonus . ' ' . decathlon_player_link($this_pid, 'bonus points applied to DPI totals') . '</div>';

  return $html;
}

/**
 *
 */
function decathlon_player_scorecards_get_eligible_cell($num) {
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
function decathlon_player_scorecards_suggestion_log($pid, $athlon) {

  // calculation factors are stored in the ranking array
  $results = decathlon_get_rankings_player_sums($pid);  
  $log = $results['log'];

  // only show results for this athlon
  $games = decathlon_get_event_games($athlon);

  // trim log to athlon games and sort
  $my_log = array();
  foreach ($games as $gid => $name) {
    if (isset($log[$gid])) {
      $my_log[$gid] = $log[$gid];
    }
  }

  uksort($my_log, function($a, $b) {
      return strcmp(decathlon_get_term_name($a), decathlon_get_term_name($b));
    });

  $suggestions = array(
    decathlon_easy_suggestion($log, $athlon),
    decathlon_medium_suggestion($log, $athlon),
    decathlon_hard_suggestion($log, $athlon),
  );
  $content = theme('item_list', array(
               'items' => $suggestions, 
               'type' => 'ul',
               'attributes' => array('class' => 'suggestions'),
             ));

  if (isset($my_log)) {
    foreach ($my_log as $gid => $point_list) {
      $rows = array();
      foreach ($point_list as $points => $factor_list) {
        $rows[] = array(
          $points, 
          $factor_list['spread'],
          $factor_list['bump'],
          $factor_list['multiplier'] . $factor_list['multiplier_suffix'],
          $factor_list['game_weight'],
          array('data' => $factor_list['result'], 'class' => array('highlight-col')),
        );
      }
  
      $content .= theme('table', array(
                    'rows' => $rows,
                    'caption' => l(decathlon_get_term_name($gid), 'scores/games/' . $gid),
                    'header' => array('Points', 'Spread', 'Bump', 'Likelihood', 'Game Weight', 'Result'),
                    'attributes' => array(
                      'class' => array('log-table')
                      ),
                    'sticky' => FALSE,
                  ));
    }
  }

  return $content;
}

/**
 *
 */
function decathlon_player_scorecards_dpi_averages($pid) {

  module_load_include('inc', 'decathlon_dpi');

  $games = decathlon_get_event_games();
  asort($games);
  $rows = array();

  // write array of arrays for theme table
  foreach ($games as $gid => $name) {
    $row = array();

    // first column: game name
    $row[] = decathlon_game_link($gid);

    for ($i = 1; $i <= 8; $i++) {
      $multiplier = decathlon_dpi_get_player_multipliers($gid, $pid, $i);

      // columns 2 through 8: multiplier
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
