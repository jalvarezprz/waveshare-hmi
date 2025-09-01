#pragma once
#include "lvgl.h"

/**
 * @file ui_router_mount.h
 * @brief Shim para indicar al router en qué contenedor (parent) debe montar las vistas.
 *
 * Permite desacoplar las vistas del uso de `lv_scr_act()`, de modo que puedan
 * montarse dentro de un contenedor arbitrario (p.ej., el Content del scaffold).
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Establece el contenedor padre donde el router montará las vistas.
 * @param parent Objeto LVGL que actuará como parent; puede ser NULL para limpiar.
 *
 * Si es NULL, el router volverá a usar `lv_scr_act()` como destino por defecto.
 */
void ui_router_mount_set(lv_obj_t* parent);

/**
 * @brief Devuelve el contenedor configurado para montar vistas, o NULL si no hay.
 */
lv_obj_t* ui_router_mount_get(void);

/**
 * @brief Devuelve el contenedor configurado o, si no existe, el @p fallback.
 * @param fallback Objeto alternativo (típicamente `lv_scr_act()`).
 */
lv_obj_t* ui_router_mount_get_or(lv_obj_t* fallback);

#ifdef __cplusplus
}
#endif
