<?php
/**
 * JSON List API endpoint
 */
 function recipes_api_list_json() {
   $efq = new EntityFieldQuery();
   $retval = array();
   $params = drupal_get_query_parameters();
    _recipes_api_sanitize_recursive($params);
    //kpr($params); die();
   //@todo recursively santize parameters - Ethan did this below
   //@todo turn query parameters into database arguments
   // that just means you use these query parameters to
   // fill out entity field query methods
   $efq->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', RECIPES_CONTENT_TYPE)
    ->propertyCondition('status', NODE_PUBLISHED);
    if(!empty($params['id'])) {
      $efq->propertyCondition('nid', $params['id']);
    }
    if(!empty($params['title'])) {
      $efq->propertyCondition('title', '%' . $params['title'] . '%', 'LIKE');
    }
    if(!empty($params['season'])) {
      $efq->fieldCondition('field_season','value', $params['season'], 'LIKE');
    }
    if(!empty($params['dish_type'])) {
      $efq->fieldCondition('field_dish_type', 'tid', $params['dish_type']);
    }

   $results = $efq->execute();
   if(empty($results['node'])) {
     _recipes_api_json_respond($retval);
   }
   // Clean up node output. Don't use a bunch of unsets. Figure out how to only
   // select specific fields from the database. To get started, you should first
   // mindmap the entire api response structure out. This will help you
   // sort out dependencies. Explore the efq_extra_field module.
   $data = array();
   foreach(array_keys($results['node']) as $nid) {
     $data[] = _recipes_api_process_item($nid);
   }
   _recipes_api_json_respond($data);
 }

/**
 * Recursively santitize incoming user input.
 */
 function _recipes_api_sanitize_recursive(&$input) {
   foreach($input as &$value) {
     if(is_array($value)) {
       _recipes_api_sanitize_recursive($value);
     }
     else {
       $value = filter_xss(check_plain($value));
     }
   }
 }

 function _recipes_api_process_item($item_id) {
   $emw = entity_metadata_wrapper('node', $item_id);
   $item = array(
     'id' => $emw->nid->value(),
     'title' => $emw->title->value(),
     'image' => $emw->field_image->value(),
     'ingredients'=> $emw->field_ingredients->value(),
     'directions'=> array(
       'summary'=> $emw->body->value(),
       'recipe_steps'=> $emw->field_recipe_steps->value(),
     ),
     'dish_type' => $emw->field_dish_type->value(),
     'season'=> $emw->field_season->value(),
     'time' => array(
       'total_time' => $emw->field_total_time_to_prepare->value(),
       'cook' => $emw->field_cook_time->value(),  
       'prep' => $emw->field_prep_time->value(),
     ),
     'featured' => $emw->field_featured->value(),
     'quick_reference' => $emw->field_quick_reference->value()
   );
   return $item;
 }
