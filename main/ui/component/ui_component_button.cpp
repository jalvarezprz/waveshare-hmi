/**
 * @file ui_component_button.cpp
 * @brief Implementacion del componente Button (creacion y mutadores).
 * @ingroup ui_component_button
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"
#include "lvgl.h"

namespace Ui::Component::Button {

/** @brief Selector tipado (OR de part/state sin warnings). */
static inline lv_style_selector_t sel(lv_part_t part, lv_state_t state) {
    return static_cast<lv_style_selector_t>(part | state);
}

/**
 * @brief Aplica la variante visual seleccionada usando ThemeStyles.
 * @param btn     Objeto raiz (lv_btn).
 * @param s       Styles activos.
 * @param v       Variante a aplicar.
 * @param setSize Si true, aplica tamano desde tokens (ancho/alto).
 *
 * Nota: Los estados PRESSED/FOCUSED/Disabled se derivan de las recetas.
 */
static void apply_variant(lv_obj_t* btn, UiThemeStyles& s, Variant v, bool setSize) {
    switch (v) {
        case Variant::Primary:    Ui::applyButtonPrimary  (btn, s, setSize); break;
        case Variant::Secondary:  Ui::applyButtonSecondary(btn, s, setSize); break;
        case Variant::Ghost:      Ui::applyButtonGhost    (btn, s, setSize); break;
        case Variant::Destructive:{
            Ui::applyButtonSecondary(btn, s, setSize);
            lv_obj_set_style_bg_color(btn, s.tokens.colorError, LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, s.tokens.colorOnError, LV_PART_MAIN);
            lv_obj_set_style_bg_color(btn,
                lv_color_mix(s.tokens.colorSurface, s.tokens.colorError, s.tokens.opaPressed),
                sel(LV_PART_MAIN, LV_STATE_PRESSED));
            break;
        }
        case Variant::Success: {
            Ui::applyButtonSecondary(btn, s, setSize);
            lv_obj_set_style_bg_color(btn, s.tokens.colorSuccess, LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, s.tokens.colorOnSuccess, LV_PART_MAIN);
            lv_obj_set_style_bg_color(btn,
                lv_color_mix(s.tokens.colorSurface, s.tokens.colorSuccess, s.tokens.opaPressed),
                sel(LV_PART_MAIN, LV_STATE_PRESSED));
            break;
        }
        case Variant::Warning: {
            Ui::applyButtonSecondary(btn, s, setSize);
            lv_obj_set_style_bg_color(btn, s.tokens.colorWarning, LV_PART_MAIN);
            lv_obj_set_style_text_color(btn, s.tokens.colorOnWarning, LV_PART_MAIN);
            lv_obj_set_style_bg_color(btn,
                lv_color_mix(s.tokens.colorSurface, s.tokens.colorWarning, s.tokens.opaPressed),
                sel(LV_PART_MAIN, LV_STATE_PRESSED));
            break;
        }
    }
}

/**
 * @copydoc Ui::Component::Button::create
 */
