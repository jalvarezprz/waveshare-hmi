#include "ui/component/ui_component_button_presets.h"
#include "lvgl.h"

namespace ButtonPresets {

Button Back(lv_obj_t* parent, void (*onClick)(void))
{
    // Tamaño base coherente con lo que ya vienes usando
    return Button::create(
        parent,
        "Anterior",
        LV_SYMBOL_LEFT,
        onClick,
        120, 44
    );
}

Button Forward(lv_obj_t* parent, void (*onClick)(void))
{
    return Button::create(
        parent,
        "Siguiente",
        LV_SYMBOL_RIGHT,
        onClick,
        120, 44
    );
}

} // namespace ButtonPresets
