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
  $term = taxonomy_term_load($pid);
  $content .= '<div class="player-scorecard">';
  $content .= '<div class="player-image">';
  $content .= l(theme('image_style', array(
                  'style_name' => 'player_medium', 
                  'path' => !empty($term->field_picture['und'][0]['uri']) ? $term->field_picture['und'][0]['uri'] : 'public://default_images/yoda.jpg',
                )), 'scores/players/' . $pid, array('html' => TRUE));
  $content .= '</div>';
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

  foreach ($results['play'] as $athlon => $type) {

    if ($athlon_only != NULL && $athlon != $athlon_only) {
      continue;
    }

    // offense
    if (isset($results['offense'][$athlon]['game'])) {
      $offense = decathlon_game_icon($results['offense'][$athlon]['game']);
    } else {
      $offense = "?";
    }

    // defense
    if (isset($results['defense'][$athlon]['game'])) {
      $defense = decathlon_game_icon($results['defense'][$athlon]['game']);
    } else {
      $defense = "-";
    }

    // eligibility
    $eligibility = decathlon_get_leaderboard_eligibility($results, $athlon);

    $cols = array(
      ucfirst($athlon),
      decathlon_get_ordinal($ranking[$athlon]),
      $type['points'] + $type['rank'] + $bonus,
      decathlon_get_eligible_string($eligibility, 'qualified'),
      $offense,
      $defense,
    );
    $rows[] = array(
      'data' => $cols,
      'class' => array(),
    );

    $totals['points'] += $type['points'];
    $totals['rank'] += $type['rank'];
  }

  if ($athlon_only == NULL) {

    // offense
    if (isset($results['offense']['decathlon']['game'])) {
      $offense = decathlon_game_icon($results['offense']['decathlon']['game']);
    } else {
      $offense = "?";
    }

    // defense
    if (isset($results['defense']['decathlon']['game'])) {
      $defense = decathlon_game_icon($results['defense']['decathlon']['game']);
    } else {
      $defense = "-";
    }

    // eligibility
    $eligibility = decathlon_get_leaderboard_eligibility($results, 'decathlon');

    $rows[] = array(
      'data' => array(
        'Decathlon',
        decathlon_get_ordinal($ranking['decathlon']),
        $totals['points'] + $totals['rank'] + $bonus,
        decathlon_get_eligible_string($eligibility, 'qualified'),
        $offense,
        $defense,
      ),
      'class' => array(),
    );
  }

  $header = array(
    'Athlon',
    'Rank',
    'DPI',
    'Eligibility',
    'Offense',
    'Defense',
  );

  $html = theme('table', array('sticky' => FALSE, 'header' => $header, 'rows' => $rows, 'attributes' => array('class' => 'dpi-table')));
  $html .= '<div class="dpi-table table-footnote">' . $bonus . ' bonus points applied to DPI totals' . '</div>';

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
