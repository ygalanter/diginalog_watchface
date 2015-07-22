#include "simple_analog.h"

#include "pebble.h"

#include "string.h"
#include "stdlib.h"


// layers to display: Day of the week, month, bluetooth status, battery percentage
Layer *info_layer;
TextLayer *dow_label;
TextLayer *month_label;
TextLayer *bluetooth_label;
TextLayer *battery_label;

// buffers to display: Day of the week, month,  battery percentage
char dow_buffer[4];
char month_buffer[4];
char battery_buffer[4];

// buffers to display Dat of the month, Hour, Minute
char num_buffer[4];
char hour_buffer[3];
char minute_buffer[3];

static GPath *minute_arrow;
static GPath *hour_arrow;
Layer *hands_layer;
Window *window;

 

static void hands_update_proc(Layer *layer, GContext *ctx) {
    
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  int32_t hour_angle = (TRIG_MAX_ANGLE * (((t->tm_hour % 12) * 6) + (t->tm_min / 10))) / (12 * 6);
  int32_t minute_angle = TRIG_MAX_ANGLE * t->tm_min / 60;
  
  // minute/hour hand
  gpath_rotate_to(hour_arrow,  hour_angle);
  gpath_rotate_to(minute_arrow, minute_angle);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_stroke_color(ctx, GColorBlack);

  gpath_draw_outline(ctx, hour_arrow);
  gpath_draw_outline(ctx, minute_arrow);
  gpath_draw_filled(ctx, minute_arrow);
  gpath_draw_filled(ctx, hour_arrow);
  
  //drawing circles with time
  if (!clock_is_24h_style()) {
    
        if( t->tm_hour > 11 ) {   // YG Jun-25-2014: 0..11 - am 12..23 - pm
           if( t->tm_hour > 12 ) t->tm_hour -= 12;
        } else {
            if( t->tm_hour == 0 ) t->tm_hour = 12;
        }        
    } 
  
  strftime(hour_buffer, sizeof(hour_buffer), "%H", t);
  strftime(minute_buffer, sizeof(minute_buffer), "%M", t);
  strftime(num_buffer, sizeof(num_buffer), "%d", t);
  
  graphics_context_set_text_color(ctx, GColorWhite);
  
  GRect bounds = layer_get_bounds(layer);
  GPoint center = grect_center_point(&bounds);
  
  int currY = -57 * cos_lookup(minute_angle) / TRIG_MAX_RATIO + center.y - 7;
  int currX = 57 * sin_lookup(minute_angle) / TRIG_MAX_RATIO + center.x;
  
  graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 13);
  graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, GPoint(currX, currY), 11);
  
  graphics_draw_text(ctx, minute_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(currX-10, currY-12, 20, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
  currY = -30 * cos_lookup(hour_angle) / TRIG_MAX_RATIO + center.y -7;
  currX = 30 * sin_lookup(hour_angle) / TRIG_MAX_RATIO + center.x;
  
  
  graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(currX, currY), 13);
  graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, GPoint(currX, currY), 11);
  
  graphics_draw_text(ctx, hour_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(currX-10, currY-12, 20, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

  graphics_context_set_fill_color(ctx, GColorWhite); graphics_fill_circle(ctx, GPoint(center.x, center.y - 7), 14);
  graphics_context_set_fill_color(ctx, GColorBlack); graphics_fill_circle(ctx, GPoint(center.x, center.y - 7), 11);
  
  graphics_draw_text(ctx, num_buffer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD), GRect(center.x - 10, center.y - 7 - 12, 20, 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
  
}

static void info_update_proc(Layer *layer, GContext *ctx) {

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  // displaying DOW
  strftime(dow_buffer, sizeof(dow_buffer), "%a", t);
  text_layer_set_text(dow_label, dow_buffer);
  
  // displaying month
  strftime(month_buffer, sizeof(month_buffer), "%b", t);
  text_layer_set_text(month_label, month_buffer);
  
  //displaying battery
  snprintf(battery_buffer, sizeof(battery_buffer), "%d", battery_state_service_peek().charge_percent);
  text_layer_set_text(battery_label, battery_buffer);

}

static void timer_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(window));
}

