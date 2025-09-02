#pragma once
#include "lvgl.h"
#include <functional>

namespace Ui {

/**
 * @brief Crea un botón "Atrás" estándar con icono y tipografía del theme.
 * @param parent  Contenedor donde se inserta.
 * @param on_click Callback a ejecutar al pulsar (puede ser nullptr).
 * @return lv_obj_t* botón creado.
 */
lv_obj_t* create_back_button(lv_obj_t* parent, std::function<void()> on_click = nullptr);

} // namespace Ui
