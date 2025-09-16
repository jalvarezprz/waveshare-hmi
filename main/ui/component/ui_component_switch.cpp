/**
 * @file ui_component_switch.cpp
 * @brief Implementacion del componente Switch (toggle).
 * @ingroup ui_component_switch
 */

#include "ui/component/ui_component_switch.h"
#include "ui/component/ui_component_common.h"
#include "ui/theme/ui_theme_styles.h"
#include "lvgl.h"

namespace Ui::Component::Switch {

using Ui::Component::Common::flex_row;
using Ui::Component::Common::set_min_touch;

/* Aplica variante visual al lv_switch (track y knob) usando palette de styles. */
static void apply_variant(lv_obj_t* sw, Ui::UiThemeStyles& s, Variant v) {
    if (!sw) return;

    // Colores base
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

    // Inactivo: usa superficie
    lv_obj_set_style_bg_color(sw, s.tokens.colorSurface, LV_PART_MAIN);
    lv_obj_set_style_bg_opa  (sw, s.tokens.opaEnabled, LV_PART_MAIN);

    // Activo: color de la variante (para parte MAIN cuando CHECKED)
    lv_obj_set_style_bg_color(sw, active, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa  (sw, s.tokens.opaEnabled, LV_PART_MAIN | LV_STATE_CHECKED);

    // Knob: contraste sobre el track
    lv_obj_set_style_bg_color(sw, s.tokens.colorOnSurface, LV_PART_KNOB);
    lv_obj_set_style_bg_color(sw, onActive,                LV_PART_KNOB | LV_STATE_CHECKED);

    // Bordes suaves
    lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_radius(sw, LV_RADIUS_CIRCLE, LV_PART_KNOB);
}

/* Crea label con estilo tipografico Body. */
static lv_obj_t* make_label(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* txt) {
    if (!txt || !txt[0]) return nullptr;
    lv_obj_t* lbl = lv_label_create(parent);
    lv_obj_set_style_text_font(lbl, s.tokens.fontBody, LV_PART_MAIN);
    lv_obj_set_style_text_color(lbl, s.tokens.colorText, LV_PART_MAIN);
    lv_label_set_text(lbl, txt);
    return lbl;
}

/* Tacto minimo razonable para switch (accesibilidad). */
static void ensure_touch_size(lv_obj_t* sw) {
    // Para un RGB 7" es comodo ~44px; si la altura es menor, subela.
    set_min_touch(sw, 44);
}

/* Construccion del arbol: [lblL] [switch] [lblR] */
Handle create(lv_obj_t* parent, Ui::UiThemeStyles& styles, const Props& p, const Callbacks& cb) {
    Ui::themeInitOnce();

    Handle h{};
    h.root = lv_obj_create(parent);
    lv_obj_remove_style_all(h.root); // contenedor desnudo
    flex_row(h.root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, /*gap*/8);

    // Etiqueta izquierda
    h.lblL = make_label(h.root, styles, p.labelLeft);

    // Switch
    h.sw = lv_switch_create(h.root);
    apply_variant(h.sw, styles, p.variant);
    ensure_touch_size(h.sw);

    // Estado inicial y flags
    if (p.checked)  lv_obj_add_state(h.sw, LV_STATE_CHECKED);
    if (!p.enabled) lv_obj_add_state(h.sw, LV_STATE_DISABLED);

    // Etiqueta derecha
    h.lblR = make_label(h.root, styles, p.labelRight);

    // Eventos
    if (cb.onChange) {
        // Capturamos por valor cb y userData via puntero const (como en Button)
        lv_obj_add_event_cb(h.sw, [](lv_event_t* e){
            auto* obj  = lv_event_get_target(e);
            auto* user = static_cast<const Callbacks*>(lv_event_get_user_data(e));
            Handle hh{ lv_obj_get_parent(obj), obj, nullptr, nullptr };
            bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
            user->onChange(hh, checked, nullptr);
        }, LV_EVENT_VALUE_CHANGED, (void*)&cb);
    }

    return h;
}

void setChecked(Handle& h, bool v) {
    if (!h.sw) return;
    if (v) lv_obj_add_state(h.sw, LV_STATE_CHECKED);
    else   lv_obj_clear_state(h.sw, LV_STATE_CHECKED);
}

bool isChecked(const Handle& h) {
    return h.sw && lv_obj_has_state(h.sw, LV_STATE_CHECKED);
}

void setEnabled(Handle& h, bool v) {
    if (!h.sw) return;
    if (v) lv_obj_clear_state(h.sw, LV_STATE_DISABLED);
    else   lv_obj_add_state(h.sw, LV_STATE_DISABLED);
}

void setLabels(Handle& h, Ui::UiThemeStyles& s, const char* leftTxt, const char* rightTxt) {
    // Izquierda
    if (leftTxt && leftTxt[0]) {
        if (!h.lblL) h.lblL = make_label(h.root, s, leftTxt);
        else lv_label_set_text(h.lblL, leftTxt);
    } else if (h.lblL) {
        lv_obj_del(h.lblL); h.lblL = nullptr;
    }
    // Derecha
    if (rightTxt && rightTxt[0]) {
        if (!h.lblR) h.lblR = make_label(h.root, s, rightTxt);
        else lv_label_set_text(h.lblR, rightTxt);
    } else if (h.lblR) {
        lv_obj_del(h.lblR); h.lblR = nullptr;
    }
}

} // namespace Ui::Component::Switch
