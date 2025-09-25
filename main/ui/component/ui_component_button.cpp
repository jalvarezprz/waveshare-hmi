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
    auto& S = Ui::getThemeStyles();     // ← define S

    // Styles del tema (btnPrimary_main/label)
    lv_obj_add_style(root_,  &S.btnPrimary,  LV_PART_MAIN);
    if (label_) {
        lv_obj_add_style(label_, &S.btnPrimary, 0);
    }
}

void Button::onEvent(lv_event_t* e) {
    const auto code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        if (onClick_) onClick_(userData_);
    }
}

} // namespace Ui
