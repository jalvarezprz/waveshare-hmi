#include "ui_router_adapter.h"
#include "lvgl.h"

extern "C" {
#include "cJSON.h"
}

// Sustituye este cuerpo por tu lógica real de navegación (ui_router_go, etc.)
extern "C" void ui_router_on_menu_item_selected(const cJSON* item)
{
    const char* id_str = nullptr;

    if (item) {
        // Intenta por orden: id, key, path
        const cJSON* id = cJSON_GetObjectItemCaseSensitive((cJSON*)item, "id");
        if (cJSON_IsString(id) && id->valuestring && id->valuestring[0]) {
            id_str = id->valuestring;
        } else {
            const cJSON* key = cJSON_GetObjectItemCaseSensitive((cJSON*)item, "key");
            if (cJSON_IsString(key) && key->valuestring && key->valuestring[0]) {
                id_str = key->valuestring;
            } else {
                const cJSON* path = cJSON_GetObjectItemCaseSensitive((cJSON*)item, "path");
                if (cJSON_IsString(path) && path->valuestring && path->valuestring[0]) {
                    id_str = path->valuestring;
                }
            }
        }
    }

    LV_LOG_USER("ui_router_on_menu_item_selected(): %s", id_str ? id_str : "(sin id)");

    // Ejemplo de integración (cuando quieras navegar de verdad):
    // if(id_str) {
    //     ui_router_go_by_id(id_str);    // o resuelve el siguiente nodo y llama a tu render
    // }
}
