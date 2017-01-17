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
        $title = 'Umps';
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
      $content = decathlon_report_umps();
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
      0 => t('Umps'),
    ),
    '#default_value' => isset($conf['type']) ? $conf['type'] : 0,
    '#title' => t('Report'),
  );

  return $form;
}

function decathlon_report_edit_form_submit($form, &$form_state) {
  $form_state['conf']['type'] = $form_state['values']['type'];
}

function decathlon_report_umps() {

  $events = taxonomy_get_tree(taxonomy_vocabulary_machine_name_load("event")->vid);
  $content = "";

  foreach ($events as $event) {

    $results = db_query("SELECT umpire, COUNT(umpire) AS total FROM {decathlon_contests} WHERE event = :event GROUP BY umpire", array(':event' => $event->tid))->fetchAllKeyed();

    $rows = array();
    $total = 0;
    $i = 1;
    foreach ($results as $ump => $count) {
      $user = user_load($ump);
      $rows[] = array($i, $user->name, $count);
      $total += $count;
      $i++;
    }
    $rows[] = array(
      'data' => array('', 'TOTAL', $total), 
      'class' => array('table-totals'),
    );

    $table = theme('table', array(
                  'sticky' => FALSE, 
                  'header' => array('', 'Umpire', 'Records'), 
                  'rows' => $rows,
                ));

    $content .= theme(
      'ctools_collapsible',
      array(
        'handle' => $event->name,
        'content' => $table, 
        'collapsed' => TRUE
      )
    );
  }

  return $content;
}
