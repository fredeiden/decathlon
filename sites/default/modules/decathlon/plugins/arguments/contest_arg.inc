<?php

/**
 * @file
 *
 * Copied from ctools_plugin_example simplecontext.
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Decathlon contest number"),
  'keyword' => 'contest',
  'description' => t('Creates a contest from the contest number.'),
  'context' => 'contest_arg_context',
  'placeholder form' => array(
    '#type' => 'textfield',
    '#description' => t('Enter the contest number'),
  ),
);

/**
 * Get the contest context using the arg.
 */
function contest_arg_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  if ($empty) {
    return ctools_context_create_empty('contest');
  }
  // Do whatever error checking is required, returning FALSE if it fails the test
  // Normally you'd check
  // for a missing object, one you couldn't create, etc.
  if (empty($arg)) {
    return FALSE;
  }
  return ctools_context_create('contest', $arg);
}
