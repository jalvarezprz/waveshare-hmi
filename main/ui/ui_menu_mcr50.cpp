#include "ui_menu_mcr50.h"
#include "esp_log.h"
#include "lvgl.h"

#include <cstring>     // strcmp
#include <cstdlib>     // malloc, free
#include <string.h>    // ::strdup
#include <vector>
#include <string>

extern "C" {
#include "cJSON.h"
}

#include "ui_router.h"
#include "ui_pin.h"
#include "ui_menu_mcr50_json.h"

static const char* TAG = "UI_MENU_MCR50";

/* ==================== Carga / utilidades JSON =================== */

cJSON* loadMenuMcr50() {
    // Parsear el JSON embebido definido en ui_menu_mcr50_json.cpp
    cJSON* root = cJSON_Parse(ui_menu_mcr50_json);
    if (!root) {
        ESP_LOGE(TAG, "Error al parsear JSON embebido");

        // Diagnóstico opcional: mostrar un pequeño contexto para cazar el fallo de codificación
        const char* ep = cJSON_GetErrorPtr();
        if (ep) {
            const char* start = (ep - 40 > ui_menu_mcr50_json) ? ep - 40 : ui_menu_mcr50_json;
            const char* end   = ep + 40;
            std::string snippet(start, end);
            ESP_LOGE(TAG, "cJSON error cerca de: >>>%s<<<", snippet.c_str());
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
    return std::strcmp(id,"params")==0 || std::strcmp(id,"hw")==0
        || std::strcmp(id,"ddc")==0   || std::strcmp(id,"bus")==0;
}

/* Ruta actual dentro del árbol de menús (lista de IDs) */
static std::vector<std::string> g_path;

/* Busca el array de hijos y el título del nodo apuntado por g_path */
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

    for (size_t depth = 0; depth < g_path.size(); ++depth) {
        const char* want = g_path[depth].c_str();

        cJSON* it = nullptr;
        cJSON* found = nullptr;
        cJSON_ArrayForEach(it, node_array) {
            const char* id = cJSON_GetStringValue(cJSON_GetObjectItem(it,"id"));
            if (id && std::strcmp(id, want) == 0) { found = it; break; }
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

/* -------- helpers para localizar un nodo hoja concreto -------- */
static const cJSON* find_node_by_path_and_id(const cJSON* root,
                                             const std::vector<std::string>& path,
                                             const char* leaf_id)
{
    const cJSON* cur = cJSON_GetObjectItem(root, "menu");
    if(!cJSON_IsArray(cur)) return nullptr;

    for (const auto& idp : path) {
        const cJSON* it = nullptr; bool found=false;
        cJSON_ArrayForEach(it, cur) {
            const char* nid = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
            if(nid && idp == nid) {
                const cJSON* items = cJSON_GetObjectItem(it, "items");
                if(items && cJSON_IsArray(items)) { cur = items; found=true; break; }
                else return nullptr;
            }
        }
        if(!found) return nullptr;
    }

    const cJSON* it = nullptr;
    cJSON_ArrayForEach(it, cur) {
        const char* nid = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
        if(nid && leaf_id && std::strcmp(nid, leaf_id)==0) return it;
    }
    return nullptr;
}

/* ======= forward del renderer genérico de lista (SE USA MÁS ABAJO) ======= */
static void ui_mcr50_show_menu_generic();

/* ---------------- renderer detail (view:"detail") -------------- */
static void ui_render_detail_from_node(const cJSON* node) {
    lv_obj_clean(lv_scr_act());

    const char* title_txt = cJSON_GetStringValue(cJSON_GetObjectItem(node, "title"));
    const cJSON* fields   = cJSON_GetObjectItem(node, "fields");

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Detalle");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* col = lv_obj_create(cont);
    lv_obj_set_size(col, 740, 300);
    lv_obj_align(col, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 8, 0);
    lv_obj_set_style_pad_all(col, 8, 0);     // reemplazo de pad_col
    lv_obj_set_scroll_dir(col, LV_DIR_VER);

    if (fields && cJSON_IsArray(fields)) {
        const cJSON* f = nullptr;
        cJSON_ArrayForEach(f, fields) {
            const char* label = cJSON_GetStringValue(cJSON_GetObjectItem(f, "label"));
            const char* unit  = cJSON_GetStringValue(cJSON_GetObjectItem(f, "unit"));
            const char* mock  = cJSON_GetStringValue(cJSON_GetObjectItem(f, "mock"));

            lv_obj_t* row = lv_obj_create(col);
            lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
            lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
            lv_obj_set_style_pad_all(row, 6, 0);
            lv_obj_set_style_radius(row, 8, 0);

            lv_obj_t* l = lv_label_create(row);
            lv_label_set_text(l, label ? label : "-");
            lv_obj_set_width(l, 300);

            lv_obj_t* v = lv_label_create(row);
            std::string val = mock ? mock : "--";
            if(unit && *unit) { val += " "; val += unit; }
            lv_label_set_text(v, val.c_str());
            lv_obj_add_flag(v, LV_OBJ_FLAG_FLEX_IN_NEW_TRACK);
        }
    } else {
        lv_obj_t* info = lv_label_create(col);
        lv_label_set_text(info, "(Sin campos definidos)");
    }

    lv_obj_t* back = lv_btn_create(cont);
    lv_obj_set_size(back, 120, 48);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
    lv_obj_t* bl = lv_label_create(back);
    lv_label_set_text(bl, "ATRAS");
    lv_obj_center(bl);
    lv_obj_add_event_cb(back, [](lv_event_t*){
        ui_mcr50_show_menu_generic();
    }, LV_EVENT_CLICKED, nullptr);
}

/* ======================= Render genérico ======================== */
typedef struct {
    char* id;           // id del item
    bool  has_children; // si tiene "items"
} MenuItemUD;

/* forward ya declarado arriba */
// static void ui_mcr50_show_menu_generic();

static void ui_mcr50_show_menu_generic()
{
    lv_obj_clean(lv_scr_act());

    cJSON* root = loadMenuMcr50(); if (!root) return;

    cJSON* children = nullptr;
    const char* title_txt = "Menú";
    if (!find_node_by_path(root, &children, &title_txt)) {
        ESP_LOGE(TAG, "Ruta inválida");
        cJSON_Delete(root);
        return;
    }

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Menú");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

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

            /* Representación dinámica en listas: si hay "value" en el item, muéstralo a la derecha */
            const char* val = cJSON_GetStringValue(cJSON_GetObjectItem(it, "value"));
            if (val) {
                lv_obj_set_width(btn, LV_PCT(100));
                lv_obj_t* lbl_val = lv_label_create(btn);
                lv_label_set_text(lbl_val, val);
                lv_label_set_long_mode(lbl_val, LV_LABEL_LONG_CLIP);
                lv_obj_set_width(lbl_val, LV_SIZE_CONTENT);
                lv_obj_align(lbl_val, LV_ALIGN_RIGHT_MID, -10, 0);

                /* Opcional: acotar ancho del label de título para evitar solapamientos */
                lv_obj_t* title_lbl = lv_obj_get_child(btn, 0);
                if (title_lbl) {
                    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_DOT);
                    lv_obj_set_width(title_lbl, LV_PCT(70));
                }
            }

            MenuItemUD* ud = (MenuItemUD*) std::malloc(sizeof(MenuItemUD));
            ud->id = id ? ::strdup(id) : NULL;   // <- corregido
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
                    /* Hoja: ¿es una vista "detail"? */
                    cJSON* root_local = loadMenuMcr50();
                    if (root_local) {
                        const cJSON* node = find_node_by_path_and_id(root_local, g_path, ud->id);
                        const char* view = node ? cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)node, "view")) : nullptr;
                        if (node && view && std::strcmp(view, "detail")==0) {
                            ui_render_detail_from_node(node);
                        } else {
                            ESP_LOGI(TAG, "Leaf selected: %s (sin view:\"detail\")", ud->id);
                        }
                        cJSON_Delete(root_local);
                    }
                }
            }, LV_EVENT_CLICKED, ud);

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MenuItemUD* ud = (MenuItemUD*) lv_event_get_user_data(e);
                if (ud) {
                    if (ud->id) std::free(ud->id);
                    std::free(ud);
                }
            }, LV_EVENT_DELETE, ud);
        }
    } else {
        lv_obj_t* info = lv_label_create(cont);
        lv_label_set_text(info, "Elemento sin submenús (TODO: acción específica)");
        lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);
    }

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

/* (helpers opcionales)
void ui_mcr50_build_tend_menu() { g_path = {"tend"}; ui_mcr50_show_menu_generic(); }
void ui_mcr50_build_params_menu() { g_path = {"params"}; ui_mcr50_show_menu_generic(); }
*/
