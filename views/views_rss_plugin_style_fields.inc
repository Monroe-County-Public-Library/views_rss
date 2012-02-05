<?php

/**
 * @file
 * Extend the view_plugin_style class to provide an RSS view style.
 */

class views_rss_plugin_style_fields extends views_plugin_style {

  /**
   * Attach this view to another display as a feed.
   */
  function attach_to($display_id, $path, $title) {
    $display = $this->view->display[$display_id]->handler;
    $url_options = array('absolute' => TRUE);
    $input = $this->view->get_exposed_input();
    if ($input) {
      $url_options['query'] = $input;
    }

    $url = url($this->view->get_url(NULL, $path), $url_options);
    if ($display->has_path() && !$this->options['feed_settings']['feed_in_links']) {
      if (empty($this->preview)) {
        drupal_add_feed($url, $title);
      }
    }
    else {
      if (empty($this->view->feed_icon)) {
        $this->view->feed_icon = '';
      }

      $this->view->feed_icon .= theme('feed_icon', array('url' => $url, 'title' => $title));
      drupal_add_html_head_link(array(
        'rel' => 'alternate',
        'type' => 'application/rss+xml',
        'title' => $title,
        'href' => $url,
      ));
    }
  }

  function option_definition() {
    $options = parent::option_definition();

    // Namespace defaults.
    $namespaces = views_rss_get_namespaces();
    if (count($namespaces)) {
      foreach ($namespaces as $module => $module_namespaces) {
        foreach (array_keys($module_namespaces) as $namespace) {
          $options['namespaces'][$module][$namespace] = array('default' => NULL);
        }
      }
    }

    // Channel element defaults.
    $channel_elements = views_rss_get_channel_elements();
    if (count($channel_elements)) {
      foreach ($channel_elements as $module => $module_channel_elements) {
        foreach (array_keys($module_channel_elements) as $element) {
          list($namespace, $element_name) = views_rss_extract_element_names($element, 'core');
          $options['channel'][$namespace][$module][$element_name] = array('default' => NULL);
        }
      }
    }

    // Item element defaults.
    $item_elements = views_rss_get_item_elements();
    if (count($item_elements)) {
      foreach ($item_elements as $module => $module_item_elements) {
        foreach (array_keys($module_item_elements) as $element) {
          list($namespace, $element_name) = views_rss_extract_element_names($element, 'core');
          $options['item'][$namespace][$module][$element_name] = array('default' => NULL);
        }
      }
    }

    // Other feed settings defaults.
    $options['feed_settings']['feed_in_links'] = array('default' => 0);
    $options['feed_settings']['exposed_in_source'] = array('default' => 0);

    return $options;
  }

