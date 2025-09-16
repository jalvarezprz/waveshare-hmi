#pragma once
#include "lvgl.h"

namespace Ui {

// Interfaz mínima común a todos los componentes
class UiComponent {
public:
    virtual ~UiComponent() = default;
    virtual lv_obj_t* create(lv_obj_t* parent) = 0;
};

} // namespace Ui
