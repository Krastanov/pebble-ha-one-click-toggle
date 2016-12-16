#include <pebble.h>

static Window *s_window;
static TextLayer *s_txt_layer;
static Layer *s_img_layer;
static GBitmap *s_bitmap;
static int switch_state = -1;

static void on_img_layer_update(Layer* layer, GContext* ctx) {
    gbitmap_destroy(s_bitmap);
    if (switch_state == -1) {return;}
    if (switch_state) {
        s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_ON_96);        
    } else {
        s_bitmap = gbitmap_create_with_resource(RESOURCE_ID_OFF_96);        
    }
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, s_bitmap, GRect(0,0,96,96));
}

static void prv_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    s_txt_layer = text_layer_create(GRect(0, bounds.size.h-40, bounds.size.w, 30));
    text_layer_set_background_color(s_txt_layer, GColorClear);
    text_layer_set_text_color(s_txt_layer, GColorBlack);
    text_layer_set_font(s_txt_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    text_layer_set_text_alignment(s_txt_layer, GTextAlignmentCenter);
    text_layer_set_text(s_txt_layer, "Toggling...");
    layer_add_child(window_layer, text_layer_get_layer(s_txt_layer));

    s_img_layer = layer_create(GRect(bounds.size.w/2-96/2, bounds.size.h/2-96/2, 96, 96));
    layer_add_child(window_layer, s_img_layer);
    layer_set_update_proc(s_img_layer, on_img_layer_update);
}

static void prv_window_unload(Window *window) {
    gbitmap_destroy(s_bitmap);
    layer_destroy(s_img_layer);
    text_layer_destroy(s_txt_layer);
    window_destroy(s_window);
}

static void prv_toggle_state() {
    DictionaryIterator *out;
    AppMessageResult result = app_message_outbox_begin(&out);
    if (result != APP_MSG_OK) {
        text_layer_set_text(s_txt_layer, "Outbox Failed");
    }
    dict_write_uint8(out, MESSAGE_KEY_TOGGLE, 1);
    result = app_message_outbox_send();
    if (result != APP_MSG_OK) {
        text_layer_set_text(s_txt_layer, "Message Failed");
    }
}

static void prv_exit_application(void *data) {
  exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
  window_stack_remove(s_window, false);
}

static void prv_inbox_received_handler(DictionaryIterator *iter, void *context) {
    Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_APP_READY);
    if (ready_tuple) {
        if(launch_reason() == APP_LAUNCH_USER || launch_reason() == APP_LAUNCH_QUICK_LAUNCH) {
            prv_toggle_state();
        } else {
            text_layer_set_text(s_txt_layer, "App Installed!");
        }
        return;
    }

    Tuple *switch_state_tuple = dict_find(iter, MESSAGE_KEY_SWITCH_STATE);
    if (switch_state_tuple) {
        switch_state = switch_state_tuple->value->int32;
        if (switch_state) {
            text_layer_set_text(s_txt_layer, "ON");
        } else {
            text_layer_set_text(s_txt_layer, "OFF");
        }
        layer_mark_dirty(s_img_layer);
        int timeout = preferred_result_display_duration();
        AppTimer *timer = app_timer_register(timeout, prv_exit_application, NULL);
    }
}

static void prv_init(void) {
    app_message_register_inbox_received(prv_inbox_received_handler);
    app_message_open(256, 256);

    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = prv_window_load,
      .unload = prv_window_unload,
    });
    window_stack_push(s_window, false);
}

static void prv_update_app_glance(AppGlanceReloadSession *session, size_t limit, void *context) {
    if (limit < 1) return;

    AppGlanceSlice entry;
    if (switch_state) {
        entry = (AppGlanceSlice) {
          .layout = {
            //.icon = RESOURCE_ID_ON_25,
            .subtitle_template_string = "ON"
          },
          .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
        };
    } else {
        entry = (AppGlanceSlice) {
          .layout = {
            //.icon = RESOURCE_ID_OFF_25,
            .subtitle_template_string = "OFF"
          },
          .expiration_time = APP_GLANCE_SLICE_NO_EXPIRATION
        };
    }

    const AppGlanceResult result = app_glance_add_slice(session, entry);
    if (result != APP_GLANCE_RESULT_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "AppGlance Error: %d", result);
    }
}

static void prv_deinit(void) {
    app_message_deregister_callbacks();
    app_glance_reload(prv_update_app_glance, NULL);
}

int main(void) {
    prv_init();
    app_event_loop();
    prv_deinit();
}
