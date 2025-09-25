#include "ui_component_button.h"
#include <esp_log.h>
#include "ui/theme/ui_theme_styles.h"

namespace { constexpr const char* TAG = "UI.Button"; }

namespace Ui {

void Button::create(lv_obj_t* parent) {
    root_ = lv_btn_create(parent);
    if (!root_) { ESP_LOGE(TAG, "Error creando lv_btn"); return; }

    label_ = lv_label_create(root_);
    lv_label_set_text(label_, "");

    registerAllEvents();
    applyTheme();
}

void Button::setText(const char* txt) {
    if (label_) lv_label_set_text(label_, txt ? txt : "");
}

void Button::setEnabled(bool en) {
    if (!root_) return;
    if (en) lv_obj_clear_state(root_, LV_STATE_DISABLED);
    else    lv_obj_add_state(root_,   LV_STATE_DISABLED);
}

bool Button::isEnabled() const {
    return root_ && !lv_obj_has_state(root_, LV_STATE_DISABLED);
}

void Button::setOnClick(void (*cb)(void*), void* user_data) {
    onClick_ = cb;
    userData_ = user_data;
}

void Button::applyTheme() {
    auto& S = Ui::getThemeStyles();

    // 1) Cuerpo del botón (usa styles del tema: colores, padding, estados…)
    Ui::applyButtonPrimary(root_, S, /*setSize=*/false);

    // 2) Texto del botón
    if (label_) {
        // Aplica style tipográfico base (usa Montserrat si fontBody lo es)
        lv_obj_add_style(label_, &S.labelBody, 0);

        // Fuerza explícitamente la fuente Montserrat del tema (por si el style cambia)
        lv_obj_set_style_text_font(label_, S.tokens.fontBody, 0);

        // Color correcto sobre fondo "primary" y estado disabled
        lv_obj_set_style_text_color(label_, S.tokens.colorOnPrimary, 0);
        lv_obj_set_style_text_color(label_, S.tokens.colorMuted, LV_STATE_DISABLED);
    }
}

void Button::onEvent(lv_event_t* e) {
    const auto code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (onClick_) onClick_(userData_);
    }
}

} // namespace Ui
