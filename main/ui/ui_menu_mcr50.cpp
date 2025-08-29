#include "ui_menu_mcr50.h"
#include "esp_log.h"
#include "lvgl.h"

#include <string.h>    // strcmp, strdup, free
#include <vector>
#include <string>

extern "C" {
#include "cJSON.h"
}

#include "ui_router.h"
#include "ui_pin.h"

static const char* TAG = "UI_MENU_MCR50";

/* ============================================================
 * JSON embebido (con hijos de "Ent. Analóg")
 * ==========================================================*/
// ---------------------- JSON embebido ----------------------
static const char *menu_json_mcr50 = R"json(
{
  "menu": [
    {
      "id": "tend",
      "title": "Punt. Tendencia",
      "items": [
        { "id": "tend_buf",   "title": "Buffer Tendenc" },
        { "id": "tend_flash", "title": "Flash EEPROM"   }
      ]
    },
    { "id": "params", "title": "Parámetros" },
    {
      "id": "info",
      "title": "Inf. Sistema",
      "items": [
        {
          "id": "ain",
          "title": "Ent. Analóg",
          "items": [
            { "id": "T_IDA_SUELO",      "title": "T_IDA_SUELO" },
            { "id": "T_RET_MAQUINA1",   "title": "T_RET_MAQUINA1" },
            { "id": "Z_MCX4_1",         "title": "Z_MCX4_1" },
            { "id": "Z_MCX4_2",         "title": "Z_MCX4_2" },
            {
              "id": "T_DEP_ALTA",
              "title": "T_DEP_ALTA",
              "items": [
                { "id": "histeresis", "title": "Histeresis Tend.", "value": "2.5 ºC" },
                { "id": "ciclo",      "title": "Ciclo Tend.",      "value": "15 min" },
                { "id": "output",     "title": "Output",           "value": "1 (ON)" },
                { "id": "input",      "title": "Input",            "value": "0 (OFF)" },
                { "id": "dirtec",     "title": "Dir. Tec." },
                { "id": "habtend",    "title": "Hab. Tend." },
                { "id": "maxlim",     "title": "Max lim1/lim2" },
                { "id": "minlim",     "title": "Min lim1/lim2" },
                { "id": "soffset",    "title": "S. Offset" },
                { "id": "suprimalm",  "title": "Suprim. Alm." },
                { "id": "valor",      "title": "Valor actual" }
              ]
            },
            { "id": "T_DEP_BAJA",      "title": "T_DEP_BAJA" },
            { "id": "T_IDA_CALD",      "title": "T_IDA_CALD" },
            { "id": "T_IDA_FANCOILS",  "title": "T_IDA_FANCOILS" }
          ]
        },
        { "id": "aout",  "title": "Sal. Analóg"  },
        { "id": "din",   "title": "Ent. Digital" },
        { "id": "dout",  "title": "Sal. Digital" },
        { "id": "tot",   "title": "Totalizador"  },
        { "id": "hours", "title": "Horas Functo."}
      ]
    },
    { "id": "hw",  "title": "Conf. Hardware" },
    { "id": "ddc", "title": "Ciclos DDC"     },
    { "id": "bus", "title": "Acceso Buswide" }
  ]
}
)json";

// -----------------------------------------------------------


/* ==================== Carga / utilidades JSON =================== */
cJSON* loadMenuMcr50() {
    cJSON *root = cJSON_Parse(menu_json_mcr50);
    if (!root) {
        ESP_LOGE(TAG, "Error al parsear JSON embebido");
        const char* ep = cJSON_GetErrorPtr();
        if (ep) {
            // Imprime 40 chars antes y después del fallo
            const char* start = ep - 40 > menu_json_mcr50 ? ep - 40 : menu_json_mcr50;
            char ctx[120] = {0};
            size_t len = 0;
            while (start[len] && &start[len] < ep + 40 && len < sizeof(ctx)-1) { ctx[len] = start[len]; len++; }
            ctx[len] = '\0';
            ESP_LOGE(TAG, "Contexto cercano a error: >>>%s<<<", ctx);
        }
    }
    return root;
}


