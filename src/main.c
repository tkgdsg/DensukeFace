#include <pebble.h>

static Window *s_main_window;
Layer *window_layer;
static TextLayer *s_time_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static BitmapLayer *s_bluetooth_layer;
static GBitmap *s_bluetooth_bitmap;

static TextLayer *s_battery_layer;
static char s_battery_buffer[16];

static void layer_update_callback(Layer *me, GContext* ctx) {
    GRect bounds = gbitmap_get_bounds (s_bluetooth_bitmap);
    graphics_draw_bitmap_in_rect(ctx, s_bluetooth_bitmap, 
        (GRect) { 
          .origin = { PBL_IF_ROUND_ELSE(158,126), PBL_IF_ROUND_ELSE(90,148) }, .size = bounds.size });
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[9];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M:%S" : "%I:%M:%S", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  // bluetooth connection status
  if(bluetooth_connection_service_peek()) {
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bluetooth_layer));
  } else {
    layer_remove_from_parent(bitmap_layer_get_layer(s_bluetooth_layer));
  }
  
  // battery status
  BatteryChargeState charge_state = battery_state_service_peek();
  snprintf(s_battery_buffer, sizeof(s_battery_buffer), "%d%%", charge_state.charge_percent);

  text_layer_set_text(s_battery_layer, s_battery_buffer);
  
}
 
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

    // Create GBitmap
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_EXAMPLE_IDENTIFIER);

// Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);

// Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

    // Create GBitmap
  s_bluetooth_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BLUETOOTH_IDENTIFIER);

// Create BitmapLayer to display the GBitmap
  s_bluetooth_layer = bitmap_layer_create(bounds);
//  s_bluetooth_layer = bitmap_layer_create(gbitmap_get_bounds(s_bluetooth_bitmap));
  
// Set the bitmap onto the layer and add to the window
  bitmap_layer_set_bitmap(s_bluetooth_layer, s_bluetooth_bitmap);

  layer_set_update_proc(bitmap_layer_get_layer(s_bluetooth_layer), layer_update_callback);
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(5, 0), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  
  // Create the TextLayer with specific bounds
  s_battery_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(5, 1), PBL_IF_ROUND_ELSE(90, 148), 32, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "100%");
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}