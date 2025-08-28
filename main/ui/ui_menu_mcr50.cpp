extern "C" {
#include "cJSON.h"   // usa la ruta que has verificado
}

#include "ui_menu_mcr50.h"
#include "esp_log.h"

static const char *TAG = "UI_MENU_MCR50";

// JSON embebido como cadena estática
static const char *menu_json_mcr50 = R"json(
{
  "menu":[
    {"id":"tend","title":"Punt. Tendencia","items":[
      {"id":"tend_buf","title":"Buffer Tendenc"},
      {"id":"tend_flash","title":"Flash EEPROM"}
    ]},
    {"id":"params","title":"Parámetros"},
    {"id":"info","title":"Inf. Sistema","items":[
      {"id":"ain","title":"Ent. Analóg"},
      {"id":"aout","title":"Sal. Analóg"},
      {"id":"din","title":"Ent. Digital"},
      {"id":"dout","title":"Sal. Digital"},
      {"id":"tot","title":"Totalizador"},
      {"id":"hours","title":"Horas Functo."}
    ]},
    {"id":"hw","title":"Conf. Hardware"},
    {"id":"ddc","title":"Ciclos DDC"},
    {"id":"bus","title":"Acceso Buswide"}
  ]
}
)json";

// Devuelve un objeto cJSON a partir del JSON embebido
cJSON* loadMenuMcr50() {
    cJSON *root = cJSON_Parse(menu_json_mcr50);
    if (!root) {
        ESP_LOGE(TAG, "Error al parsear JSON");
        return nullptr;
    }
    return root;
}

// Función de depuración
void printMenuMcr50() {
    cJSON *root = loadMenuMcr50();
    if (!root) return;

    cJSON *menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) {
        ESP_LOGE(TAG, "Estructura inválida");
        cJSON_Delete(root);
        return;
    }

    int count = cJSON_GetArraySize(menu);
    ESP_LOGI(TAG, "Menús de primer nivel (MCR50): %d", count);

    for (int i = 0; i < count; i++) {
        cJSON *item = cJSON_GetArrayItem(menu, i);
        const char *id = cJSON_GetObjectItem(item, "id")->valuestring;
        const char *title = cJSON_GetObjectItem(item, "title")->valuestring;

        ESP_LOGI(TAG, " %d) %s -> %s", i + 1, id, title);
    }

    cJSON_Delete(root);
}
