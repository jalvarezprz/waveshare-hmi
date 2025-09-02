#include "ui/component/ui_component_button_presets.h"
#include "lvgl.h"

namespace ButtonPresets {

Button Back(lv_obj_t* parent, void (*onClick)(void)) {
    auto btn = Button::create(parent, "Atrás", LV_SYMBOL_LEFT, onClick);
    lv_obj_align(btn.root(), LV_ALIGN_BOTTOM_LEFT, 16, -16);
    return btn;
}

Button Forward(lv_obj_t* parent, void (*onClick)(void)) {
    auto btn = Button::create(parent, "Siguiente", LV_SYMBOL_RIGHT, onClick);
    lv_obj_align(btn.root(), LV_ALIGN_BOTTOM_RIGHT, -16, -16);
    return btn;
}

} // namespace ButtonPresets
