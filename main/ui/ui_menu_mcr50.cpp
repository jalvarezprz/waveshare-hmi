extern "C" {
#include "cJSON.h"
}
#include "ui_menu_mcr50.h"
#include "esp_log.h"
#include "lvgl.h"
#include <array>
#include <string>

static const char* TAG = "UI_MENU_MCR50";

// ---------- JSON embebido (igual que ya tenías) ----------
static const char *menu_json_mcr50 = R"json(
{ "menu":[
    {"id":"tend","title":"Punt. Tendencia","items":[
      {"id":"tend_buf","title":"Buffer Tendenc"},
      {"id":"tend_flash","title":"Flash EEPROM"}]},
    {"id":"params","title":"Parámetros"},
    {"id":"info","title":"Inf. Sistema","items":[
      {"id":"ain","title":"Ent. Analóg"},
      {"id":"aout","title":"Sal. Analóg"},
      {"id":"din","title":"Ent. Digital"},
      {"id":"dout","title":"Sal. Digital"},
      {"id":"tot","title":"Totalizador"},
      {"id":"hours","title":"Horas Functo."}]},
    {"id":"hw","title":"Conf. Hardware"},
    {"id":"ddc","title":"Ciclos DDC"},
    {"id":"bus","title":"Acceso Buswide"}
]} )json";

// ---------- API JSON ----------
cJSON* loadMenuMcr50() {
    cJSON *root = cJSON_Parse(menu_json_mcr50);
    if (!root) ESP_LOGE(TAG, "Error al parsear JSON");
    return root;
}

void printMenuMcr50() {
    cJSON *root = loadMenuMcr50(); if(!root) return;
    cJSON *menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { ESP_LOGE(TAG, "Estructura inválida"); cJSON_Delete(root); return; }
    ESP_LOGI(TAG, "Menús (nivel 1): %d", cJSON_GetArraySize(menu));
    cJSON *it = nullptr; int idx = 0;
    cJSON_ArrayForEach(it, menu) {
        const char *id = cJSON_GetStringValue(cJSON_GetObjectItem(it,"id"));
        const char *tt = cJSON_GetStringValue(cJSON_GetObjectItem(it,"title"));
        ESP_LOGI(TAG, "%d) %s -> %s", ++idx, id?id:"?", tt?tt:"?");
    }
    cJSON_Delete(root);
}

// ---------- UI: menú principal ----------
static bool is_protected_id(const char* id) {
    static const std::array<const char*,4> protectedIds = {"params","hw","ddc","bus"};
    if(!id) return false;
    for (auto p: protectedIds) if (std::string(id)==p) return true;
    return false;
}

static void show_pin_note() {
    static const char* btns[] = {"OK", ""};
    lv_obj_t* m = lv_msgbox_create(NULL, "Acceso restringido",
                                   "Se requiere PIN (pantalla de clave).",
                                   btns, false);
    lv_obj_center(m);
}

static void menu_btn_event_cb(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target(e);
    const char* id   = (const char*)lv_obj_get_user_data(btn);   // requiere LV_USE_USER_DATA=1
    const char* text = lv_label_get_text(lv_obj_get_child(btn, 0));

    ESP_LOGI(TAG, "Click: %s (%s)", text?text:"<no text>", id?id:"<no id>");
    if (is_protected_id(id)) {
        show_pin_note();           // aquí dispararías tu diálogo de clave real
        return;
    }
    // TODO: navegar a submenús según 'id'
}

void ui_mcr50_build_main_menu() {
    cJSON* root = loadMenuMcr50(); if(!root) return;
    cJSON* menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { cJSON_Delete(root); return; }

    // Contenedor de toda la lista
    lv_obj_t* list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, 780, 440);          // 800x480 menos márgenes
    lv_obj_center(list);

    // Crea una entrada por ítem de nivel 1
    cJSON *it = nullptr;
    cJSON_ArrayForEach(it, menu) {
        const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
        const char* tt = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));
        lv_obj_t* btn = lv_list_add_btn(list, NULL, tt?tt:"<sin título>");
        lv_obj_add_event_cb(btn, menu_btn_event_cb, LV_EVENT_CLICKED, NULL);

        // Guarda el id como user_data (activa LV_USE_USER_DATA=1 en lv_conf.h)
        lv_obj_set_user_data(btn, (void*)id);
    }

    cJSON_Delete(root);
}