  /**
   * Provide a form for setting options.
   *
   * @param array $form
   * @param array $form_state
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $handlers = $this->display->handler->get_handlers('field');
    if (empty($handlers)) {
      drupal_set_message(t('You need at least one field before you can configure your field settings.'), 'warning');
    }
    else {

      // Field chooser.
      $field_names = array('' => '--');
      foreach ($handlers as $field => $handler) {
        if ($label = $handler->label()) {
          $field_names[$field] = $label;
        }
        else {
          $field_names[$field] = $handler->ui_name();
        }
      }

      // Channel elements settings.
      $channel_elements = views_rss_get_channel_elements();
      if (count($channel_elements)) {
        foreach ($channel_elements as $module => $module_channel_elements) {
          foreach ($module_channel_elements as $element => $definition) {
            if (!isset($definition['configurable']) || $definition['configurable']) {
              list($namespace, $element_name) = views_rss_extract_element_names($element, 'core');
              // Add fieldset for namespace if not yet added.
              if (!isset($form['channel'][$namespace])) {
                $form['channel'][$namespace] = array(
                  '#type' => 'fieldset',
                  '#title' => t('Channel elements : @namespace', array('@namespace' => $namespace)),
                  '#description' => t('Provide values for &lt;channel&gt; elements in "@namespace" namespace. See <a href="@guide_url">Views RSS documentation</a> for more information.', array(
                    '@namespace' => $namespace,
                    '@guide_url' => url('http://drupal.org/node/1344136'),
                  )),
                  '#collapsible' => TRUE,
                  '#collapsed' => TRUE,
                );
              }
              if (isset($this->options['channel'][$namespace][$module][$element_name])) {
                $default_value = $this->options['channel'][$namespace][$module][$element_name];
              }
              elseif (isset($definition['default_value'])) {
                $default_value = $definition['default_value'];
              }
              else {
                $default_value = NULL;
              }
              $form['channel'][$namespace][$module][$element_name] = array(
                '#type' => isset($definition['type']) ? $definition['type'] : 'textfield',
                '#title' => check_plain(isset($definition['title']) ? $definition['title'] : $element_name),
                '#description' => check_plain(isset($definition['description']) ? $definition['description'] : NULL),
                '#default_value' => $default_value,
                '#required' => isset($definition['required']) ? $definition['required'] : FALSE,
              );
              // Add help link if provided.
              if (isset($definition['help']) && $definition['help']) {
                $form['channel'][$namespace][$module][$element_name]['#description'] .= ' ' . l('[?]', $definition['help'], array('attributes' => array('title' => t('Need more information?'))));
              }
            }
          }
        }
      }

      // Item elements settings.
      $item_elements = views_rss_get_item_elements();
      if (count($item_elements)) {
        foreach ($item_elements as $module => $module_item_elements) {
          foreach ($module_item_elements as $element => $definition) {
            if (!isset($definition['configurable']) || $definition['configurable']) {
              list($namespace, $element_name) = views_rss_extract_element_names($element, 'core');
              // Add fieldset for namespace if not yet added.
              if (!isset($form['item'][$namespace])) {
                $form['item'][$namespace] = array(
                  '#type' => 'fieldset',
                  '#title' => t('Item elements : @namespace', array('@namespace' => $namespace)),
                  '#description' => t('Select fields containing relevant values for &lt;item&gt; elements in "@namespace" namespace. See <a href="@guide_url">Views RSS documentation</a> for more information.', array(
                    '@namespace' => $namespace,
                    '@guide_url' => url('http://drupal.org/node/1344136'),
                  )),
                  '#collapsible' => TRUE,
                  '#collapsed' => TRUE,
                );
              }
              if (isset($this->options['item'][$namespace][$module][$element_name])) {
                $default_value = $this->options['item'][$namespace][$module][$element_name];
              }
              else {
                $default_value = NULL;
              }
              $form['item'][$namespace][$module][$element_name] = array(
                '#type' => 'select',
                '#title' => check_plain(isset($definition['title']) ? $definition['title'] : $element_name),
                '#description' => check_plain(isset($definition['description']) ? $definition['description'] : NULL),
                '#options' => $field_names,
                '#default_value' => $default_value,
              );
              // Add help link if provided.
              if (isset($definition['help']) && $definition['help']) {
                $form['item'][$namespace][$module][$element_name]['#description'] .= ' ' . l('[?]', $definition['help'], array('attributes' => array('title' => t('Need more information?'))));
              }
            }
          }
        }
      }

      // Undefined namespaces derived from <channel> and/or <item> 
      // elements defined by extension modules.
      $namespaces = views_rss_get_namespaces();
      if (count($namespaces)) {
        foreach ($namespaces as $module => $module_namespaces) {
          foreach ($module_namespaces as $namespace => $definition) {
            if (empty($definition['uri'])) {
              // Add fieldset for namespace if not yet added.
              if (!isset($form['namespaces'])) {
                $form['namespaces'] = array(
                  '#type' => 'fieldset',
                  '#title' => t('Namespaces'),
                  '#description' => t('Enter missing URLs for namespaces derived from &lt;channel&gt; and/or &lt;item&gt; elements defined by extension modules.'),
                  '#collapsible' => TRUE,
                  '#collapsed' => TRUE,
                );
              }
              if (!empty($this->options['namespaces'][$module][$namespace])) {
                $default_value = $this->options['namespaces'][$module][$namespace];
              }
              else {
                $default_value = NULL;
              }
              $form['namespaces'][$module][$namespace] = array(
                '#type' => 'textfield',
                '#title' => check_plain($namespace),
                '#default_value' => $default_value,
              );
            }
          }
        }
      }

      // Other feed settings.
      $form['feed_settings'] = array(
        '#type' => 'fieldset',
        '#title' => t('Other feed settings'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $form['feed_settings']['feed_in_links'] = array(
        '#type' => 'checkbox',
        '#title' => t('Display feed icon in the links attached to the view'),
        '#default_value' => !empty($this->options['feed_settings']['feed_in_links']),
        '#weight' => 1,
      );
      $form['feed_settings']['exposed_in_source'] = array(
        '#type' => 'checkbox',
        '#title' => t("Add exposed filter values to <em>url</em> argument of feed &lt;source&gt; element"),
        '#description' => t('By default &lt;source&gt; element of each feed &lt;item&gt; will point to main feed URL (including arguments if provided). Enabling this option will add also all exposed filter values to the feed URL in &lt;source&gt; element.'),
        '#default_value' => !empty($this->options['feed_settings']['exposed_in_source']),
        '#weight' => 2,
      );

    }
  }

  /**
   * Map views row result to an RSS item.
   */
  function map_rows($rows) {
    // Fields must be pre-rendered starting from version 2.3 of Views module.
    $rendered = array();
    $keys = array_keys($this->view->field);
    foreach ($rows as $count => $row) {
      foreach ($keys as $id) {
        $rendered[$count][$id] = $this->view->field[$id]->theme($row);
      }
    }
    // Rewrite view rows to XML item rows.
    $items = array();
    $item_elements = views_rss_get_item_elements();
    foreach ($rendered as $id => $row) {
      $item = array();
      foreach ($item_elements as $module => $module_item_elements) {
        foreach (array_keys($module_item_elements) as $element) {
          list($namespace, $element_name) = views_rss_extract_element_names($element, 'core');
          // Assign values for all elements, not only those defined in view settings.
          // If element value is not defined in view settings, let's just assign NULL.
          // It will not be passed to final theme function anyway during processing
          // taking place in template_preprocess_views_view_views_rss().
          if (
            isset($this->options['item'][$namespace][$module][$element_name]) 
            && isset($row[$this->options['item'][$namespace][$module][$element_name]])
          ) {
            $item[$module][$element] = $row[$this->options['item'][$namespace][$module][$element_name]];
          }
          else {
            $item[$module][$element] = NULL;
          }
        }
      }
      $items[] = $item;
    }
    return $items;
  }

}