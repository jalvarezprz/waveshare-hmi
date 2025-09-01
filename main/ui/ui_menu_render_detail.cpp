/**
 * @file ui_menu_render_detail.cpp
 * @brief Implementación del render de vista detalle (view:"detail").
 * @ingroup ui_menu
 */

#include "ui_menu_render_detail.h"
#include "ui_menu_field_factory.h"
#include "esp_log.h"
#include "lvgl.h"
#include <cmath>
#include <string>
#include <cstring>

extern "C" {
#include "cJSON.h"
}

/// Etiqueta de log para este módulo.
static const char* TAG_DETAIL = "UI_MENU_DETAIL";

// --- Helpers de tamaño para sliders (detalle) ---
static void ui_slider_apply_sizes(lv_obj_t* slider, int max_width_px, int track_h_px, int knob_diam_px)
{
    // Ancho responsivo: 100% del padre con tope max
    lv_obj_set_width(slider, LV_PCT(100));
    lv_obj_set_style_max_width(slider, max_width_px, 0);  // LVGL v8+

    // Altura de la pista (track)
    lv_obj_set_height(slider, track_h_px);

    // Tamaño del knob: knob_size = track_h + 2*pad_knob
    int pad_knob = (knob_diam_px - track_h_px) / 2;
    if (pad_knob < 0) pad_knob = 0;
    lv_obj_set_style_pad_all(slider, pad_knob, LV_PART_KNOB);

    // Aumentar área táctil sin afectar lo visual
    lv_obj_set_ext_click_area(slider, 6);
}

static void ui_slider_apply_colors_basic(lv_obj_t* slider)
{
    // Pista
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A2C33), LV_PART_MAIN);
    // Indicador (relleno)
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x6FA8FF), LV_PART_INDICATOR);
    // Knob
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xEDEEF0), LV_PART_KNOB);
}

// --- Helper: encuentra el primer slider descendiente de un objeto ---
static lv_obj_t* find_first_slider(lv_obj_t* root)
{
    if (!root) return nullptr;

    extern const lv_obj_class_t lv_slider_class; // LVGL v8
    if (lv_obj_check_type(root, &lv_slider_class)) return root;

    uint32_t n = lv_obj_get_child_cnt(root);
    for (uint32_t i = 0; i < n; ++i) {
        lv_obj_t* child = lv_obj_get_child(root, i);
        lv_obj_t* s = find_first_slider(child);
        if (s) return s;
    }
    return nullptr;
}

