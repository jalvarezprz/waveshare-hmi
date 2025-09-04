/**
 * @file ui_component_button.cpp
 * @brief Implementación de un componente de botón reutilizable.
 * @ingroup ui_component
 */

#include "ui/component/ui_component_button.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/theme/ui_theme_tokens.h"
#include "lvgl.h"

namespace Ui::Component
{

// Helper para selector tipado (evita warnings por OR entre enums distintos)
static inline lv_style_selector_t sel(lv_part_t part, lv_state_t state) {
    return static_cast<lv_style_selector_t>(part | state);
}

/**
 * @brief Crea un botón primario con estilos del tema y estados pressed/focused.
 *        Mantiene la firma existente en tu header.
 */
lv_obj_t* create_button_primary(lv_obj_t* parent, const char* text, lv_event_cb_t cb)
{
    Ui::themeInitOnce();
    auto& styles = Ui::getThemeStyles();

    // Botón base
    lv_obj_t* btn = lv_btn_create(parent);
    Ui::applyButtonPrimary(btn, styles, /*setSize=*/true);

    // Estados via tokens (sin LV_OPA_* sueltos)
    lv_obj_set_style_bg_color(btn,
                              Ui::Tokens::button_primary_bg_pressed(),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    lv_obj_set_style_bg_color(btn,
                              Ui::Tokens::button_primary_bg_focused(),
                              sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    // Texto
    if (text && *text) {
        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, text);
        lv_obj_set_style_text_font(lbl, Ui::Tokens::font_body(), LV_PART_MAIN);
    }

    // Callback
    if (cb) {
        lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, nullptr);
    }

    return btn;
}

} // namespace Ui::Component
