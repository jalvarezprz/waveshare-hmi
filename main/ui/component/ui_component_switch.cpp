#include "ui_component_switch.h"
#include <esp_log.h>

namespace {
constexpr const char* TAG = "UI.Switch";
}

namespace Ui {

void Switch::create(lv_obj_t* parent) {
    root_ = lv_switch_create(parent);
    if (!root_) {
        ESP_LOGE(TAG, "Error creando lv_switch");
        return;
    }

    registerAllEvents();
    applyTheme();
}

void Switch::setState(bool on) {
    if (!root_) return;
    if (on) {
        lv_obj_add_state(root_, LV_STATE_CHECKED);
    } else {
        lv_obj_clear_state(root_, LV_STATE_CHECKED);
    }
}

bool Switch::getState() const {
    if (!root_) return false;
    return lv_obj_has_state(root_, LV_STATE_CHECKED);
}

void Switch::setOnToggle(void (*cb)(bool, void*), void* user_data) {
    onToggle_ = cb;
    userData_ = user_data;
}

void Switch::applyTheme() {
    // Aquí aplicar tokens/styles según tu theme.
    // Ejemplo: lv_obj_set_size(root_, 60, 30);
}

void Switch::onEvent(lv_event_t* e) {
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        bool state = getState();
        ESP_LOGI(TAG, "Switch toggled -> %s", state ? "ON" : "OFF");
        if (onToggle_) {
            onToggle_(state, userData_);
        }
    }
}

} // namespace Ui
