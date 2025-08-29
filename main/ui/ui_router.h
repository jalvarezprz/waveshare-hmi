#pragma once

// Pantallas que vamos a conmutar
enum class UiScreen {
    MAIN_MENU,
    INFO_MENU,
    PARAMS_MENU,
    HW_MENU, 
    DDC_MENU, 
    BUS_MENU
};

// Cambia de pantalla (borra la actual y construye la nueva)
void ui_router_go(UiScreen s);
