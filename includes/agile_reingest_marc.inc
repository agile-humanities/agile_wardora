<?php

/**
 * @file
 * Upload MARCXML file.
 */

/**
 * Upload MARCXML form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   A Drupal form definition.
 */
function agile_marcxml_file_form(array $form, array &$form_state, $object) {
  $form_state['object_id'] = $object->id;
  return array(
    'file' => array(
      '#type' => 'managed_file',
      '#title' => t('MARCXML File'),
      '#description' => t('A file containing a MARCXML record, to be transformed to MODS. Click "Next" to skip this step and create a record from scratch.'),
      '#upload_validators' => array(
        'file_validate_extensions' => array('xml'),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update'),
    ),
  );
}

/**
 * Populate the MODS datastream with the transformed MARCXML.
 *
 * Only populates the MODS datastream if a file was uploaded.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function agile_marcxml_file_form_submit(array &$form, array &$form_state) {
  module_load_include('inc', 'islandora_marcxml', 'includes/utilities');
  $module_path = drupal_get_path('module', 'xml_form_builder');
  $object = islandora_object_load($form_state['object_id']);
  $file = file_load($form_state['values']['file']);
  if ($file) {
    $success = islandora_marcxml_populate_mods($object, $file);
    $message = $success ? t("MODS updated") : t("Update was not successful");
    drupal_set_message($message);

    // To ensure the file is only used once, delete it.
    file_delete($file);
    unset($form_state['values']['file']);

    $xsl = new DOMDocument();
    $xsl->load("$module_path/transforms/mods_to_dc.xsl");
    $input = new DOMDocument();
    $input->loadXML($object['MODS']->content);
    $xpath = new DOMXPath($input);
    $xpath->registerNamespace("mods", "http://www.loc.gov/mods/v3");
    $labels = $xpath->query('//mods:titleInfo/mods:title');
    if ($object->label != $labels->item(0)->nodeValue){
      $object->label = $labels->item(0)->nodeValue;
    }
    $processor = new XSLTProcessor();
    $processor->importStylesheet($xsl);
    $dc = $processor->transformToXML($input);
    $object['DC']->content = $dc;
  }
}
