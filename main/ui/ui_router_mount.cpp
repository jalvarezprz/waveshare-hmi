#include "ui_router_mount.h"

static lv_obj_t* s_mount_parent = nullptr;

extern "C" {

void ui_router_mount_set(lv_obj_t* parent) {
    s_mount_parent = parent;
}

lv_obj_t* ui_router_mount_get(void) {
    return s_mount_parent;
}

lv_obj_t* ui_router_mount_get_or(lv_obj_t* fallback) {
    return s_mount_parent ? s_mount_parent : fallback;
}

} // extern "C"
