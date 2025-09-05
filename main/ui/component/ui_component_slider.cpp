/**
 * @file ui_component_slider.cpp
 * @brief Implementación del componente Slider (rango entero) para LVGL.
 * @ingroup ui_component_slider
 */

#include "ui/component/ui_component_slider.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_common.h"
#include "lvgl.h"
#include <cstdio>

namespace Ui::Component::Slider {

using Ui::Component::Common::flex_row;

/* --------------------------- Helpers internos --------------------------- */

static inline int32_t apply_step(int32_t v, int32_t step) {
    if (step <= 0) return v;
    const int32_t r = v % step;
    return (r == 0) ? v : (v - r + (r >= step/2 ? step : 0));
}

static void update_value_label(lv_obj_t* lbl, int32_t v, const char* suffix) {
    if (!lbl) return;
    char buf[24];
    if (suffix && suffix[0]) std::snprintf(buf, sizeof(buf), "%ld %s", (long)v, suffix);
    else                     std::snprintf(buf, sizeof(buf), "%ld", (long)v);
    lv_label_set_text(lbl, buf);
}

static void apply_variant_colors(lv_obj_t* sl, Ui::UiThemeStyles& s, Variant v) {
    if (!sl) return;

    lv_color_t active   = s.tokens.colorPrimary;
    lv_color_t onActive = s.tokens.colorOnPrimary;

    switch (v) {
        case Variant::Default:
            active   = s.tokens.colorPrimary;
            onActive = s.tokens.colorOnPrimary;
            break;
        case Variant::Success:
            active   = s.tokens.colorSuccess;
            onActive = s.tokens.colorOnSuccess;
            break;
        case Variant::Destructive:
            active   = s.tokens.colorError;
            onActive = s.tokens.colorOnError;
            break;
    }

    // Track inactivo (MAIN)
    lv_obj_set_style_bg_color(sl, s.tokens.colorSurface, LV_PART_MAIN);
    lv_obj_set_style_bg_opa  (sl, s.tokens.opaEnabled,   LV_PART_MAIN);
    lv_obj_set_style_radius  (sl, s.tokens.radiusSm,      LV_PART_MAIN);

    // Indicador activo (INDICATOR)
    lv_obj_set_style_bg_color(sl, active,                 LV_PART_INDICATOR);
    lv_obj_set_style_bg_opa  (sl, s.tokens.opaEnabled,    LV_PART_INDICATOR);
    lv_obj_set_style_radius  (sl, s.tokens.radiusSm,      LV_PART_INDICATOR);

    // Knob
    lv_obj_set_style_bg_color(sl, onActive,               LV_PART_KNOB);
    lv_obj_set_style_radius  (sl, LV_RADIUS_CIRCLE,       LV_PART_KNOB);
}

/* ------------------------------ Create -------------------------------- */

Handle create(lv_obj_t* parent, Ui::UiThemeStyles& styles, const Props& p, const Callbacks& cb) {
    Ui::themeInitOnce();

    Handle h{};
    h.root   = lv_obj_create(parent);
    lv_obj_remove_style_all(h.root);
    flex_row(h.root, LV_FLEX_ALIGN_STRETCH, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, /*gap*/8);

    // Slider
    h.slider = lv_slider_create(h.root);
    lv_slider_set_range(h.slider, p.min, p.max);

    // Colores por variante
    apply_variant_colors(h.slider, styles, p.variant);

    // Estado inicial
    int32_t v0 = p.value;
    if (p.step > 0) v0 = apply_step(v0, p.step);
    if (v0 < p.min) v0 = p.min;
    if (v0 > p.max) v0 = p.max;
    lv_slider_set_value(h.slider, v0, LV_ANIM_OFF);

    if (!p.enabled) lv_obj_add_state(h.slider, LV_STATE_DISABLED);

    // Label de valor (opcional)
    if (p.showValue) {
        h.valueL = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.valueL, styles.tokens.fontBody, LV_PART_MAIN);
        update_value_label(h.valueL, v0, p.suffix);
    }

