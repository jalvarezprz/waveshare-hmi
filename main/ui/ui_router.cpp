#include "ui_router.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui_view_menu_list.h"   // builders de pantallas
#include "ui/ui_router_mount.h"

static const char* TAG = "UI_ROUTER";

/**
 * @brief Limpia todos los hijos del contenedor indicado (seguro con LVGL).
 */
static void clear_container(lv_obj_t* container) {
    if (!container) return;
    while (lv_obj_get_child_cnt(container) > 0) {
        lv_obj_del(lv_obj_get_child(container, 0));
    }
}

void ui_router_go(UiScreen s) {
    // Usar el contenedor montado (scaffold.content()) o la pantalla activa como fallback
    lv_obj_t* parent = ui_router_mount_get_or(lv_scr_act());

    ESP_LOGI(TAG, "Navegar a pantalla %d", static_cast<int>(s));

    // Limpiar solo el contenedor objetivo (no toda la pantalla)
    clear_container(parent);

    // TODO (próximo paso): pasar 'parent' a los builders para que creen dentro de él.
    // p.ej.: ui_build_main_menu(parent);

    switch (s) {
        case UiScreen::MAIN_MENU:
            ui_build_main_menu();     // actualmente sin parámetro; crea como hasta ahora
            break;

        case UiScreen::INFO_MENU:
            ui_build_info_menu();     // actualmente sin parámetro
            break;

        default:
            break;
    }
}
