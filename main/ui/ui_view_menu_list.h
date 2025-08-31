/**
 * @file ui_view_menu_list.h
 * @brief Entrypoints de la vista de menú en forma de lista.
 *
 * Expone constructores de pantalla para el menú principal y para la sección “info”.
 * Mantiene wrappers de compatibilidad hacia las utilidades JSON.
 * @ingroup ui_menu
 */

#pragma once

#include "ui_menu_json_utilities.h"  // ui_menu_json_load / ui_menu_json_print

// Forward mínimo de LVGL para no arrastrar lvgl.h a todos los consumidores.
struct _lv_obj_t;
typedef _lv_obj_t lv_obj_t;

/**
 * @brief Construye el menú de primer nivel en la pantalla activa.
 */
void ui_build_main_menu();

/**
 * @brief Construye directamente la vista “info” del menú.
 */
void ui_build_info_menu();