Handle create(lv_obj_t* parent, UiThemeStyles& styles, const Props& p, const Callbacks& cb) {
    Ui::themeInitOnce();

    Handle h{};
    h.root = lv_btn_create(parent);

    // Flags y estados iniciales
    if (p.toggle)  lv_obj_add_flag(h.root, LV_OBJ_FLAG_CHECKABLE);
    if (!p.enabled) lv_obj_add_state(h.root, LV_STATE_DISABLED);
    if (p.checked) lv_obj_add_state(h.root, LV_STATE_CHECKED);

    // Variante visual
    apply_variant(h.root, styles, p.variant, /*setSize=*/true);

    // Hijos: icono y texto (orden segun iconPos)
    if (p.icon && p.icon[0] && p.iconPos != IconPos::None) {
        h.icon = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.icon, styles.tokens.fontIcon, LV_PART_MAIN);
        lv_label_set_text(h.icon, p.icon);
    }
    if (p.text && p.text[0] && p.iconPos != IconPos::Only) {
        h.label = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.label, styles.tokens.fontBody, LV_PART_MAIN);
        lv_label_set_text(h.label, p.text);
    }

    // Layout interno
    lv_obj_set_flex_flow(h.root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(h.root, styles.tokens.btnIconGap, LV_PART_MAIN);
    if (h.icon && h.label && p.iconPos == IconPos::Right) {
        lv_obj_move_to_index(h.label, 0); // texto primero
    }

    // Loading
    if (p.loading) {
        lv_obj_add_state(h.root, LV_STATE_DISABLED);
        // (opcional) spinner...
    }

    // Eventos -> callbacks
    if (cb.onClick) {
        lv_obj_add_event_cb(h.root, [](lv_event_t* e){
            auto* obj  = lv_event_get_target(e);
            auto* user = static_cast<const Callbacks*>(lv_event_get_user_data(e));
            Handle hh{ obj, nullptr, nullptr };
            user->onClick(hh, nullptr);
        }, LV_EVENT_CLICKED, (void*)&cb);
    }
    if (p.toggle && cb.onToggle) {
        lv_obj_add_event_cb(h.root, [](lv_event_t* e){
            auto* obj  = lv_event_get_target(e);
            auto* user = static_cast<const Callbacks*>(lv_event_get_user_data(e));
            Handle hh{ obj, nullptr, nullptr };
            bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
            user->onToggle(hh, checked, nullptr);
        }, LV_EVENT_VALUE_CHANGED, (void*)&cb);
    }
    if (cb.onLong) {
        lv_obj_add_event_cb(h.root, [](lv_event_t* e){
            auto* obj  = lv_event_get_target(e);
            auto* user = static_cast<const Callbacks*>(lv_event_get_user_data(e));
            Handle hh{ obj, nullptr, nullptr };
            user->onLong(hh, nullptr);
        }, LV_EVENT_LONG_PRESSED, (void*)&cb);
    }

    return h;
}

/**
 * @copydoc Ui::Component::Button::setEnabled
 */
void setEnabled(Handle& h, UiThemeStyles&, bool enabled) {
    if (!h.root) return;
    if (enabled) lv_obj_clear_state(h.root, LV_STATE_DISABLED);
    else         lv_obj_add_state  (h.root, LV_STATE_DISABLED);
}

/**
 * @copydoc Ui::Component::Button::setChecked
 */
void setChecked(Handle& h, UiThemeStyles&, bool checked) {
    if (!h.root) return;
    if (checked) lv_obj_add_state(h.root, LV_STATE_CHECKED);
    else         lv_obj_clear_state(h.root, LV_STATE_CHECKED);
}

/**
 * @copydoc Ui::Component::Button::setLoading
 */
void setLoading(Handle& h, UiThemeStyles&, bool loading) {
    if (!h.root) return;
    if (loading) {
        lv_obj_add_state(h.root, LV_STATE_DISABLED);
        // (opcional) spinner...
    } else {
        lv_obj_clear_state(h.root, LV_STATE_DISABLED);
    }
}

/**
 * @copydoc Ui::Component::Button::setText
 */
void setText(Handle& h, UiThemeStyles& s, const char* txt) {
    if (!h.label) {
        h.label = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.label, s.tokens.fontBody, LV_PART_MAIN);
    }
    lv_label_set_text(h.label, txt ? txt : "");
}

/**
 * @copydoc Ui::Component::Button::setIcon
 */
void setIcon(Handle& h, UiThemeStyles& s, const char* icon, IconPos pos) {
    if (!icon || !icon[0]) { if (h.icon) { lv_obj_del(h.icon); h.icon=nullptr; } return; }
    if (!h.icon) {
        h.icon = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.icon, s.tokens.fontIcon, LV_PART_MAIN);
    }
    lv_label_set_text(h.icon, icon);
    // Reorden segun pos (simple; si necesitas robustez, guarda orden en user data)
    if (h.icon && h.label) {
        if (pos == IconPos::Right) lv_obj_move_to_index(h.label, 0);
        else                       lv_obj_move_to_index(h.icon, 0);
    }
}

bool isChecked(const Handle& h) { return h.root && lv_obj_has_state(h.root, LV_STATE_CHECKED); }

void focus(Handle& h)           { if (h.root) lv_group_focus_obj(h.root); }

} // namespace Ui::Component::Button
