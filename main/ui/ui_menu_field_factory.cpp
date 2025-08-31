/**
 * @file ui_menu_field_factory.cpp
 * @brief Implementación de la factoría de widgets LVGL para campos (detail).
 * @ingroup ui_menu
 */
#include "ui_menu_field_factory.h"
#include "esp_log.h"
#include "lvgl.h"
#include <cmath>
#include <cstring>
#include <string>

extern "C" {
#include "cJSON.h"
}

static const char* TAG_FF = "UI_FIELD_FACTORY";

// Helpers locales
static inline double json_get_number(const cJSON* obj, const char* key, double def) {
    const cJSON* it = cJSON_GetObjectItem((cJSON*)obj, key);
    return (it && cJSON_IsNumber(it)) ? it->valuedouble : def;
}
static void set_value_label(lv_obj_t* lbl, double v, const char* unit) {
    char buf[64];
    const bool is_int = std::fabs(v - std::round(v)) < 0.0005;
    if (unit && *unit) {
        if (is_int) std::snprintf(buf, sizeof(buf), "%d %s", (int)std::llround(v), unit);
        else        std::snprintf(buf, sizeof(buf), "%.3f %s", v, unit);
    } else {
        if (is_int) std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(v));
        else        std::snprintf(buf, sizeof(buf), "%.3f", v);
    }
    lv_label_set_text(lbl, buf);
}

lv_obj_t* ui_menu_field_create_widget(lv_obj_t* parent, const cJSON* f)
{
    if (!parent || !f) return nullptr;

    const char* widget_type = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "widget_type"));
    const char* unit        = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "unit"));
    const char* mock_str    = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "mock"));
    const cJSON* editableJS = cJSON_GetObjectItem((cJSON*)f, "editable");
    const bool editable     = cJSON_IsBool(editableJS) ? cJSON_IsTrue(editableJS) : false;

    // ===================== SLIDER =====================
    if (widget_type && std::strcmp(widget_type, "slider") == 0) {
        double min   = json_get_number(f, "min",   0.0);
        double max   = json_get_number(f, "max", 100.0);
        double step  = json_get_number(f, "step",  1.0);
        double mock  = 0.0;
        if (mock_str) {
            char* endp=nullptr;
            mock = std::strtod(mock_str, &endp);
            if (endp==mock_str) mock = min;
        }

        // Decimales derivados de step
        int decimals = 0; double x = step;
        while (decimals < 6 && std::fabs(x - std::round(x)) > 1e-9) { x *= 10.0; decimals++; }
        const int scale = (decimals > 0) ? (int)std::llround(std::pow(10.0, decimals)) : 1;

        // Rango entero escalado
        const int32_t smin = (int32_t)std::llround(min  * scale);
        const int32_t smax = (int32_t)std::llround(max  * scale);
        const int32_t sval = (int32_t)std::llround(mock * scale);

        // Contenedor horizontal: [slider | label-valor]
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
        set_value_label(lbl_val, (double)sval / (double)scale, unit);

        // User-data del slider (para LVGL callbacks)
        struct SliderUD { lv_obj_t* label; int scale; double min, max, step; const char* unit; };
        auto *ud = (SliderUD*)std::malloc(sizeof(SliderUD));
        ud->label = lbl_val;
        ud->scale = scale;
        ud->min   = min;
        ud->max   = max;
        ud->step  = step;
        ud->unit  = unit;

        // VALUE_CHANGED: actualizar etiqueta con el valor ajustado al step
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
            if (u->label) set_value_label(u->label, snapped, u->unit);
        }, LV_EVENT_VALUE_CHANGED, ud);

        // DELETE: liberar el user-data
        lv_obj_add_event_cb(slider, +[](lv_event_t* e){
            SliderUD* u = (SliderUD*)lv_event_get_user_data(e);
            if (u) std::free(u);
        }, LV_EVENT_DELETE, ud);

        if (!editable) lv_obj_add_state(slider, LV_STATE_DISABLED);
        return wrap;
    }

    // ===================== DROPDOWN =====================
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

    // ===================== NUMBER (read-only) =====================
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

    // ===================== TEXT (read-only) =====================
    if (widget_type && std::strcmp(widget_type, "text") == 0) {
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, mock_str ? mock_str : "--");
        return lbl;
    }

    // ===================== BUTTON (placeholder) ==================
    if (widget_type && std::strcmp(widget_type, "button") == 0) {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_t* l   = lv_label_create(btn);
        lv_label_set_text(l, "OK");
        lv_obj_center(l);
        if (!editable) lv_obj_add_state(btn, LV_STATE_DISABLED);
        return btn;
    }

    // Desconocido
    ESP_LOGW(TAG_FF, "Widget no soportado: %s", widget_type ? widget_type : "(null)");
    lv_obj_t* fallback = lv_label_create(parent);
    lv_label_set_text(fallback, "--");
    return fallback;
}
