/**
 * @file ui_menu_json_utilities.cpp
 * @brief Implementación de utilidades para cargar e inspeccionar el árbol de menú en JSON.
 *
 * @ingroup ui_menu
 */

#include "ui_menu_json_utilities.h"
#include "esp_log.h"
#include <string>

extern "C" {
#include "cJSON.h"
}

#include "ui_menu_json_tree.h"  ///< Recurso embebido: `extern const char ui_menu_json_tree[];`

/// Etiqueta de log para este módulo.
static const char* TAG_JSON = "UI_MENU_JSON";

cJSON* ui_menu_json_load(void)
{
    // Parsear el JSON embebido definido en ui_menu_json_tree.*
    cJSON* root = cJSON_Parse(ui_menu_json_tree);
    if (!root) {
        ESP_LOGE(TAG_JSON, "Error al parsear JSON embebido");

        // Diagnóstico: mostrar un pequeño contexto alrededor del error reportado por cJSON.
        const char* ep = cJSON_GetErrorPtr();
        if (ep) {
            const char* start = (ep - 40 > ui_menu_json_tree) ? ep - 40 : ui_menu_json_tree;
            const char* end   = ep + 40;
            std::string snippet(start, end);
            ESP_LOGE(TAG_JSON, "cJSON error cerca de: >>>%s<<<", snippet.c_str());
        }
    }
    return root;
}

void ui_menu_json_print(void)
{
    cJSON* root = ui_menu_json_load();
    if (!root) return;

    cJSON* menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) {
        ESP_LOGE(TAG_JSON, "Estructura inválida: falta 'menu' como array");
        cJSON_Delete(root);
        return;
    }

    ESP_LOGI(TAG_JSON, "Entradas raíz: %d", cJSON_GetArraySize(menu));

    int i = 0;
    cJSON* it = nullptr;
    cJSON_ArrayForEach(it, menu) {
        const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
        const char* tt = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));
        ESP_LOGI(TAG_JSON, " %d) %s -> %s", ++i, id ? id : "?", tt ? tt : "?");
    }

    cJSON_Delete(root);
}

/* ---------------------------------- Ejemplo de uso ----------------------------------
 * @code{.c}
 * #include "ui_menu_json_utilities.h"
 * #include "cJSON.h"
 *
 * void ejemplo(void) {
 *     cJSON* root = ui_menu_json_load();
 *     if (!root) return;
 *
 *     // ... trabajar con el árbol ...
 *
 *     cJSON_Delete(root); // ¡Importante!
 * }
 * @endcode
 * ----------------------------------------------------------------------------------- */
