#include "ui_router_mount.h"
#include "lvgl.h"

static lv_obj_t* s_mount_parent = nullptr;
static lv_obj_t* s_mount_screen = nullptr;  // screen donde se fijó el parent

extern "C" {

void ui_router_mount_set(lv_obj_t* parent) {
    s_mount_parent = parent;
    s_mount_screen = parent ? lv_obj_get_screen(parent) : nullptr;
}

lv_obj_t* ui_router_mount_get(void) {
    return s_mount_parent;
}

lv_obj_t* ui_router_mount_get_or(lv_obj_t* fallback) {
    // Si no hay parent configurado → fallback
    if (!s_mount_parent) return fallback;

    // Si el objeto parent ya no es válido → fallback
    if (!lv_obj_is_valid(s_mount_parent)) return fallback;

    // Si no conocemos el screen → fallback
    if (!s_mount_screen) return fallback;

    // Si el screen activo ha cambiado → fallback
    if (lv_scr_act() != s_mount_screen) return fallback;

    // Todo OK → usar parent
    return s_mount_parent;
}

} // extern "C"
