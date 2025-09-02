#pragma once
#include "ui/component/ui_component_button.h"

namespace ButtonPresets {

Button Back(lv_obj_t* parent, void (*onClick)(void));
Button Forward(lv_obj_t* parent, void (*onClick)(void));

} // namespace ButtonPresets
