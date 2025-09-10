// ui/menu/ui_menu_tree.cpp
#include "ui_menu_tree.h"
#include <cstring>

/* =========================================================================================
 *  JSON embebido (un documento por pantalla). Cargarás SOLO el que te pidan.
 * ========================================================================================= */

const char UI_SCREEN_MAIN[] = R"json(
{
  "schemaVersion": 1,
  "id": "main",
  "view": "menu_grid",
  "title": "Principal",
  "elements": [
    { "id": "tend",   "title": "Punt. Tendencia", "icon": "trend",    "action": "NAV:/screen/tend"   },
    { "id": "params", "title": "Parámetros",      "icon": "gear",     "action": "NAV:/screen/params" },
    { "id": "info",   "title": "Inf. Sistema",    "icon": "info",     "action": "NAV:/screen/info"   },
    { "id": "hw",     "title": "Conf. Hardware",  "icon": "chip",     "action": "NAV:/screen/hw"     },
    { "id": "ddc",    "title": "Ciclos DDC",      "icon": "repeat",   "action": "NAV:/screen/ddc"    },
    { "id": "bus",    "title": "Acceso Buswide",  "icon": "bus",      "action": "NAV:/screen/bus"    },
    { "id": "test1",  "title": "Prueba Uno",      "icon": "star",     "action": "NAV:/screen/test1" },
    { "id": "test2",  "title": "Prueba Dos",      "icon": "flash",    "action": "NAV:/screen/test2" },
    { "id": "test3",  "title": "Prueba Tres",     "icon": "hammer",   "action": "NAV:/screen/test3" },
    { "id": "panel",  "title": "Panel Pruebas",   "icon": "settings", "action": "NAV:/screen/panel" }
  ]
}
)json";


const char UI_SCREEN_TEND[] = R"json(
{
  "schemaVersion": 1,
  "id": "tend",
  "view": "menu_list",
  "title": "Punt. Tendencia",
  "elements": [
    { "id": "tend_buf",   "title": "Buffer Tendenc", "icon": "buffer", "action": "DO:/tend/buffer" },
    { "id": "tend_flash", "title": "Flash EEPROM",   "icon": "memory", "action": "DO:/tend/flash_eeprom" }
  ]
}
)json";

const char UI_SCREEN_INFO[] = R"json(
{
  "schemaVersion": 1,
  "id": "info",
  "view": "menu_list",
  "title": "Inf. Sistema",
  "elements": [
    { "id": "T_DEP_ALTA",     "title": "T_DEP_ALTA",     "icon": "temp",   "action": "NAV:/detail/T_DEP_ALTA" },
    { "id": "T_DEP_BAJA",     "title": "T_DEP_BAJA",     "icon": "temp",   "action": "NAV:/detail/T_DEP_BAJA" },
    { "id": "T_IDA_CALD",     "title": "T_IDA_CALD",     "icon": "number", "action": "NAV:/detail/T_IDA_CALD" },
    { "id": "T_IDA_FANCOILS", "title": "T_IDA_FANCOILS", "icon": "number", "action": "NAV:/detail/T_IDA_FANCOILS" }
  ]
}
)json";

const char UI_SCREEN_PARAMS[] = R"json(
{
  "schemaVersion": 1,
  "id": "params",
  "view": "menu_list",
  "title": "Parámetros",
  "elements": [
    { "id": "param_general", "title": "Generales", "icon": "settings", "action": "NAV:/screen/params_general" }
  ]
}
)json";

const char UI_SCREEN_HW[] = R"json(
{
  "schemaVersion": 1,
  "id": "hw",
  "view": "menu_list",
  "title": "Conf. Hardware",
  "elements": [
    { "id": "io",   "title": "Entradas/Salidas", "icon": "io",   "action": "NAV:/screen/hw_io" },
    { "id": "bus",  "title": "Bus",              "icon": "bus",  "action": "NAV:/screen/bus"   }
  ]
}
)json";

const char UI_SCREEN_DDC[] = R"json(
{
  "schemaVersion": 1,
  "id": "ddc",
  "view": "menu_list",
  "title": "Ciclos DDC",
  "elements": [
    { "id": "ciclo_a", "title": "Ciclo A", "icon": "repeat", "action": "DO:/ddc/run_a" },
    { "id": "ciclo_b", "title": "Ciclo B", "icon": "repeat", "action": "DO:/ddc/run_b" }
  ]
}
)json";

const char UI_SCREEN_BUS[] = R"json(
{
  "schemaVersion": 1,
  "id": "bus",
  "view": "menu_list",
  "title": "Acceso Buswide",
  "elements": [
    { "id": "scan",   "title": "Escanear nodos", "icon": "search", "action": "DO:/bus/scan" },
    { "id": "status", "title": "Estado bus",     "icon": "status", "action": "DO:/bus/status" }
  ]
}
)json";

