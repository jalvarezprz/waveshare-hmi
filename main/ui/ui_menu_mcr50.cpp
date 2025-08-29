#include "ui_menu_mcr50.h"
#include "esp_log.h"
#include "lvgl.h"
#include <string.h>   // strcmp, strdup, free

extern "C" {
#include "cJSON.h"
}

#include "ui_router.h"
#include "ui_pin.h"

static const char* TAG = "UI_MENU_MCR50";

// ---------------------- JSON embebido ----------------------
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
// -----------------------------------------------------------


// ===================== API JSON ============================
cJSON* loadMenuMcr50() {
    cJSON *root = cJSON_Parse(menu_json_mcr50);
    if (!root) ESP_LOGE(TAG, "Error al parsear JSON embebido");
    return root;
}

void printMenuMcr50() {
    cJSON *root = loadMenuMcr50(); if (!root) return;
    cJSON *menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { ESP_LOGE(TAG, "Estructura inválida"); cJSON_Delete(root); return; }

    ESP_LOGI(TAG, "Menús de primer nivel (MCR50): %d", cJSON_GetArraySize(menu));
    int idx = 0; cJSON *it = nullptr;
    cJSON_ArrayForEach(it, menu) {
        const char *id = cJSON_GetStringValue(cJSON_GetObjectItem(it,"id"));
        const char *tt = cJSON_GetStringValue(cJSON_GetObjectItem(it,"title"));
        ESP_LOGI(TAG, " %d) %s -> %s", ++idx, id?id:"?", tt?tt:"?");
    }
    cJSON_Delete(root);
}
// ===========================================================


// ======= Helpers =======
static bool is_protected_id(const char* id) {
    if (!id) return false;
    return !strcmp(id,"params") || !strcmp(id,"hw") || !strcmp(id,"ddc") || !strcmp(id,"bus");
}
// =======================


// ===================== UI: menú principal ==================
void ui_mcr50_build_main_menu() {
    cJSON* root = loadMenuMcr50(); if (!root) return;
    cJSON* menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { cJSON_Delete(root); return; }

    // Lista contenedora
    lv_obj_t* list = lv_list_create(lv_scr_act());
    lv_obj_set_size(list, 780, 440);     // 800x480 con márgenes
    lv_obj_center(list);

    cJSON* it = nullptr;
    cJSON_ArrayForEach(it, menu) {
        const char* id_json = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
        const char* tt      = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));

        lv_obj_t* btn = lv_list_add_btn(list, NULL, tt ? tt : "?");

        // Copia propia del ID (sobrevive al cJSON_Delete)
        char* id_copy = id_json ? strdup(id_json) : NULL;

        // CLICK
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            const char* id = (const char*) lv_event_get_user_data(e);
            if (!id) return;

            // Acceso sin PIN
            if (!strcmp(id,"info")) { ui_router_go(UiScreen::INFO_MENU); return; }
            if (!strcmp(id,"tend")) { ESP_LOGI(TAG, "Tendencias (pendiente)"); return; }

            // Acceso con PIN
            if (is_protected_id(id)) {
                ui_show_pin_dialog(2410, [id](bool ok){
                    if (!ok) return;
                    if (!strcmp(id,"params")) ui_router_go(UiScreen::PARAMS_MENU);
                    else if (!strcmp(id,"hw")) ui_router_go(UiScreen::HW_MENU);
                    else if (!strcmp(id,"ddc")) ui_router_go(UiScreen::DDC_MENU);
                    else if (!strcmp(id,"bus")) ui_router_go(UiScreen::BUS_MENU);
                });
                return;
            }

            ESP_LOGI(TAG, "Entrada no manejada: %s", id);
        }, LV_EVENT_CLICKED, (void*)id_copy);

        // DELETE: liberar la copia
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            void* ud = lv_event_get_user_data(e);
            if (ud) free(ud);
        }, LV_EVENT_DELETE, id_copy);
    }

    cJSON_Delete(root);
}
// ===========================================================


// =============== UI: submenú 'Inf. Sistema' ===================
void ui_mcr50_build_info_menu() {
    cJSON* root = loadMenuMcr50(); if (!root) return;
    cJSON* menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { cJSON_Delete(root); return; }

    // Buscar el nodo "info"
    cJSON* info = nullptr;
    cJSON_ArrayForEach(info, menu) {
        const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(info, "id"));
        if (id && !strcmp(id, "info")) break;
    }
    if (!info) { cJSON_Delete(root); return; }

    cJSON* items = cJSON_GetObjectItem(info, "items");

    // Contenedor de pantalla
    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    // Título
    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, "Inf. Sistema");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Lista de opciones
    lv_obj_t* list = lv_list_create(cont);
    lv_obj_set_size(list, 740, 320);
    lv_obj_align(list, LV_ALIGN_CENTER, 0, 10);

    if (cJSON_IsArray(items)) {
        cJSON* it = nullptr;
        cJSON_ArrayForEach(it, items) {
            const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
            const char* tt = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));

            lv_obj_t* btn = lv_list_add_btn(list, NULL, tt ? tt : "?");
            char* id_copy = id ? strdup(id) : NULL;

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                const char* id = (const char*) lv_event_get_user_data(e);
                if (id) ESP_LOGI(TAG, "Inf. Sistema -> %s", id);
                // TODO: aquí llamaremos a pantallas específicas (AIN/AOUT/DIN/DOUT/TOT/HOURS)
            }, LV_EVENT_CLICKED, id_copy);

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                void* ud = lv_event_get_user_data(e);
                if (ud) free(ud);
            }, LV_EVENT_DELETE, id_copy);
        }
    }

    // Botón ATRAS
    lv_obj_t* back = lv_btn_create(cont);
    lv_obj_set_size(back, 120, 48);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
    lv_obj_t* l = lv_label_create(back);
    lv_label_set_text(l, "ATRAS");
    lv_obj_center(l);

    lv_obj_add_event_cb(back, [](lv_event_t*){
        ui_router_go(UiScreen::MAIN_MENU);
    }, LV_EVENT_CLICKED, nullptr);

    cJSON_Delete(root);
}
// ===========================================================
