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
  module_load_include('inc', 'decathlon_dpi');
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
      
      // suggestion
      $suggestion = array("-", "-", "-");
      // first column is simply 1 point
      isset($results[$pid]['log']) && $suggestion[0] = decathlon_easy_suggestion($results[$pid]['log'], $athlon);
      
      // second column is average of 2 through 1-from-end
      isset($results[$pid]['log']) && $suggestion[1] = decathlon_medium_suggestion($results[$pid]['log'], $athlon);

      // third column is end
      isset($results[$pid]['log']) && $suggestion[2] = decathlon_hard_suggestion($results[$pid]['log'], $athlon);

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
          '<div class="name">' . decathlon_player_link($pid) . '</div>' .
          '<div class="offense">' . $suggestion[0] . '</div>' .
          '<div class="offense">' . $suggestion[1] . '</div>' .
          '<div class="offense">' . $suggestion[2] . '</div>' .
          '<div class="offense">' . l("&#10142;", 'player/' . $pid . '/suggestions', array('html' => TRUE, 'attributes' => array('class' => array('highlight')))) . '</div>';
        
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
            '<div class="name">' . decathlon_player_link($pid) . '</div>' .
            '<div class="offense">' . $suggestion[0] . '</div>' .
            '<div class="offense">' . $suggestion[1] . '</div>' .
            '<div class="offense">' . $suggestion[2] . '</div>';
          
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
    '<li class="offense"> </li>' .
    '<li class="offense"> </li>' .
    '<li class="offense"> </li>' .
    '<li class="offense"> </li>' .
    '</ul>';

  $content = $header;
  if (empty($rows)) {
    $content .= "<div>No qualifiers</div>";
  } else {
    $content .= theme('item_list', array(
                  'items' => $rows, 
                  'type' => 'ul', 
                  'attributes' => array('class' => 'leaderboard-list'),
                ));
  }
  
  return $content;
}

