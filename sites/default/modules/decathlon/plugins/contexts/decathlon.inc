<?php


/**
 * @file
 * Decathlon ctools context type plugin
 *
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Decathlon"),
  'description' => t('A single "decathlon" context, or data element.'),
  'context' => 'decathlon_context_create_decathlon',
  'context name' => 'decathlon',
  'keyword' => 'decathlon',
  'convert list' => 'decathlon_convert_list',
  'convert' => 'decathlon_convert',
  'placeholder form' => array(
    '#type' => 'textfield',
    '#description' => t('Enter some data to represent this "decathlon".'),
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
function decathlon_context_create_decathlon($empty, $data = NULL, $conf = FALSE) {

  $context = new ctools_context('decathlon');
  $context->plugin = 'decathlon';

  if ($empty) {
    return $context;
  }

  if ($conf) {
    if (!empty($data)) {
      $context->data = new stdClass();
      $context->data->event = decathlon_get_event();
      $context->title = t("Decathlon context from config");
      return $context;
    }
  }
  else {
    // $data is coming from an arg - it's just a string.
    // This is used for keyword.
    $context->title = $data;
    $context->argument = $data;
    $context->data = new stdClass();
    $context->data->event = decathlon_get_event();
    $context->arg_length = strlen($context->argument);
    return $context;
  }
}

/**
 * Provide a list of sub-keywords.
 *
 * This is used to provide keywords from the context for use in a content type,
 * pane, etc.
 */
function decathlon_convert_list() {
  return array(
    'event' => t('Reporting Event'),
  );
}

/**
 * Convert a context into a string to be used as a keyword by content types, etc.
 */
function decathlon_convert($context, $type) {
  switch ($type) {
    case 'event':
      return $context->data->event;
  }
}

