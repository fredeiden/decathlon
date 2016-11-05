<?php


/**
 * @file
 * 
 * Copied from ctools_plugin_example simplecontext
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Contest"),
  'description' => t('A single decathlon contest.'),
  'context' => 'decathlon_context_create_contest',  // func to create context
  'context name' => 'contest',
  'settings form' => 'contest_settings_form',
  'keyword' => 'contest',

  // Provides a list of items which are exposed as keywords.
  'convert list' => 'contest_convert_list',
  // Convert keywords into data.
  'convert' => 'contest_convert',

  'placeholder form' => array(
    '#type' => 'textfield',
    '#description' => t('Enter contest number.'),
  ),
);

/**
 * Create a context, either from manual configuration or from an argument on the URL.
 *
 * @param $empty
 *   If true, just return an empty context.
 * @param $data
 *   If from settings form, an array as from a form. If from argument, a string.
 * @param $conf
 *   TRUE if the $data is coming from admin configuration, FALSE if it's from a URL arg.
 *
 * @return
 *   a Context object/
 */
function decathlon_context_create_contest($empty, $data = NULL, $conf = FALSE) {

  module_load_include('inc', 'decathlon');

  $context = new ctools_context('contest');
  $context->plugin = 'contest';

  if ($empty) {
    return $context;
  }

  if ($conf) {
    if (!empty($data)) {
      $context->data = new stdClass();
      return $context;
    }
  }
  else {
      // $data is coming from an arg - it's just a string.
      $record = db_query("SELECT event, gid, umpire, log FROM decathlon_contests WHERE contest = :contest", array(':contest' => $data))->fetchAssoc();

      $context->title = decathlon_get_term_name($record['gid']);
      $context->argument = $data;

      $context->data = new stdClass();
      $context->data->contest = $data;
      $context->data->event = $record['event'];
      $context->data->game_id = $record['gid'];
      $context->data->umpire = user_load($record['umpire'])->name;
      $context->data->log = $record['log'];

      return $context;
  }
}

function contest_settings_form($conf, $external = FALSE) {
  if (empty($conf)) {
    $conf = array(
      'contest_setting' => 'contest setting',
    );
  }
  $form = array();
  $form['contest_setting'] = array(
    '#type' => 'textfield',
    '#title' => t('Contest number'),
    '#size' => 50,
  );
  return $form;
}



/**
 * Provide a list of sub-keywords.
 *
 * This is used to provide keywords from the context for use in a content type,
 * pane, etc.
 */
function contest_convert_list() {
  return array(
    'contest' => t('Contest'),
    'event' => t('Event'),
    'game_id' => t('Game ID'),
    'umpire' => t('Umpire'),
    'log' => t('Log'),
  );
}

/**
 * Convert a context into a string to be used as a keyword by content types, etc.
 */
function contest_convert($context, $type) {
  switch ($type) {
    case 'contest':
      return $context->data->contest;
    case 'event':
      return decathlon_get_term_name($context->data->event);
    case 'game_id':
      return $context->data->game_id;
    case 'umpire':
      return $context->data->umpire;
    case 'log':
      return $context->data->log;
  }
}