const char UI_SCREEN_TEST1[] = R"json(
{
  "schemaVersion": 1,
  "id": "test1",
  "view": "menu_list",
  "title": "Pantalla de Prueba 1",
  "elements": [
    { "id": "dummy", "title": "Elemento de prueba", "icon": "dot", "action": "DO:/test1/dummy" }
  ]
}
)json";

const char UI_SCREEN_TEST2[] = R"json(
{
  "schemaVersion": 1,
  "id": "test2",
  "view": "menu_list",
  "title": "Pantalla de Prueba 2",
  "elements": [
    { "id": "dummy", "title": "Otro elemento", "icon": "dot", "action": "DO:/test2/dummy" }
  ]
}
)json";

const char UI_SCREEN_TEST3[] = R"json(
{
  "schemaVersion": 1,
  "id": "test3",
  "view": "menu_list",
  "title": "Pantalla de Prueba 3",
  "elements": [
    { "id": "dummy", "title": "Elemento final", "icon": "dot", "action": "DO:/test3/dummy" }
  ]
}
)json";

const char UI_SCREEN_PANEL[] = R"json(
{
  "schemaVersion": 1,
  "id": "panel",
  "view": "sw_temp_panel",
  "title": "Panel de Pruebas",
  "elements": [
    { "id": "sw1", "title": "Salida 1", "icon": "lv:power", "action": "DO:/io/sw1", "enabled": true,  "visible": true },
    { "id": "sw2", "title": "Salida 2", "icon": "lv:power", "action": "DO:/io/sw2", "enabled": true,  "visible": true },
    { "id": "sw3", "title": "Salida 3", "icon": "lv:power", "action": "DO:/io/sw3", "enabled": true,  "visible": true },

    { "id": "t1",  "title": "T. Depósito", "icon": "lv:warning", "enabled": true, "visible": true },
    { "id": "t2",  "title": "T. Ambiente", "icon": "lv:warning", "enabled": true, "visible": true },
    { "id": "t3",  "title": "T. Retorno",  "icon": "lv:warning", "enabled": true, "visible": true }
  ]
}
)json";

/* // --- Alias legacy opcional (descomenta si lo necesitas) ---
const char ui_menu_json_tree[] = R"json(
  // Puedes delegar al MAIN o mantener un documento monolítico de compatibilidad.
)json";
*/

/* =========================================================================================
 *  Catálogo embebido y utilidades
 * ========================================================================================= */

struct ScreenEntry {
    const char* id;
    const char* json;
};

static const ScreenEntry kScreens[] = {
    { UI_SCREEN_ID_MAIN,   UI_SCREEN_MAIN   },
    { UI_SCREEN_ID_TEND,   UI_SCREEN_TEND   },
    { UI_SCREEN_ID_INFO,   UI_SCREEN_INFO   },
    { UI_SCREEN_ID_PARAMS, UI_SCREEN_PARAMS },
    { UI_SCREEN_ID_HW,     UI_SCREEN_HW     },
    { UI_SCREEN_ID_DDC,    UI_SCREEN_DDC    },
    { UI_SCREEN_ID_BUS,    UI_SCREEN_BUS    },

    // --- nuevas pantallas de prueba ---
    { "test1", UI_SCREEN_TEST1 },
    { "test2", UI_SCREEN_TEST2 },
    { "test3", UI_SCREEN_TEST3 },
    { UI_SCREEN_ID_PANEL, UI_SCREEN_PANEL },
};


const char* ui_menu_tree_get(const char* screen_id) {
    if (!screen_id) return nullptr;
    for (const auto& e : kScreens) {
        if (std::strcmp(e.id, screen_id) == 0) return e.json;
    }
    return nullptr;
}

const char* const* ui_menu_tree_ids(size_t* out_count) {
    static const char* kIds[] = {
        UI_SCREEN_ID_MAIN,
        UI_SCREEN_ID_TEND,
        UI_SCREEN_ID_INFO,
        UI_SCREEN_ID_PARAMS,
        UI_SCREEN_ID_HW,
        UI_SCREEN_ID_DDC,
        UI_SCREEN_ID_BUS,
    };
    if (out_count) *out_count = sizeof(kIds) / sizeof(kIds[0]);
    return kIds;
}

size_t ui_menu_tree_count() {
    size_t n = 0;
    (void)ui_menu_tree_ids(&n);
    return n;
}

bool ui_menu_tree_exists(const char* screen_id) {
    return ui_menu_tree_get(screen_id) != nullptr;
}
