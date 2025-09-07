#pragma once
#pragma once
#include <string_view>

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

// ------------------------------------------------------------------
// Adaptador declarativo para acciones tipo "NAV:/xxx"
// Permite usar strings de acción desde presets/componentes sin
// acoplarlos a la enum ni a los builders.
// ------------------------------------------------------------------
namespace Ui { namespace Router {

inline void dispatch(const char* action) {
    if (!action || !*action) return;
    std::string_view a(action);

         if (a == "NAV:/main")   ui_router_go(UiScreen::MAIN_MENU);
    else if (a == "NAV:/info")   ui_router_go(UiScreen::INFO_MENU);
    else if (a == "NAV:/params") ui_router_go(UiScreen::PARAMS_MENU);
    else if (a == "NAV:/hw")     ui_router_go(UiScreen::HW_MENU);
    else if (a == "NAV:/ddc")    ui_router_go(UiScreen::DDC_MENU);
    else if (a == "NAV:/bus")    ui_router_go(UiScreen::BUS_MENU);
    // else: no-op (o añade aquí logging si lo deseas)
}

}} // namespace Ui::Router
