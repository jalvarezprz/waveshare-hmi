#pragma once
#include "ui/component/ui_component_button.h"

namespace ButtonPresets {

    /**
     * @brief Botón “Atrás”: texto "Atrás" + LV_SYMBOL_LEFT
     * @param parent   Contenedor padre
     * @param onClick  Callback (void (*)())
     * @return Button  Wrapper con root(), etc.
     */
    Button Back(lv_obj_t* parent, void (*onClick)(void));

    /**
     * @brief Botón “Siguiente”: texto "Siguiente" (sin icono por defecto)
     *        (si quieres icono ▶ más tarde, lo añadimos)
     */
    Button Forward(lv_obj_t* parent, void (*onClick)(void));
}