void printMenuMcr50() {
    cJSON *root = loadMenuMcr50(); if (!root) return;
    cJSON *menu = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(menu)) { ESP_LOGE(TAG, "Estructura inválida"); cJSON_Delete(root); return; }
    ESP_LOGI(TAG, "Entradas raíz: %d", cJSON_GetArraySize(menu));
    int i = 0; cJSON *it = nullptr;
    cJSON_ArrayForEach(it, menu) {
        const char *id = cJSON_GetStringValue(cJSON_GetObjectItem(it,"id"));
        const char *tt = cJSON_GetStringValue(cJSON_GetObjectItem(it,"title"));
        ESP_LOGI(TAG, " %d) %s -> %s", ++i, id?id:"?", tt?tt:"?");
    }
    cJSON_Delete(root);
}

/* ======================== Router helpers ======================== */
static bool is_protected_root_id(const char* id) {
    if (!id) return false;
    return !strcmp(id,"params") || !strcmp(id,"hw") || !strcmp(id,"ddc") || !strcmp(id,"bus");
}

/* Ruta actual dentro del árbol de menús (lista de IDs) */
static std::vector<std::string> g_path;

/* Busca el nodo (objeto JSON) que corresponde a g_path.
   Devuelve además el array de sus hijos en `*out_array`
   y el título en `*out_title`. */
static bool find_node_by_path(cJSON* root,
                              cJSON** out_array,    // "menu" o "items"
                              const char** out_title)
{
    cJSON* node_array = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(node_array)) return false;

    const char* title = "Menú principal";
    cJSON* current = nullptr;

    if (g_path.empty()) {
        *out_array = node_array;
        *out_title = title;
        return true;
    }

    // Recorre cada id de la ruta
    for (size_t depth = 0; depth < g_path.size(); ++depth) {
        const char* want = g_path[depth].c_str();

        cJSON* it = nullptr;
        cJSON* found = nullptr;
        cJSON_ArrayForEach(it, node_array) {
            const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(it,"id"));
            if (id && strcmp(id, want) == 0) { found = it; break; }
        }
        if (!found) return false;

        current = found;
        title   = cJSON_GetStringValue(cJSON_GetObjectItem(current,"title"));
        node_array = cJSON_GetObjectItem(current, "items");
        if (!node_array) { // hoja
            *out_array = nullptr;
            *out_title = title ? title : "";
            return true;
        }
        if (!cJSON_IsArray(node_array)) return false;
    }

    *out_array = node_array;
    *out_title = title ? title : "";
    return true;
}

/* ======================= Render genérico ======================== */

typedef struct {
    char* id;           // id del item
    bool  has_children; // si tiene "items"
} MenuItemUD;

