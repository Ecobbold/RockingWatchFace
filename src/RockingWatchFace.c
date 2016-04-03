#include <pebble.h>
#include "RockingWatchFace.h"

static Window *s_main_window;
static TextLayer *s_time_layer;
static GFont s_time_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GDrawCommandImage *s_command_image;
static Layer *s_canvas_layer;

////static BitmapLayer *s_text_ur_layer;
////static GBitmap *s_text_ur_bitmap;

//changing text (CT)
static AppSync s_sync;
static uint8_t s_sync_buffer[64];
static BitmapLayer *s_icon_layer;
static GBitmap *s_icon_bitmap = NULL;

enum TextKey {
  TEXT_KEY = 0x0,        // TUPLE_INT
};
static const uint32_t TEXT_ICONS[] = {
  RESOURCE_ID_TEXT_YOUROCK_R, // 0
  RESOURCE_ID_TEXT_ROCKIT //1
};
static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}
static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
  switch (key) {
    case TEXT_KEY:
      if (s_icon_bitmap) {
        gbitmap_destroy(s_icon_bitmap);
      }

      s_icon_bitmap = gbitmap_create_with_resource(TEXT_ICONS[new_tuple->value->uint8]);
      bitmap_layer_set_compositing_mode(s_icon_layer, GCompOpSet);
      bitmap_layer_set_bitmap(s_icon_layer, s_icon_bitmap);
      break;
  }
}


static void update_proc(Layer *layer, GContext *ctx) {
  // Set the origin offset from the context for drawing the image
  GPoint origin = GPoint(10, 20);
  

  // Draw the GDrawCommandImage to the GContext

  gdraw_command_image_draw(ctx, s_command_image, origin);

}

static void main_window_load(Window *window) {
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_HOME);
  
  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

  // Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));  
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(20, 10), bounds.size.w, 55));

  // Create GFont
  s_time_font = fonts_load_custom_font(
  resource_get_handle(RESOURCE_ID_RALEWAY_45));


  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  // Apply to TextLayer
text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  int rockx = 65;
  int rocky = 102;
  // Create the canvas Layer
  s_canvas_layer = layer_create(GRect(rockx,PBL_IF_ROUND_ELSE(rocky+5, rocky), bounds.size.w, bounds.size.h));

  // Set the LayerUpdateProc
  layer_set_update_proc(s_canvas_layer, update_proc);

  // Add to parent Window
  layer_add_child(window_layer, s_canvas_layer);  
  
  //You rock text
   // Create GBitmap
  //s_text_ur_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TEXT_YOUROCK_R);
 //// s_text_ur_bitmap = gbitmap_create_with_resource(RESOURCE_ID_TEXT_ROCKIT);
  
  
  // Create BitmapLayer to display the GBitmap
  /////s_text_ur_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(20, 15), PBL_IF_ROUND_ELSE(20, 10), bounds.size.w, bounds.size.h));

  // Set the bitmap onto the layer and add to the window
 // bitmap_layer_set_bitmap(s_text_ur_layer, s_text_ur_bitmap);
  //layer_add_child(window_layer, bitmap_layer_get_layer(s_text_ur_layer));
  
  //CT
   s_icon_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(20, 15), PBL_IF_ROUND_ELSE(65, 60), bounds.size.w, 80));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_icon_layer));
  
    Tuplet initial_values[] = {
      TupletInteger(TEXT_KEY, (uint8_t) 1),
    };
  app_sync_init(&s_sync, s_sync_buffer, sizeof(s_sync_buffer),
      initial_values, ARRAY_LENGTH(initial_values),
      sync_tuple_changed_callback, sync_error_callback, NULL);

 }
static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  // Unload GFont
  fonts_unload_custom_font(s_time_font);  
  // Destroy GBitmap
gbitmap_destroy(s_background_bitmap);

// Destroy BitmapLayer
bitmap_layer_destroy(s_background_layer);
  
   layer_destroy(s_canvas_layer);
  gdraw_command_image_destroy(s_command_image);
  
  //destroy you rock
  // Destroy GBitmap
 //// gbitmap_destroy(s_text_ur_bitmap);

  // Destroy BitmapLayer
 //// bitmap_layer_destroy(s_text_ur_layer);
  
  //CT
    if (s_icon_bitmap) {
    gbitmap_destroy(s_icon_bitmap);
  }
    bitmap_layer_destroy(s_icon_layer);

}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
update_time();
}

static void init() {
// Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorWhite);
  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  // Make sure the time is displayed from the start
update_time();
  
  // Register with TickTimerService
tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   // Create the object from resource file
  s_command_image = gdraw_command_image_create_with_resource(RESOURCE_ID_ROCKFRIEND);

}


static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
  
    app_sync_deinit(&s_sync);

}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

