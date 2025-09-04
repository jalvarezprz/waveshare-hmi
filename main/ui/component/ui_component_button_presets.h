#pragma once
/**
 * @file button_presets.h
 * @brief Factorías de botones con estilos y tamaños basados en tokens.
 * @ingroup ui_widgets
 */
#include "lvgl.h"

namespace ButtonPresets {

/**
 * @brief Botón “Atrás” (ghost) con icono y texto “Atrás”.
 * @param parent  Contenedor padre.
 * @param onClick Callback (opcional).
 * @return lv_obj_t* botón creado.
 */
lv_obj_t* Back(lv_obj_t* parent, void (*onClick)(void));

/**
 * @brief Botón primario con texto (y opcionalmente icono).
 * @param parent  Contenedor padre.
 * @param text    Texto del botón (no nullptr).
 * @param icon    Texto/ símbolo LVGL para icono (puede ser nullptr).
 * @param onClick Callback (opcional).
 * @return lv_obj_t* botón creado.
 */
lv_obj_t* Primary(lv_obj_t* parent, const char* text, const char* icon, void (*onClick)(void));

/**
 * @brief Botón secundario con texto (y opcionalmente icono).
 * @param parent  Contenedor padre.
 * @param text    Texto del botón (no nullptr).
 * @param icon    Texto/ símbolo LVGL para icono (puede ser nullptr).
 * @param onClick Callback (opcional).
 * @return lv_obj_t* botón creado.
 */
lv_obj_t* Secondary(lv_obj_t* parent, const char* text, const char* icon, void (*onClick)(void));

} // namespace ButtonPresets
