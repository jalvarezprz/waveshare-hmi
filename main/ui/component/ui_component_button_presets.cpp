#include "ui/component/ui_component_button_presets.h"
#include "lvgl.h"

/**
 * @file button_presets.cpp
 * @brief Implementación de factorías de botón basadas en tokens/estilos.
 * @ingroup ui_widgets
 */
#include "ui_component_button_presets.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/theme/ui_theme_tokens.h"
#include <cstring>

namespace {

// Callback thin-wrapper para CLICKED usando user_data.
static void clicked_cb_thunk(lv_event_t* e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
    using Fn = void(*)(void);
    auto fn = reinterpret_cast<Fn>(lv_event_get_user_data(e));
    if (fn) fn();
}

// Crea layout de contenido (icono + texto) dentro del botón.
static void add_icon_and_text(lv_obj_t* btn, const char* text, const char* icon)
{
    // Layout interno
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(btn, Ui::Tokens::button_icon_gap(), LV_PART_MAIN);

    // Icono (opcional)
    if (icon && icon[0] != '\0') {
        lv_obj_t* icon_lbl = lv_label_create(btn);
        lv_obj_set_style_text_font(icon_lbl, Ui::Tokens::font_icon(), LV_PART_MAIN);
        lv_label_set_text(icon_lbl, icon);
    }

    // Texto (obligatorio)
    lv_obj_t* txt_lbl = lv_label_create(btn);
    lv_obj_set_style_text_font(txt_lbl, Ui::Tokens::font_body(), LV_PART_MAIN);
    lv_label_set_text(txt_lbl, (text && text[0] != '\0') ? text : "");
}

} // namespace

namespace ButtonPresets {

lv_obj_t* Back(lv_obj_t* parent, void (*onClick)(void))
{
    Ui::themeInitOnce();
    auto& styles = Ui::getThemeStyles();

    // Botón tipo ghost
    lv_obj_t* btn = lv_btn_create(parent);
    Ui::applyButtonGhost(btn, styles, /*setSize=*/true);
    add_icon_and_text(btn, "Atrás", LV_SYMBOL_LEFT);

    if (onClick) {
        lv_obj_add_event_cb(btn, clicked_cb_thunk, LV_EVENT_ALL, reinterpret_cast<void*>(onClick));
    }
    return btn;
}

lv_obj_t* Primary(lv_obj_t* parent, const char* text, const char* icon, void (*onClick)(void))
{
    Ui::themeInitOnce();
    auto& styles = Ui::getThemeStyles();

    lv_obj_t* btn = lv_btn_create(parent);
    Ui::applyButtonPrimary(btn, styles, /*setSize=*/true);
    add_icon_and_text(btn, text, icon);

    if (onClick) {
        lv_obj_add_event_cb(btn, clicked_cb_thunk, LV_EVENT_ALL, reinterpret_cast<void*>(onClick));
    }
    return btn;
}

lv_obj_t* Secondary(lv_obj_t* parent, const char* text, const char* icon, void (*onClick)(void))
{
    Ui::themeInitOnce();
    auto& styles = Ui::getThemeStyles();

    lv_obj_t* btn = lv_btn_create(parent);
    Ui::applyButtonSecondary(btn, styles, /*setSize=*/true);
    add_icon_and_text(btn, text, icon);

    if (onClick) {
        lv_obj_add_event_cb(btn, clicked_cb_thunk, LV_EVENT_ALL, reinterpret_cast<void*>(onClick));
    }
    return btn;
}

} // namespace ButtonPresets
