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

/* ---------------------------------------------------------------------------
 * Render de la vista detalle
 * --------------------------------------------------------------------------- */
void ui_menu_render_detail_from_node(const cJSON* node, void (*on_back)(void))
{
    lv_obj_clean(lv_scr_act());

    const char* title_txt = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)node, "title"));
    const cJSON* fields   = cJSON_GetObjectItem((cJSON*)node, "fields");

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Detalle");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* col = lv_obj_create(cont);
    lv_obj_set_size(col, 740, 300);
    lv_obj_align(col, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 8, 0);
    lv_obj_set_style_pad_all(col, 8, 0);
    lv_obj_set_scroll_dir(col, LV_DIR_VER);

    if (fields && cJSON_IsArray(fields)) {
        const cJSON* f = nullptr;
        cJSON_ArrayForEach(f, fields) {
            const char* label = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "label"));

            lv_obj_t* row = lv_obj_create(col);
            lv_obj_set_width(row, LV_PCT(100));
            lv_obj_set_style_pad_all(row, 6, 0);
            lv_obj_set_style_radius(row, 8, 0);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_column(row, 8, 0);
            lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* l = lv_label_create(row);
            lv_label_set_text(l, label ? label : "-");
            lv_obj_set_width(l, 300);

            lv_obj_t* w = ui_menu_field_create_widget(row, f);
            if (w) lv_obj_set_flex_grow(w, 1);
        }
    } else {
        lv_obj_t* info = lv_label_create(col);
        lv_label_set_text(info, "(Sin campos definidos)");
    }

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
