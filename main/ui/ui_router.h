#pragma once

// Pantallas que vamos a conmutar (de momento solo estas dos)
enum class UiScreen {
    MAIN_MENU,
    INFO_MENU
};

// Cambia de pantalla (borra la actual y construye la nueva)
void ui_router_go(UiScreen s);
