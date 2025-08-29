#include "ui_router.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui_menu_mcr50.h"   // builders de pantallas

static const char* TAG = "UI_ROUTER";

static void clear_active_screen() {
    // Elimina todos los hijos de la pantalla activa; seguro con LVGL
    lv_obj_clean(lv_scr_act());
}

void ui_router_go(UiScreen s) {
    ESP_LOGI(TAG, "Navegar a pantalla %d", static_cast<int>(s));
    clear_active_screen();

    switch (s) {
        case UiScreen::MAIN_MENU:
            ui_mcr50_build_main_menu();
            break;
        case UiScreen::INFO_MENU:
            ui_mcr50_build_info_menu();   // ver stub más abajo
            break;
        default:
            break;
    }
}
