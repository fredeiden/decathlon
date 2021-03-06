<?php

/**
 * Edit game players, scores, honors
 */
function decathlon_edit_edit_form($form, &$form_state, $game) {

  module_load_include('inc', 'decathlon');

  // load game ID
  $gid = db_query(
    "SELECT gid FROM {decathlon_contests} WHERE contest = :contest",
    array(':contest' => $game)
  )->fetchField();

  // load game scores
  $scores = db_query(
    "SELECT pid, score FROM {decathlon_scores} WHERE contest = :contest",
    array(':contest' => $game)
  )->fetchAll();

  // load made honors
  $honors_by_pid = array();
  $honors = db_query(
    "SELECT pid, honor, count FROM {decathlon_honors} WHERE contest = :contest",
    array(':contest' => $game)
  )->fetchAll();
  foreach ($honors as $row) {
    $honors_by_pid[$row->pid][$row->honor] = $row->count;
  }

  // load possible honors
  $honors_list = views_get_view_result('honors_list', 'panel_pane_1', $gid);

  // load players
  $view = views_get_view_result('players', 'panel_pane_2');
  $players = array();
  foreach ($view as $row) {
    $players[$row->tid] = $row->taxonomy_term_data_name;
  }

  $form['test'] = array(
    '#type' => 'item',
    '#markup' => 'Editing ' . decathlon_get_term_name($gid) . ' game number ' . $game,
  );

  $form['scores'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  foreach ($scores as $row) {
    $pid = $row->pid;
    $form['scores'][$pid] = array(
      '#type' => 'fieldset',
    );
    $form['scores'][$pid]['player'] = array(
      '#type' => 'select',
      '#title' => 'Player',
      '#options' => $players,
      '#default_value' => $pid,
    );
    $form['scores'][$pid]['score'] = array(
      '#type' => 'textfield',
      '#title' => 'Score',
      '#default_value' => $row->score,
      '#size' => 10,
    );
    if (count($honors_list) > 0) {
      foreach($honors_list as $h) {
        $honor = taxonomy_term_load($h->tid);
        if ($honor->field_auto['und'][0]['value'] == 1) {
          $form['scores'][$pid]['honors'][$h->tid] = array(
            '#type' => 'item',
            '#title' => t('Honor awarded automatically'),
          );
        } else {
          $options = array();
          for ($i = 0; $i <= $honor->field_max_per_game['und'][0]['value']; $i++) {
            $options[$i] = $i;
          }

          $form['scores'][$pid]['honors'][$h->tid] = array(
            '#type' => 'radios',
            '#title' => $honor->name,
            '#options' => $options,
            '#default_value' => isset($honors_by_pid[$pid][$h->tid]) ? $honors_by_pid[$pid][$h->tid] : 0,
            '#prefix' => '<div class="inline-radios">',
            '#suffix' => '</div>',
          );
        }
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit Changes',
  );

  return $form;
}

/**
 * Submit function for edit
 */
function decathlon_edit_edit_form_submit($form, &$form_state) {

  $contest = $form_state['build_info']['args'][0];
  $scores = $form_state['values']['scores'];
  $changed = FALSE;
  foreach ($scores as $pid => $data) {

    $new_fields = array();

    // Update the decathlon_scores table, which has the columns:
    //  contest
    //  pid
    //  score
    // If a player has been changed, update the player.
    // If a score has been changed, update the score.
    // Though unlikely, do them together in case both are being altered.
    if ($pid != $data['player'] || $data['score'] != $form['scores'][$pid]['score']['#default_value']) {
      if ($pid != $data['player']) {
        $new_fields['pid'] = $data['player'];
        drupal_set_message("Changed player from " . decathlon_player_link($pid) . ' to ' . decathlon_player_link($data['player']) );
      }
      if ($data['score'] != $form['scores'][$pid]['score']['#default_value']) {
        $new_fields['score'] = $data['score'];
        drupal_set_message("Changed score to " . $data['score']);
      }
      db_update('decathlon_scores')
        ->fields($new_fields)
        ->condition('contest', $contest, '=')
        ->condition('pid', $pid, '=')
        ->execute();
      $changed = TRUE;
    }

    // Update the decathlon_honors table, which has the columns:
    //  contest
    //  pid
    //  honor
    //  count
    // If an honor has changed or been added, merge the row.
    // If an honor has been set to 0, delete the row.
    // Watch out for player change.
    if (isset($data['honors'])) {
      foreach ($data['honors'] as $honor => $value) {
        if ($value != $form['scores'][$pid]['honors'][$honor]['#default_value']) {
          if ($value == 0) {
            // remove the row when the count is changed to 0
            db_delete('decathlon_honors')
              ->condition('contest', $contest)
              ->condition('pid', $pid)
              ->condition('honor', $honor)
              ->execute();
          } else {
            // use db_merge in case this is a new entry
            db_merge('decathlon_honors')
              ->key(array(
                  'contest' => $contest,
                  'pid' => $pid,
                  'honor' => $honor,
                ))
              ->fields(array(
                  'count' => $value,
                ))
              ->execute();
          }
          drupal_set_message("Changed honor count to " . $value);
          $changed = TRUE;
        }
        if ($pid != $data['player']) { // player has changed
          if ($value != 0) { // the row has been deleted
            db_update('decathlon_honors')
              ->fields(array(
                  'pid' => $data['player']
                ))
              ->condition('contest', $contest, '=')
              ->condition('pid', $pid, '=')
              ->condition('honor', $honor, '=')
              ->execute();
            $changed = TRUE;
          }
        }
      }
    }
  }

  if ($changed == TRUE) {

    // reset points, bestings, skunks
    module_load_include('inc', 'decathlon', 'modules/decathlon_scoring/decathlon_scoring');
    decathlon_scoring_reset_scores(decathlon_get_event());
  }

  $redirect = '/scores/game/' . $contest;
  $form_state['redirect'] = $redirect;
}

/**
 * Delete a contest
 */
function decathlon_edit_delete_form($form, &$form_state, $game) {

  $form['alert'] = array(
    '#type' => 'item',
    '#markup' => '<div id="progress-status">Deleting contest ID ' . $game . '</div>',
  );

  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Delete',
    '#ajax' => array(
      'callback' => 'decathlon_edit_delete_progress_callback',
      'progress' => array(
        'type' => 'throbber',
      ),
    ),
  );

  $form['actions']['cancel'] = array(
    '#type' => 'submit',
    '#value' => 'Cancel',
    '#limit_validation_errors' => array(),       // No validation
    '#submit' => array('decathlon_edit_delete_form_cancel'),
  );

  return $form;
}

/**
 * Cancel for delete
 */
function decathlon_edit_delete_form_cancel($form, &$form_state) {
  $contest = $form_state['build_info']['args'][0];
  $redirect = '/scores/game/' . $contest;
  $form_state['redirect'] = $redirect;
}

/**
 * Submit function for delete
 */
function decathlon_edit_delete_progress_callback($form, &$form_state) {

  // contest ID for deleting table rows
  $contest = $form_state['build_info']['args'][0];
  $commands = array();

  // delete honors table rows
    db_delete('decathlon_honors')
      ->condition('contest', $contest)
      ->execute();

  // delete scores table rows
    db_delete('decathlon_scores')
      ->condition('contest', $contest)
      ->execute();

  // delete contests table rows
    db_delete('decathlon_contests')
      ->condition('contest', $contest)
      ->execute();

  // reset scores
  module_load_include('inc', 'decathlon', 'modules/decathlon_scoring/decathlon_scoring');
  decathlon_scoring_reset_scores(decathlon_get_event());

  // update status
  $commands[] = ajax_command_html('#progress-status', t('Deleted.'));

  // redirect to front page
  ctools_include('ajax');
  ctools_add_js('ajax-responder');
  $commands[] = ctools_ajax_command_redirect('leaderboard');

  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}
