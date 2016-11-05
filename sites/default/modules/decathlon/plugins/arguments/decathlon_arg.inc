<?php

/**
 * @file
 *
 * Argument handler for a decathlon context
 *
 */

/**
 * Plugins are described by creating a $plugin array which will be used
 * by the system that includes this file.
 */
$plugin = array(
  'title' => t("Decathon arg"),
  // keyword to use for %substitution
  'keyword' => 'decathlon',
  'description' => t('Creates a "decathon" context from the arg.'),
  'context' => 'decathlon_arg_context',
  // placeholder_form is used in panels preview, for example, so we can
  // preview without getting the arg from a URL
  'placeholder form' => array(
    '#type' => 'textfield',
    '#description' => t('Enter the arg'),
  ),
);

function decathlon_arg_context($arg = NULL, $conf = NULL, $empty = FALSE) {
  $context = ctools_context_create('decathlon');
  return $context;
}