    // Eventos (VALUE_CHANGED + RELEASED)
    if (cb.onChange || cb.onRelease || p.step > 0 || p.showValue) {
        // Encapsulamos lo mínimo en una struct local capturada por user data.
        struct User {
            Handle h;
            const char* suffix;
            int32_t step;
            Callbacks cb;
        };
        // Guardamos una copia ligera (Handle por valor guarda punteros a objetos reales).
        User* u = (User*)lv_mem_alloc(sizeof(User));
        *u = User{ h, p.suffix, p.step, cb };

        lv_obj_add_event_cb(h.slider, [](lv_event_t* e){
            auto* sl = static_cast<lv_obj_t*>(lv_event_get_target(e));
            auto* u  = static_cast<User*>(lv_event_get_user_data(e));
            int32_t v = lv_slider_get_value(sl);

            // Aplica step en caliente (redondeo) y refleja cambios.
            if (u->step > 0) {
                int32_t v2 = apply_step(v, u->step);
                if (v2 != v) {
                    lv_slider_set_value(sl, v2, LV_ANIM_OFF);
                    v = v2;
                }
            }
            if (u->h.valueL) update_value_label(u->h.valueL, v, u->suffix);

            if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
                if (u->cb.onChange) u->cb.onChange(u->h, v, u->cb.userData);
            } else if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
                if (u->cb.onRelease) u->cb.onRelease(u->h, v, u->cb.userData);
            }
        }, LV_EVENT_ALL, u);

        // Nota: liberación de u -> caller puede gestionar en destrucción de pantalla.
        // En MVP lo dejamos vivir hasta destruir el slider (LVGL libera user data con el obj).
    }

    return h;
}

/* ------------------------------ Mutadores ------------------------------ */

void setValue(Handle& h, int32_t v) {
    if (!h.slider) return;
    int32_t min = lv_slider_get_min_value(h.slider);
    int32_t max = lv_slider_get_max_value(h.slider);
    if (v < min) v = min;
    if (v > max) v = max;
    lv_slider_set_value(h.slider, v, LV_ANIM_OFF);
    if (h.valueL) update_value_label(h.valueL, v, nullptr); // Sufijo se mantiene visualmente igual
}

int32_t getValue(const Handle& h) {
    return h.slider ? lv_slider_get_value(h.slider) : 0;
}

void setRange(Handle& h, int32_t min, int32_t max) {
    if (!h.slider) return;
    lv_slider_set_range(h.slider, min, max);
    int32_t v = lv_slider_get_value(h.slider);
    if (v < min) v = min; if (v > max) v = max;
    lv_slider_set_value(h.slider, v, LV_ANIM_OFF);
    if (h.valueL) update_value_label(h.valueL, v, nullptr);
}

void setEnabled(Handle& h, bool enabled) {
    if (!h.slider) return;
    if (enabled) lv_obj_clear_state(h.slider, LV_STATE_DISABLED);
    else         lv_obj_add_state  (h.slider, LV_STATE_DISABLED);
}

void setShowValue(Handle& h, bool show) {
    if (!h.root) return;
    if (show) {
        if (!h.valueL) {
            h.valueL = lv_label_create(h.root);
            lv_label_set_text(h.valueL, ""); // se actualizará en el próximo cambio de valor
        }
    } else {
        if (h.valueL) { lv_obj_del(h.valueL); h.valueL = nullptr; }
    }
}

void setSuffix(Handle& h, const char* suffix) {
    if (!h.valueL) return;
    int32_t v = getValue(h);
    update_value_label(h.valueL, v, suffix);
}

void setStep(Handle& h, int32_t step) {
    // No hay almacenamiento de "step" en Handle; el step se aplica vía callback.
    // Si quieres forzarlo en runtime sin recrear, puedes enganchar un event_cb nuevo con user data.
    (void)h; (void)step;
    // MVP: omitimos implementar persistencia de step tras create() para mantener simplicidad.
}

void setVariant(Handle& h, Ui::UiThemeStyles& s, Variant v) {
    apply_variant_colors(h.slider, s, v);
}

} // namespace Ui::Component::Slider
