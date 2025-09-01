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

// ---------- Helpers locales ----------
static inline double json_get_number(const cJSON* obj, const char* key, double def) {
    const cJSON* it = cJSON_GetObjectItem((cJSON*)obj, key);
    return (it && cJSON_IsNumber(it)) ? it->valuedouble : def;
}

// Algunas fuentes por defecto no incluyen el símbolo '°'.
// Si detectamos UTF-8 "\xC2\xB0", lo reemplazamos por "C" como fallback.
static std::string sanitize_unit(const char* unit) {
    if (!unit) return {};
    std::string u(unit);
    const char* deg_utf8 = "\xC2\xB0";
    size_t pos = u.find(deg_utf8);
    if (pos != std::string::npos) {
        // Ej.: "°C" -> "C"
        // Si quieres "degC", cambia por "degC".
        u.erase(pos, 2); // quita el '°'
    }
    return u;
}

static void set_value_label(lv_obj_t* lbl, double v, const char* unit_raw) {
    char buf[64];
    const bool is_int = std::fabs(v - std::round(v)) < 0.0005;

    // saneamos unidad (evita "???")
    std::string unit = sanitize_unit(unit_raw);
    const bool have_unit = !unit.empty();

    if (have_unit) {
        if (is_int) std::snprintf(buf, sizeof(buf), "%d %s", (int)std::llround(v), unit.c_str());
        else        std::snprintf(buf, sizeof(buf), "%.3f %s", v, unit.c_str());
    } else {
        if (is_int) std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(v));
        else        std::snprintf(buf, sizeof(buf), "%.3f", v);
    }
    lv_label_set_text(lbl, buf);
}

// ---------- Estilo estándar para sliders ----------
static void style_slider_basic(lv_obj_t* slider) {
    if (!slider) return;

    // Altura exacta del track
    lv_obj_set_height(slider, 10);                // fuerza altura visual (mejor que min/max)
    lv_obj_set_style_min_height(slider, 10, 0);
    lv_obj_set_style_max_height(slider, 10, 0);

    // Knob ~18 px: knob_diam = track_h + 2*pad_knob -> 10 + 2*4 = 18
    lv_obj_set_style_pad_all(slider, 4, LV_PART_KNOB);

    // Colores
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x2A2C33), LV_PART_MAIN);       // pista
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x6FA8FF), LV_PART_INDICATOR);  // relleno
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xEDEEF0), LV_PART_KNOB);       // knob

    // Quitar paddings verticales que “engorden” el track
    lv_obj_set_style_pad_top(slider,    0, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(slider, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(slider,    0, LV_PART_INDICATOR);
    lv_obj_set_style_pad_bottom(slider, 0, LV_PART_INDICATOR);

    // Relleno horizontal y área táctil
    lv_obj_set_width(slider, LV_PCT(100));
    lv_obj_set_flex_grow(slider, 1);
    lv_obj_set_ext_click_area(slider, 6);
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
        lv_obj_set_style_pad_column(wrap, 8, 0);
        lv_obj_clear_flag(wrap, LV_OBJ_FLAG_SCROLLABLE);

        // Centrar verticalmente los hijos (para que el track no quede pegado arriba)
        lv_obj_set_flex_align(wrap,
            LV_FLEX_ALIGN_START,
            LV_FLEX_ALIGN_CENTER,
            LV_FLEX_ALIGN_CENTER
        );

        // Slider
        lv_obj_t* slider = lv_slider_create(wrap);
        lv_obj_set_flex_grow(slider, 1);

        // **ANTES ponías 22 px aquí**: lo quitamos y aplicamos nuestro estilo
        // lv_obj_set_height(slider, 22);  // <- eliminado
        style_slider_basic(slider);

        lv_slider_set_range(slider, smin, smax);
        lv_slider_set_value(slider, sval, LV_ANIM_OFF);

        // Etiqueta de valor (a la derecha)
        lv_obj_t* lbl_val = lv_label_create(wrap);
        set_value_label(lbl_val, (double)sval / (double)scale, unit);
        // Estética/legibilidad
        lv_obj_set_style_text_color(lbl_val, lv_color_hex(0xB0B3B8), 0);
        lv_label_set_long_mode(lbl_val, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(lbl_val, LV_SIZE_CONTENT);
        // Alinear verticalmente con el slider
        lv_obj_align(lbl_val, LV_ALIGN_CENTER, 0, 0);

        // User-data del slider (para LVGL callbacks)
        struct SliderUD { lv_obj_t* label; int scale; double min, max, step; std::string unit; };
        auto *ud = (SliderUD*)std::malloc(sizeof(SliderUD));
        ud->label = lbl_val;
        ud->scale = scale;
        ud->min   = min;
        ud->max   = max;
        ud->step  = step;
        ud->unit  = sanitize_unit(unit); // cacheamos unidad saneada

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
            if (u->label) set_value_label(u->label, snapped, u->unit.c_str());
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
            std::string txt = sanitize_unit(unit);
            if (!txt.empty()) {
                // Mostrar "valor unidad"
                std::string v = mock_str; v.push_back(' '); v += txt;
                lv_label_set_text(lbl, v.c_str());
            } else {
                lv_label_set_text(lbl, mock_str);
            }
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
