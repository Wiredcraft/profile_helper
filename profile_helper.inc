<?php
/**
 * @file.
 */

/**
 * Batch operation callbacks.
 *
 * You have to provide all the function arguments except $batch_context.
 *
 * E.g.
 * - $batch['operations'][] = array('profile_helper_flush_cache', array());
 * - $batch['operations'][] = array('profile_helper_create_menu_links', array('user-menu', array(
 *     'user/login' => 'Log in',
 *     'user/register' => 'Register',
 *   ), 0));
 *
 */

/**
 * Run tasks provided by profiler.
 */
function profile_helper_invoke_profiler($profile_key, $task, $url, &$batch_context) {
  profiler_profile_tasks(profiler_v2_load_config($profile_key), $task, $url);
}

/**
 * For Drupal core.
 */

/**
 * Flush caches.
 */
function profile_helper_flush_cache(&$batch_context) {
  module_rebuild_cache();
  drupal_get_schema(NULL, TRUE);
  drupal_flush_all_caches();
}

/**
 * For Features.
 */

/**
 * Revert all features.
 *
 * Difference with features_revert().
 * - call features_get_component_states() with $rebuild_only = FALSE and $reset = TRUE.
 */
function profile_helper_revert_features(&$batch_context) {
  if (!module_exists('features')) {
    return;
  }

  module_load_include('inc', 'features', 'features.export');
  features_include();

  $restore_states = array(FEATURES_OVERRIDDEN, FEATURES_REBUILDABLE, FEATURES_NEEDS_REVIEW);

  $items = array();
  $states = features_get_component_states(array(), FALSE, TRUE);
  foreach ($states as $module_name => $components) {
    foreach ($components as $component => $state) {
      if (in_array($state, $restore_states)) {
        $items[$module_name][] = $component;
      }
    }
  }

  return features_revert($items);
}

/**
 * Revert a features feature.
 */
function profile_helper_revert_feature($module_name, $components, &$batch_context) {
  if (!module_exists('features')) {
    return;
  }

  // TODO
}

/**
 * For Feeds.
 */

/**
 * Import a CSV file.
 */
function profile_helper_import_csv($importer_id, $filepath, &$batch_context) {
  if (!function_exists('feeds_source') || !is_file($filepath)) {
    return;
  }

  $source = feeds_source($importer_id);
  if (empty($source) || !($source->importer->fetcher instanceof FeedsFileFetcher)) {
    return;
  }

  $source->addConfig(
    array(
      'FeedsFileFetcher' => array(
        'source' => $filepath,
      ),
    )
  );
  while (FEEDS_BATCH_COMPLETE != $source->import());
}
