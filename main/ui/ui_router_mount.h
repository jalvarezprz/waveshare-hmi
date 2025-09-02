#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_router_mount_set(lv_obj_t* parent);
lv_obj_t* ui_router_mount_get(void);

/**
 * @brief Devuelve el contenedor configurado si sigue en la pantalla activa;
 *        en caso contrario devuelve @p fallback (p.ej. lv_scr_act()).
 */
lv_obj_t* ui_router_mount_get_or(lv_obj_t* fallback);

#ifdef __cplusplus
}
#endif
