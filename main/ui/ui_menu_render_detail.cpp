/**
 * @file ui_menu_render_detail.cpp
 * @brief Implementación del render de vista detalle (view:"detail").
 * @ingroup ui_menu
 */

#include "ui_menu_render_detail.h"
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
 * Factoría interna de widgets para un campo de "view:detail".
 * (Se moverá a un módulo propio en un paso posterior: ui_menu_field_factory.*)
 * Soporta: slider, dropdown, number, text, button.
 * --------------------------------------------------------------------------- */
 
static lv_obj_t* create_field_widget(lv_obj_t* parent, const cJSON* f)
{
   
   struct SliderUD { lv_obj_t* label; int scale; double min, max, step; };
 
   if (!f) return nullptr;

    const char* widget_type = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "widget_type"));
    const char* unit        = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "unit"));
    const char* mock_str    = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "mock"));
    const cJSON* editableJS = cJSON_GetObjectItem((cJSON*)f, "editable");
    bool editable = cJSON_IsBool(editableJS) ? cJSON_IsTrue(editableJS) : false;

    auto json_get_number = [&](const char* key, double def) -> double {
        const cJSON* it = cJSON_GetObjectItem((cJSON*)f, key);
        return (it && cJSON_IsNumber(it)) ? it->valuedouble : def;
    };

    auto set_value_label = [&](lv_obj_t* lbl, double v){
        char buf[48];
        if (unit && *unit) {
            if (std::fabs(v - std::round(v)) < 0.0005) std::snprintf(buf, sizeof(buf), "%d %s", (int)std::llround(v), unit);
            else                                       std::snprintf(buf, sizeof(buf), "%.3f %s", v, unit);
        } else {
            if (std::fabs(v - std::round(v)) < 0.0005) std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(v));
            else                                       std::snprintf(buf, sizeof(buf), "%.3f", v);
        }
        lv_label_set_text(lbl, buf);
    };

    // -------- SLIDER --------
   // -------- SLIDER --------
   if (widget_type && std::strcmp(widget_type, "slider") == 0) {
      // Estructura para el user-data del slider (visible por las lambdas)
      struct SliderUD { lv_obj_t* label; int scale; double min, max, step; };

      // Leemos min/max/step/mock
      auto json_get_number = [&](const char* key, double def) -> double {
         const cJSON* it = cJSON_GetObjectItem((cJSON*)f, key);
         return (it && cJSON_IsNumber(it)) ? it->valuedouble : def;
      };
      double min   = json_get_number("min",   0.0);
      double max   = json_get_number("max", 100.0);
      double step  = json_get_number("step",  1.0);
      double mock  = 0.0;
      if (mock_str) {
         char* endp=nullptr;
         mock = std::strtod(mock_str, &endp);
         if (endp==mock_str) mock = min; // si no parsea, usa min
      }

      // Decimales derivados de step (p.ej. 0.1 -> 1 decimal, 0.01 -> 2)
      int decimals = 0; double x = step;
      while (decimals < 6 && std::fabs(x - std::round(x)) > 1e-9) { x *= 10.0; decimals++; }
      int scale = (decimals > 0) ? (int)std::llround(std::pow(10.0, decimals)) : 1;

      // Rango entero escalado
      int32_t smin = (int32_t)std::llround(min  * scale);
      int32_t smax = (int32_t)std::llround(max  * scale);
      int32_t sval = (int32_t)std::llround(mock * scale);

      // Contenedor del control + etiqueta de valor
      lv_obj_t* wrap = lv_obj_create(parent);
      lv_obj_set_width(wrap, LV_PCT(100));
      lv_obj_set_flex_flow(wrap, LV_FLEX_FLOW_ROW);
      lv_obj_set_style_pad_all(wrap, 0, 0);
      lv_obj_clear_flag(wrap, LV_OBJ_FLAG_SCROLLABLE);

      lv_obj_t* slider = lv_slider_create(wrap);
      lv_obj_set_flex_grow(slider, 1);
      lv_obj_set_height(slider, 22);
      lv_slider_set_range(slider, smin, smax);
      lv_slider_set_value(slider, sval, LV_ANIM_OFF);

      lv_obj_t* lbl_val = lv_label_create(wrap);
      set_value_label(lbl_val, (double)sval / (double)scale);

      // Reserva y rellena el user-data
      auto *ud = (SliderUD*)std::malloc(sizeof(SliderUD));
      ud->label = lbl_val;
      ud->scale = scale;
      ud->min   = min;
      ud->max   = max;
      ud->step  = step;

      // VALUE_CHANGED: actualizar label con el valor “snappeado”
      lv_obj_add_event_cb(slider, +[](lv_event_t* e){
         if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;
         SliderUD* u = (SliderUD*)lv_event_get_user_data(e);
         if (!u) return;

         int sval = lv_slider_get_value(lv_event_get_target(e));
         double real = (double)sval / (double)u->scale;
         const double step = (u->step > 0 ? u->step : 1.0);

         double t = (real - u->min) / step;
         if (t < 0) t = 0;
         const double tmax = (u->max - u->min) / step;
         if (t > tmax) t = tmax;

         const double snapped = u->min + std::round(t) * step;

         if (u->label) {
               char buf[48];
               if (std::fabs(snapped - std::round(snapped)) < 0.0005)
                  std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(snapped));
               else
                  std::snprintf(buf, sizeof(buf), "%.3f", snapped);
               lv_label_set_text(u->label, buf);
         }
      }, LV_EVENT_VALUE_CHANGED, ud);

      // DELETE: liberar el user-data
      lv_obj_add_event_cb(slider, +[](lv_event_t* e){
         SliderUD* u = (SliderUD*)lv_event_get_user_data(e);
         if (u) std::free(u);
      }, LV_EVENT_DELETE, ud);

      if (!editable) lv_obj_add_state(slider, LV_STATE_DISABLED);
      return wrap;
   }

    // -------- DROPDOWN --------
    if (widget_type && std::strcmp(widget_type, "dropdown") == 0) {
        lv_obj_t* dd = lv_dropdown_create(parent);

        const cJSON* opts = cJSON_GetObjectItem((cJSON*)f, "options");
        if (opts && cJSON_IsArray(opts)) {
            std::string opt_text;
            const cJSON* o = nullptr; bool first=true;
            cJSON_ArrayForEach(o, opts) {
                const char* s = cJSON_GetStringValue(o);
                if (!s) continue;
                if (!first) opt_text.push_back('\n');
                opt_text += s; first=false;
            }
            if (!opt_text.empty()) lv_dropdown_set_options(dd, opt_text.c_str());
        }

        if (mock_str && *mock_str) lv_dropdown_set_text(dd, mock_str);
        if (!editable) lv_obj_add_state(dd, LV_STATE_DISABLED);
        return dd;
    }

    // -------- NUMBER (RO) --------
    if (widget_type && std::strcmp(widget_type, "number") == 0) {
        lv_obj_t* lbl = lv_label_create(parent);
        if (mock_str) {
            std::string txt = mock_str;
            if (unit && *unit) { txt += " "; txt += unit; }
            lv_label_set_text(lbl, txt.c_str());
        } else {
            lv_label_set_text(lbl, "--");
        }
        return lbl;
    }

    // -------- TEXT (RO) --------
    if (widget_type && std::strcmp(widget_type, "text") == 0) {
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, mock_str ? mock_str : "--");
        return lbl;
    }

    // -------- BUTTON (placeholder) --------
    if (widget_type && std::strcmp(widget_type, "button") == 0) {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_t* l   = lv_label_create(btn);
        lv_label_set_text(l, "OK");
        lv_obj_center(l);
        if (!editable) lv_obj_add_state(btn, LV_STATE_DISABLED);
        return btn;
    }

    ESP_LOGW(TAG_DETAIL, "Widget no soportado en detail: %s", widget_type ? widget_type : "(null)");
    lv_obj_t* fallback = lv_label_create(parent);
    lv_label_set_text(fallback, "--");
    return fallback;
}

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

            lv_obj_t* w = create_field_widget(row, f);
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