// --- Helper: localiza y aplica estilo al slider real dentro del widget dado ---
static void style_slider_if_found(lv_obj_t* candidate, const char* field_id_or_null)
{
    lv_obj_t* s = find_first_slider(candidate);
    if (!s) {
        ESP_LOGW(TAG_DETAIL, "No se encontro lv_slider en widget de campo '%s'",
                 field_id_or_null ? field_id_or_null : "?");
        return;
    }

    // Tamaños y colores base
    ui_slider_apply_sizes(s, /*max_width_px*/ 720, /*track_h_px*/ 10, /*knob_diam_px*/ 18);
    ui_slider_apply_colors_basic(s);

    // Asegurar altura EXACTA del track (evitar que el tema la engorde)
    lv_obj_set_style_min_height(s, 10, 0);
    lv_obj_set_style_max_height(s, 10, 0);

    // Eliminar padding vertical que empuje la barra
    lv_obj_set_style_pad_top(s,    0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(s, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(s,    0, LV_PART_INDICATOR);
    lv_obj_set_style_pad_bottom(s, 0, LV_PART_INDICATOR);

    // Rellenar horizontal y permitir crecer
    lv_obj_set_width(s, LV_PCT(100));
    lv_obj_set_flex_grow(s, 1);

    // Centrado por si el tema lo coloca arriba
    lv_obj_align(s, LV_ALIGN_CENTER, 0, 0);

    ESP_LOGI(TAG_DETAIL, "Styled slider @%p (field '%s')", (void*)s,
             field_id_or_null ? field_id_or_null : "?");
}

/* ---------------------------------------------------------------------------
 * Render de la vista detalle
 * --------------------------------------------------------------------------- */
void ui_menu_render_detail_from_node(const cJSON* node, void (*on_back)(void))
{
    lv_obj_clean(lv_scr_act());

    // Tema base oscuro en la pantalla
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x121212), 0);

    const char* title_txt = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)node, "title"));
    const cJSON* fields   = cJSON_GetObjectItem((cJSON*)node, "fields");

    // Contenedor principal (tarjeta)
    lv_obj_t* cont = lv_obj_create(scr);
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1E1F25), 0);
    lv_obj_set_style_border_width(cont, 1, 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x2A2C33), 0);
    lv_obj_set_style_radius(cont, 12, 0);
    lv_obj_set_style_pad_all(cont, 10, 0);

    // Título
    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Detalle");
    lv_obj_set_style_text_color(title, lv_color_hex(0xEDEEF0), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Columna con los campos
    lv_obj_t* col = lv_obj_create(cont);
    lv_obj_set_size(col, 740, 300);
    lv_obj_align(col, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 8, 0);
    lv_obj_set_style_pad_all(col, 8, 0);
    lv_obj_set_scroll_dir(col, LV_DIR_VER);
    lv_obj_set_style_bg_color(col, lv_color_hex(0x1A1B21), 0);
    lv_obj_set_style_border_width(col, 1, 0);
    lv_obj_set_style_border_color(col, lv_color_hex(0x2A2C33), 0);
    lv_obj_set_style_radius(col, 10, 0);

    if (fields && cJSON_IsArray(fields)) {
        const cJSON* f = nullptr;
        cJSON_ArrayForEach(f, fields) {
            const char* label = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "label"));

            // Fila para cada field
            lv_obj_t* row = lv_obj_create(col);
            lv_obj_set_width(row, LV_PCT(100));
            lv_obj_set_style_pad_all(row, 6, 0);
            // Más aire vertical para que la barra no quede pegada
            lv_obj_set_style_pad_top(row, 8, 0);
            lv_obj_set_style_pad_bottom(row, 8, 0);
            lv_obj_set_style_radius(row, 8, 0);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_column(row, 8, 0);
            lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(row, lv_color_hex(0x1E1F25), 0);
            lv_obj_set_style_border_width(row, 1, 0);
            lv_obj_set_style_border_color(row, lv_color_hex(0x2A2C33), 0);

            // Centrar verticalmente los hijos en la fila (label + widget)
            lv_obj_set_flex_align(row,
                LV_FLEX_ALIGN_START,   // main axis (izq-der)
                LV_FLEX_ALIGN_CENTER,  // cross axis (arriba-abajo) -> centrado vertical
                LV_FLEX_ALIGN_CENTER
            );

            // Label izquierda
            lv_obj_t* l = lv_label_create(row);
            lv_label_set_text(l, label ? label : "-");
            lv_obj_set_style_text_color(l, lv_color_hex(0xEDEEF0), 0);
            lv_obj_set_width(l, 300);

            // Widget creado por la factoría
            lv_obj_t* w = ui_menu_field_create_widget(row, f);
            if (w) lv_obj_set_flex_grow(w, 1);

            // Detecta sliders por JSON y aplica estilo al lv_slider real
            const char* widget_type = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "widget_type"));
            const char* fid         = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "id"));

            if (w && widget_type && std::strcmp(widget_type, "slider") == 0) {
                // Con esta llamada basta: aplica ancho 100%, track 10 px, knob ~18 px, colores y centrado
                style_slider_if_found(w, fid);
            } else if (w) {
                // Fallback: por si el JSON trae otro valor pero la factoría creó un lv_slider
                lv_obj_t* maybe = find_first_slider(w);
                if (maybe) {
                    ESP_LOGW(TAG_DETAIL,
                             "Field '%s' con widget_type='%s' contenia un lv_slider; aplicando estilo por fallback",
                             fid ? fid : "?", widget_type ? widget_type : "?");
                    style_slider_if_found(w, fid);
                }
            }
        }
    } else {
        lv_obj_t* info = lv_label_create(col);
        lv_label_set_text(info, "(Sin campos definidos)");
        lv_obj_set_style_text_color(info, lv_color_hex(0xEDEEF0), 0);
    }

    // Botón volver
    lv_obj_t* back = lv_btn_create(cont);
    lv_obj_set_size(back, 120, 48);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
    lv_obj_t* bl = lv_label_create(back);
    lv_label_set_text(bl, "ATRAS");
    lv_obj_center(bl);

    // Delega la acción de volver en el callback proporcionado
    lv_obj_add_event_cb(back, [](lv_event_t* e){
        void (*cb)(void) = (void(*)(void)) lv_event_get_user_data(e);
        if (cb) cb();
    }, LV_EVENT_CLICKED, (void*)on_back);
}