static void ui_mcr50_show_menu_generic()
{
    /* Limpia la pantalla actual */
    lv_obj_clean(lv_scr_act());

    /* Vuelve a cargar el JSON y resuelve la ruta */
    cJSON* root = loadMenuMcr50(); if (!root) return;

    cJSON* children = nullptr;
    const char* title_txt = "Menú";
    if (!find_node_by_path(root, &children, &title_txt)) {
        ESP_LOGE(TAG, "Ruta inválida");
        cJSON_Delete(root);
        return;
    }

    /* Contenedor base */
    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    /* Título */
    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Menú");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    /* Lista de opciones (si tiene hijos); si es hoja, mostramos aviso */
    if (children && cJSON_IsArray(children)) {
        lv_obj_t* list = lv_list_create(cont);
        lv_obj_set_size(list, 740, 320);
        lv_obj_align(list, LV_ALIGN_CENTER, 0, 10);

        cJSON* it = nullptr;
        cJSON_ArrayForEach(it, children) {
            const char* id   = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
            const char* text = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));
            cJSON* items     = cJSON_GetObjectItem(it, "items");
            bool has_children = items && cJSON_IsArray(items);

            lv_obj_t* btn = lv_list_add_btn(list, NULL, text ? text : "?");

            // 🔹 Añadir valor dinámico si está en el JSON
            const char* val = cJSON_GetStringValue(cJSON_GetObjectItem(it, "value"));
            if (val) {
                // Opcional: dar ancho completo al botón
                lv_obj_set_width(btn, LV_PCT(100));

                // Label adicional a la derecha
                lv_obj_t* lbl_val = lv_label_create(btn);
                lv_label_set_text(lbl_val, val);
                lv_label_set_long_mode(lbl_val, LV_LABEL_LONG_CLIP);
                lv_obj_set_width(lbl_val, LV_SIZE_CONTENT);
                lv_obj_align(lbl_val, LV_ALIGN_RIGHT_MID, -10, 0);
            }

            MenuItemUD* ud = (MenuItemUD*) malloc(sizeof(MenuItemUD));
            ud->id = id ? strdup(id) : NULL;
            ud->has_children = has_children;

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MenuItemUD* ud = (MenuItemUD*) lv_event_get_user_data(e);
                if (!ud || !ud->id) return;

                /* Si estamos en raíz y es protegido, pedimos PIN antes de entrar */
                if (g_path.empty() && is_protected_root_id(ud->id)) {
                    ui_show_pin_dialog(2410, [id_copy = std::string(ud->id)](bool ok){
                        if (!ok) return;
                        g_path.push_back(id_copy);
                        ui_mcr50_show_menu_generic();
                    });
                    return;
                }

                if (ud->has_children) {
                    g_path.push_back(ud->id);
                    ui_mcr50_show_menu_generic();
                } else {
                    ESP_LOGI(TAG, "Leaf selected: %s (TODO: acción)", ud->id);
                    // Aquí podrás abrir una pantalla funcional específica.
                }
            }, LV_EVENT_CLICKED, ud);

            /* Liberar el user_data cuando se destruye el botón */
            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MenuItemUD* ud = (MenuItemUD*) lv_event_get_user_data(e);
                if (ud) {
                    if (ud->id) free(ud->id);
                    free(ud);
                }
            }, LV_EVENT_DELETE, ud);
        }
    } else {
        lv_obj_t* info = lv_label_create(cont);
        lv_label_set_text(info, "Elemento sin submenús (TODO: acción específica)");
        lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);
    }

    /* Botón ATRÁS (solo si no estamos en la raíz) */
    if (!g_path.empty()) {
        lv_obj_t* back = lv_btn_create(cont);
        lv_obj_set_size(back, 120, 48);
        lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
        lv_obj_t* l = lv_label_create(back);
        lv_label_set_text(l, "ATRAS");
        lv_obj_center(l);

        lv_obj_add_event_cb(back, [](lv_event_t*){
            if (!g_path.empty()) g_path.pop_back();
            if (g_path.empty()) {
                /* Si volvemos a raíz, puedes seguir aquí o delegar al router */
                ui_router_go(UiScreen::MAIN_MENU);
            } else {
                ui_mcr50_show_menu_generic();
            }
        }, LV_EVENT_CLICKED, nullptr);
    }

    cJSON_Delete(root);
}

/* ===================== Entry points públicos ==================== */

void ui_mcr50_build_main_menu() {
    g_path.clear();
    ui_mcr50_show_menu_generic();
}

void ui_mcr50_build_info_menu() {
    g_path.clear();
    g_path.push_back("info");
    ui_mcr50_show_menu_generic();
}

/* Si quisieras entradas dedicadas para otros de raíz:
void ui_mcr50_build_tend_menu() { g_path = {"tend"}; ui_mcr50_show_menu_generic(); }
void ui_mcr50_build_params_menu() { g_path = {"params"}; ui_mcr50_show_menu_generic(); } // pedirá PIN en raíz
*/
