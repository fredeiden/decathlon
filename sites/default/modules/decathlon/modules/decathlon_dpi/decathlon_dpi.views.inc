<?php

/**
 * Implements hook_views_data()
 */
function decathlon_dpi_views_data() {
  $data = array();

  $data['decathlon_dpi']['table'] = array(
    'group' => t('Decathlon DPI'),
    'base' => array(
      'title' => t('DPI'),
    ),
    'join' => array(
      'taxonomy_term_data' => array(
        'left_field' => 'tid',
        'field' => 'tid',
      ),
    ),
  );

  $data['decathlon_dpi']['tid'] = array(
    'title' => t('Term ID'),
    'help' => t('A taxonomy term tid.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'relationship' => array(
      'base' => 'taxonomy_term_data',
      'field' => 'tid',
      'handler' => 'views_handler_relationship',
      'label' => t('Taxonomy Term'),
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_taxonomy',
      'numeric' => TRUE,
      'validate type' => 'tid',
    ),
  );

  $data['decathlon_dpi']['multiplier'] = array(
    'title' => t('Multiplier'),
    'help' => t('The value of the DPI multiplier.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
  );

  $data['decathlon_dpi']['event'] = array(
    'title' => t('Event'),
    'help' => t('The taxonomy term tid for the event.'),
    'field' => array(
      'handler' => 'views_handler_field_numeric',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_numeric',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_taxonomy',
      'numeric' => TRUE,
      'validate type' => 'tid',
    ),
  );

  return $data;
}