// show bt connected/disconnected
void display_bt_layer(bool connected) {
  vibes_short_pulse();
  if (connected) {
    text_layer_set_text(bluetooth_label, "ß");
  } else {
    text_layer_set_text(bluetooth_label, "†");
  }
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // init date layer -> a plain parent layer to show text info
  info_layer = layer_create(bounds);
  layer_set_update_proc(info_layer, info_update_proc);
  layer_add_child(window_layer, info_layer);

  // init day of the week
  dow_label = text_layer_create(GRect(3, 143, 25, 28));
  text_layer_set_text(dow_label, dow_buffer);
  text_layer_set_font(dow_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(dow_label, GColorWhite);
  
  text_layer_set_text_alignment(dow_label, GTextAlignmentLeft);
  layer_add_child(info_layer, text_layer_get_layer(dow_label));
  
  // init month
  month_label = text_layer_create(GRect(113, 143, 25, 28));
  text_layer_set_text(month_label, month_buffer);
  text_layer_set_font(month_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(month_label, GColorWhite);
  
  text_layer_set_text_alignment(month_label, GTextAlignmentRight);
  layer_add_child(info_layer, text_layer_get_layer(month_label));
  
  // init battery
  battery_label = text_layer_create(GRect(113, 0, 25, 28));
  text_layer_set_text(battery_label, battery_buffer);
  text_layer_set_font(battery_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(battery_label, GColorWhite);
  
  text_layer_set_text_alignment(battery_label, GTextAlignmentRight);
  layer_add_child(info_layer, text_layer_get_layer(battery_label));
  
  // init bluetooth
  bluetooth_label = text_layer_create(GRect(3, 0, 25, 28));
  text_layer_set_font(bluetooth_label, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_color(bluetooth_label, GColorWhite);
  
  text_layer_set_text_alignment(bluetooth_label, GTextAlignmentLeft);
  layer_add_child(info_layer, text_layer_get_layer(bluetooth_label));
  
 text_layer_set_background_color(dow_label, GColorBlack);
 text_layer_set_background_color(month_label, GColorBlack);
 text_layer_set_background_color(battery_label, GColorBlack);
 text_layer_set_background_color(bluetooth_label, GColorBlack);
 
  // initial bluetooth check
  if (bluetooth_connection_service_peek()) {
    text_layer_set_text(bluetooth_label, "ß");
  } else {
    text_layer_set_text(bluetooth_label, "†");
  }
  

  // init hands
  hands_layer = layer_create(bounds);
  layer_set_update_proc(hands_layer, hands_update_proc);
  layer_add_child(window_layer, hands_layer);
}

static void window_unload(Window *window) {
  text_layer_destroy(dow_label);
  text_layer_destroy(month_label);
  text_layer_destroy(battery_label);
  text_layer_destroy(bluetooth_label);
  layer_destroy(info_layer);
  layer_destroy(hands_layer);
}

static void init(void) {
  setlocale(LC_ALL, "");
  window = window_create();

  window_set_background_color(window, GColorBlack);  
  
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });

  dow_buffer[0] = '\0';
  num_buffer[0] = '\0';
  
  hour_buffer[0]  = '\0';
  minute_buffer[0]  = '\0';

  // init hand paths
  minute_arrow = gpath_create(&MINUTE_HAND_POINTS);
  hour_arrow = gpath_create(&HOUR_HAND_POINTS);

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  const GPoint center = grect_center_point(&bounds);
  gpath_move_to(minute_arrow, center);
  gpath_move_to(hour_arrow, center);


  // Push the window onto the stack
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(MINUTE_UNIT, timer_tick);
  bluetooth_connection_service_subscribe(display_bt_layer);
}

static void deinit(void) {
  gpath_destroy(minute_arrow);
  gpath_destroy(hour_arrow);

  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
