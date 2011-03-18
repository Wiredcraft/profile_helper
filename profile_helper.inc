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
 * For Drupal core.
 */

/**
 * Flush caches.
 */
function profile_helper_flush_cache(&$batch_context) {
  drupal_flush_all_caches();
  drupal_get_schema(NULL, TRUE);
}

/**
 * Create an administrator role.
 */
function profile_helper_create_administrator_role(&$batch_context) {
  // Create a default role for site administrators, with all available permissions assigned.
  $admin_role = new stdClass();
  $admin_role->name = 'administrator';
  $admin_role->weight = 2;
  user_role_save($admin_role);
  user_role_grant_permissions($admin_role->rid, array_keys(module_invoke_all('permission')));
  // Set this as the administrator role.
  variable_set('user_admin_role', $admin_role->rid);

  // Assign user 1 the "administrator" role.
  db_insert('users_roles')
    ->fields(array('uid' => 1, 'rid' => $admin_role->rid))
    ->execute();
}

/**
 * Create terms.
 */
function profile_helper_create_terms($vocab_machine_name, $terms, $weight, &$batch_context) {
  if (!module_exists('taxonomy')) {
    return;
  }

  $vocab = taxonomy_vocabulary_machine_name_load($vocab_machine_name);
  if (empty($vocab->vid)) {
    return;
  }

  foreach ($terms as $term_name) {
    $term = new stdClass();
    $term->vid = $vocab->vid;
    $term->name = $term_name;
    $term->weight = $weight++;
    taxonomy_term_save($term);
  }

  // TODO: return.
}

/**
 * Create menu links.
 */
function profile_helper_create_menu_links($menu_name, $links, $weight, &$batch_context) {
  // TODO: check?

  foreach ($links as $link_path => $link_title) {
    $item = array();
    $item['menu_name'] = $menu_name;
    $item['link_path'] = $link_path;
    $item['link_title'] = $link_title;
    $item['weight'] = $weight++;
    menu_link_save($item);
  }

  // TODO: return.
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
