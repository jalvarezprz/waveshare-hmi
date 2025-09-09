/**
 * @file ui_mockup_menu.cpp
 * @brief Stub: redirige al Router. El Scaffold + ViewFactory pintan la UI.
 */
#include "esp_log.h"
#include "ui/router/ui_router.h"

static const char* TAG = "UI_MOCKUP_MENU";

extern "C" void ui_mockup_menu_load(void)
{
    ESP_LOGI(TAG, "Redirigiendo al Router → main");
    ui_router_go_screen("main");
}

extern "C" void ui_mockup_menu_unload(void)
{
    // No-op: el Router/Scaffold gestionan el ciclo de vida
}
