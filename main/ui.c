// main/ui.c
#include "lvgl.h"
#include "esp_log.h"

static const char *TAG = "ui";

/* --- Punteros a contenedores principales --- */
static lv_obj_t *header;
static lv_obj_t *content;
static lv_obj_t *footer;

/* =================== Callbacks =================== */

/* Botón "Menú" (cambia el texto del label pasado por user_data) */
static void on_menu_btn(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
        lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
        if (label) lv_label_set_text(label, "Menú pulsado");
        ESP_LOGI(TAG, "Botón Menú clicado");
    }
}

/* Slider → actualiza el label asociado con el valor */
static void on_setpoint_slider(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *s   = lv_event_get_target(e);
        lv_obj_t *lbl = (lv_obj_t *)lv_event_get_user_data(e);
        int32_t v     = lv_slider_get_value(s);
        static char buf[32];
        lv_snprintf(buf, sizeof(buf), "Setpoint: %d°C", (int)v);
        lv_label_set_text(lbl, buf);
    }
}

/* Switch → pone ON/OFF en el label asociado */
static void sw_event_cb(lv_event_t * e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
    lv_obj_t *sw    = lv_event_get_target(e);
    lv_obj_t *label = (lv_obj_t *)lv_event_get_user_data(e);
    lv_label_set_text(label, lv_obj_has_state(sw, LV_STATE_CHECKED) ? "ON" : "OFF");
}

/* =================== Construcción de la UI =================== */

void ui_create(void)
{
    /* 1) Pantalla raíz: flex column para Header / Content / Footer */
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x101418), 0);
    lv_obj_set_style_pad_all(scr, 0, 0);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr,
                          LV_FLEX_ALIGN_START,   /* main axis */
                          LV_FLEX_ALIGN_START,   /* cross axis */
                          LV_FLEX_ALIGN_START);  /* track cross axis */

    /* 2) Header */
    header = lv_obj_create(scr);
    lv_obj_set_size(header, LV_PCT(100), 60);
    lv_obj_set_style_bg_color(header, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_opa(header, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(header, 16, 0);
    lv_obj_set_style_pad_ver(header, 8, 0);
    lv_obj_set_style_border_width(header, 0, 0);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(header);
    lv_label_set_text(title, "HVAC HMI");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
#if LV_FONT_MONTSERRAT_24
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
#endif

    lv_obj_t *menu_btn = lv_btn_create(header);
    lv_obj_set_size(menu_btn, 120, LV_SIZE_CONTENT);
    lv_obj_t *menu_lbl = lv_label_create(menu_btn);
    lv_label_set_text(menu_lbl, "Menú");
    lv_obj_center(menu_lbl);
    lv_obj_add_event_cb(menu_btn, on_menu_btn, LV_EVENT_CLICKED, menu_lbl);

    /* 3) Content (crece y ocupa el resto) */
    content = lv_obj_create(scr);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x1C2128), 0);
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(content, 16, 0);
    lv_obj_set_style_border_width(content, 0, 0);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_row(content, 12, 0);  // separación vertical entre hijos

    /* 3.1) Etiqueta “Temperatura” */
    lv_obj_t *temp_lbl = lv_label_create(content);
    lv_label_set_text(temp_lbl, "Temperatura: 23.5°C");
    lv_obj_set_style_text_color(temp_lbl, lv_color_white(), 0);
#if LV_FONT_MONTSERRAT_20
    lv_obj_set_style_text_font(temp_lbl, &lv_font_montserrat_20, 0);
#endif

    /* 3.2) Slider de setpoint + etiqueta con el valor */
    lv_obj_t *slider = lv_slider_create(content);
    lv_obj_set_width(slider, LV_PCT(100));
    lv_slider_set_range(slider, 16, 28);
    lv_slider_set_value(slider, 22, LV_ANIM_OFF);

    lv_obj_t *sp_lbl = lv_label_create(content);
    lv_label_set_text(sp_lbl, "Setpoint: 22°C");
    lv_obj_set_style_text_color(sp_lbl, lv_color_white(), 0);
    lv_obj_add_event_cb(slider, on_setpoint_slider, LV_EVENT_VALUE_CHANGED, sp_lbl);

    /* Fuerza actualización inicial del label del setpoint */
    lv_event_send(slider, LV_EVENT_VALUE_CHANGED, NULL);

    /* 3.3) Switch + label ON/OFF con colores personalizados */
    {
        /* Contenedor vertical para switch + label */
        lv_obj_t *col = lv_obj_create(content);
        lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
        lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(col, 0, 0);
        lv_obj_set_style_pad_all(col, 0, 0);
        lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(col, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        lv_obj_t *sw = lv_switch_create(col);

        /* Indicador (track) ROJO cuando está OFF */
        lv_obj_set_style_bg_color(sw, lv_color_hex(0xD32F2F), LV_PART_INDICATOR);
        lv_obj_set_style_bg_opa  (sw, LV_OPA_COVER,          LV_PART_INDICATOR);

        /* Indicador (track) VERDE cuando está ON (estado CHECKED) */
        lv_obj_set_style_bg_color(sw, lv_color_hex(0x2E7D32), LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_set_style_bg_opa  (sw, LV_OPA_COVER,           LV_PART_INDICATOR | LV_STATE_CHECKED);

        /* Knob AZUL siempre (+ contorno blanco) */
        lv_obj_set_style_bg_color   (sw, lv_color_hex(0x1976D2), LV_PART_KNOB);
        lv_obj_set_style_bg_opa     (sw, LV_OPA_COVER,           LV_PART_KNOB);
        lv_obj_set_style_outline_width(sw, 2,                    LV_PART_KNOB);
        lv_obj_set_style_outline_color(sw, lv_color_white(),     LV_PART_KNOB);

        /* Label ON/OFF debajo */
        lv_obj_t *sw_lbl = lv_label_create(col);
        lv_label_set_text(sw_lbl, "OFF");
        lv_obj_set_style_text_color(sw_lbl, lv_color_white(), 0);

        /* Callback para actualizar el texto */
        lv_obj_add_event_cb(sw, sw_event_cb, LV_EVENT_VALUE_CHANGED, sw_lbl);

        /* Estado inicial opcional: enciende el switch y actualiza el label */
        // lv_obj_add_state(sw, LV_STATE_CHECKED);
        // lv_event_send(sw, LV_EVENT_VALUE_CHANGED, NULL);
    }

    /* 4) Footer: barra inferior con estado */
    footer = lv_obj_create(scr);
    lv_obj_set_size(footer, LV_PCT(100), 40);
    lv_obj_set_style_bg_color(footer, lv_color_hex(0x0B0D10), 0);
    lv_obj_set_style_bg_opa(footer, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_hor(footer, 12, 0);
    lv_obj_set_style_pad_ver(footer, 6, 0);
    lv_obj_set_style_border_width(footer, 0, 0);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *status = lv_label_create(footer);
    lv_label_set_text(status, LV_SYMBOL_WIFI " Conectado   " LV_SYMBOL_BATTERY_FULL " 100%");
    lv_obj_set_style_text_color(status, lv_color_white(), 0);

    ESP_LOGI(TAG, "UI creada");
}
