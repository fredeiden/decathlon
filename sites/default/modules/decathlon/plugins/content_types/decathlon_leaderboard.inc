<?php

$plugin = array(
  'title' => t('Decathlon Leaderboard'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_leaderboard_render',
  'edit form' => 'decathlon_leaderboard_edit_form',
  'admin info' => 'decathlon_leaderboard_admin_info',
);

function decathlon_leaderboard_admin_info($subtype, $conf, $contexts) {
  switch ($conf['type']) {
    case 0:
      $title = t('Overall Leaderboard - All Players');
      break;
    case 1: 
      $title = t('Leisure Leaderboard - All Players');
      break;
    case 2:
      $title = t('Parlor Leaderboard - All Players');
      break;
    case 3:
      $title = t('Overall Leaderboard - Leaders Only');
      break;
    case 4: 
      $title = t('Leisure Leaderboard - Leaders Only');
      break;
    case 5:
      $title = t('Parlor Leaderboard - Leaders Only');
      break;
  }

  $block = new stdClass;
  $block->title = $title;
  return $block;
}

function decathlon_leaderboard_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');
  libraries_load('scrollbox');
  drupal_add_js(drupal_get_path('module', 'decathlon') . '/decathlon.js');

  switch ($conf['type']) {
    case 0:
      $content = decathlon_leaderboard_leaders();
      break;
    case 1:
      $content = decathlon_leaderboard_leaders("leisure");
      break;
    case 2:
      $content = decathlon_leaderboard_leaders("parlor");
      break;
    case 3:
      $content = decathlon_leaderboard_leaders("decathlon", "leaders");
      break;
    case 4:
      $content = decathlon_leaderboard_leaders("leisure", "leaders");
      break;
    case 5:
      $content = decathlon_leaderboard_leaders("parlor", "leaders");
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

function decathlon_leaderboard_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('Decathlon - All Players'),
      1 => t('Leisure - All Players'),
      2 => t('Parlor - All Players'),
      3 => t('Decathlon - Leaders Only'),
      4 => t('Leisure - Leaders Only'),
      5 => t('Parlor - Leaders Only'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('List Type'),
  );

  return $form;
}

function decathlon_leaderboard_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

/**
 *  Truncated display for all player scores.
 */
function decathlon_leaderboard_leaders($athlon = "decathlon", $version = "full") {

  $results = decathlon_get_rankings_all_sums();
  $sums = decathlon_get_ranking_points();

  // create list
  $rows = array();
  $i = 1;

  if (!empty($sums)) {
    foreach ($sums[$athlon] as $pid => $info) {

      // podium house rule
      $podium = FALSE;
      $podium_text = '';
      if (isset($info['podium'])) {
        $podium = TRUE;
        $podium_text = $info['podium'];
      }
      
      // eligibility
      if (isset($results[$pid]['play'])) {
        $eligibility = decathlon_get_leaderboard_eligibility($results[$pid], $athlon);
      } else {
        // this player only has bonus points
        $eligibility = 10;
      }
      
      // offense
      if (isset($results[$pid]['offense'][$athlon]['game'])) {
        $offense = decathlon_game_icon($results[$pid]['offense'][$athlon]['game']);
      } else {
        $offense = "?";
      }

      // defense
      if (isset($results[$pid]['defense'][$athlon]['game'])) {
        $defense = decathlon_game_icon($results[$pid]['defense'][$athlon]['game']);
      } else {
        $defense = "-";
      }

      $rank = '';
      $rank_class = 'rank podium';
      if ($podium == FALSE) {
        if ($eligibility == 0) {
          $rank = $i;
          if ($i <= 3) {
            $rank_class = 'rank podium';
          }
        }
      }

      if ($version == "full") {

        $data =
          (!empty($rank) ? '<div class="' . $rank_class . '"><span>' . decathlon_get_ordinal($rank) . '</span></div>' : decathlon_get_eligible_string($eligibility, $podium_text) ).
          '<div class="name">' . l(strtoupper(decathlon_get_term_name($pid)), 'players/player/' . $pid) . '</div>' .
          '<div class="offense">' . $offense . '</div>' .
          '<div class="defense">' . $defense . '</div>';
        
        $rows[] = array(
          "data" => $data,
        );
        
        if ($podium == FALSE && $eligibility == 0) {        
          $eligibility == 0 && $i++;
        }
      } else {
        if ($podium == FALSE && $eligibility == 0) {
          
          $data =
            (!empty($rank) ? '<div class="' . $rank_class . '"><span>' . decathlon_get_ordinal($rank) . '</span></div>' : decathlon_get_eligible_string($eligibility, $podium_text) ).
            '<div class="name">' . l(strtoupper(decathlon_get_term_name($pid)), 'players/player/' . $pid) . '</div>' .
            '<div class="offense">' . $offense . '</div>' .
            '<div class="defense">' . $defense . '</div>';

          $rows[] = array(
            "data" => $data,
          );
        
          $eligibility == 0 && $i++;
        }
      }
    }
  }

  // generate the header list manually because theme_item_list assigns item-list as class and
  // overriding with template.php is more work
  $header = 
    '<ul class="leaderboard-header">' .
    '<li class="rank">Rank</li>' .
    '<li class="name">Player</li>' .
    '<li class="offense">O</li>' .
    '<li class="defense">D</li>' .
    '</ul>';

  $content = $header;
  $content .= theme('item_list', array(
                'items' => $rows, 
                'type' => 'ul', 
                'attributes' => array('class' => 'leaderboard-list'),
              ));
  
  return $content;
}

