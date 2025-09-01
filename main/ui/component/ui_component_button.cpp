#include "ui/component/ui_component_button.h"
#include "ui/theme/ui_theme_styles.h"

namespace Ui {

UiButton::UiButton(const char* text, ButtonRole role, Callback onClick)
: text_(text), role_(role), onClick_(onClick) {}

lv_obj_t* UiButton::create(lv_obj_t* parent) {
    Ui::themeInitOnce();

    lv_obj_t* btn = lv_btn_create(parent);
    auto& s = Ui::getThemeStyles();

    switch (role_) {
        case ButtonRole::Primary:   lv_obj_add_style(btn, &s.btnPrimary,   LV_PART_MAIN); break;
        case ButtonRole::Secondary: lv_obj_add_style(btn, &s.btnSecondary, LV_PART_MAIN); break;
        case ButtonRole::Ghost:     lv_obj_add_style(btn, &s.btnGhost,     LV_PART_MAIN); break;
    }

    // Label interno
    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, text_ ? text_ : ""); // hereda color del botón/estilo

    // Evento opcional
    if (onClick_) {
        lv_obj_add_event_cb(btn, onClick_, LV_EVENT_CLICKED, nullptr);
    }

    // Altura mínima amistosa al dedo
    lv_obj_set_style_min_height(btn, 36, LV_PART_MAIN);
    return btn;
}

} // namespace Ui
