<?php

$plugin = array(
  'title' => t('Decathlon Report'),
  'single' => TRUE,
  'category' => t('Decathlon'),
  'render callback' => 'decathlon_report_render',
  'edit form' => 'decathlon_report_edit_form',
  'admin info' => 'decathlon_report_admin_info',
);

function decathlon_report_admin_info($subtype, $conf, $contexts) {
  if (!empty($conf)) {
    switch ($conf['type']) {
      case 0:
        $title = 'Top Games';
        break;
      case 1:
        $title = 'Top Besters';
        break;
      case 2:
        $title = 'Top Bestees';
        break;
      case 3:
        $title = 'Top Participators';
        break;
      case 4:
        $title = 'Top Honors';
        break;
    }

    $block = new stdClass;
    $block->title = $title;
    return $block;
  }
}

function decathlon_report_render($subtype, $conf, $args, $context) {

  module_load_include('inc', 'decathlon');

  switch ($conf['type']) {
    case 0:
      $content = decathlon_report_top_games();
      break;
    case 1:
      $content = decathlon_report_top_besters();
      break;
    case 2:
      $content = decathlon_report_top_bestees();
      break;
    case 3:
      $content = decathlon_report_top_participators();
      break;
    case 4:
      $content = decathlon_report_top_honors();
      break;
  }

  $block = new stdClass();
  $block->content = $content;
  return $block;
}

function decathlon_report_edit_form($form, &$form_state) {
  $conf = $form_state['conf'];
  $form['type'] = array(
    '#type' => 'radios',
    '#options' => array(
      0 => t('Top Games'),
      1 => t('Top Besters'),
      2 => t('Top Bestees'),
      3 => t('Top Participators'),
      4 => t('Top Honors'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Report'),
  );

  return $form;
}

function decathlon_report_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_report_top_games() {
  $results = db_query("SELECT gid, COUNT(gid) AS total FROM {decathlon_contests} WHERE event = :event GROUP BY gid ORDER BY COUNT(gid) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();
  
  return decathlon_report_two_column_table($results, 'Top Games', array('Game', 'Plays'));
}

function decathlon_report_top_besters() {
  $results = db_query("SELECT bester, COUNT(bester) AS total FROM {decathlon_besting} WHERE event = :event GROUP BY bester ORDER BY COUNT(bester) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();

  return decathlon_report_two_column_table($results, 'Top Besters', array('Player', 'Bestings'));
}  
  
function decathlon_report_top_bestees() {
  $results = db_query("SELECT bestee, COUNT(bestee) AS total FROM {decathlon_besting} WHERE event = :event GROUP BY bestee ORDER BY COUNT(bestee) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();

  return decathlon_report_two_column_table($results, 'Top Bestees', array('Player', 'Bested-ings'));
}

function decathlon_report_top_participators() {
  $results = db_query("SELECT ds.pid, COUNT(ds.pid) AS total FROM {decathlon_scores} ds, decathlon_contests dc WHERE ds.contest = dc.contest AND dc.event = :event GROUP BY ds.pid ORDER BY COUNT(ds.pid) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();

  return decathlon_report_two_column_table($results, 'Top Participators', array('Player', 'Plays'));
}

function decathlon_report_top_honors() {
  $results = db_query("SELECT dh.honor, SUM(dh.count) AS total FROM {decathlon_honors} dh, decathlon_contests dc WHERE dh.contest = dc.contest AND dc.event = :event GROUP BY dh.honor ORDER BY COUNT(dh.honor) DESC", array(':event' => decathlon_get_event()))->fetchAllKeyed();

  return decathlon_report_two_column_table($results, 'Top Honors', array('Honor', 'Total'));
}

function decathlon_report_two_column_table($results, $caption, $header) {
  
  $rows = array();
  $total = 0;
  $i = 1;
  foreach ($results as $id => $count) {
    $rows[] = array($i, decathlon_get_term_name($id), $count);
    $total += $count;
    $i++;
  }
  $rows[] = array(
    'data' => array('', 'TOTAL', $total), 
    'class' => array('table-totals'),
  );

  $table = theme('table', array(
            'sticky' => TRUE, 
            'header' => array_merge(array(''), $header), 
            'rows' => $rows,
          ));

  return theme(
    'ctools_collapsible',
    array(
      'handle' => $caption,
      'content' => $table, 
      'collapsed' => TRUE
    )
  );
}
