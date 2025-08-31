/**
 * @file ui_menu_nav.cpp
 * @brief Implementación de navegación y búsquedas sobre el árbol JSON del menú.
 * @ingroup ui_menu
 */

#include "ui_menu_nav.h"
#include <cstring>   // std::strcmp
#include "esp_log.h"

extern "C" {
#include "cJSON.h"
}

namespace Ui { namespace Menu {

static const char* TAG_NAV = "UI_MENU_NAV";

// Ruta global (local al módulo)
static std::vector<std::string> g_path;

const std::vector<std::string>& ui_menu_nav_path() { return g_path; }
void ui_menu_nav_clear() { g_path.clear(); }
void ui_menu_nav_push(const std::string& id) { g_path.push_back(id); }
bool ui_menu_nav_pop() {
    if (g_path.empty()) return false;
    g_path.pop_back();
    return true;
}

bool ui_menu_nav_is_protected_root_id(const char* id) {
    if (!id) return false;
    return std::strcmp(id,"params")==0 || std::strcmp(id,"hw")==0
        || std::strcmp(id,"ddc")==0   || std::strcmp(id,"bus")==0;
}

bool ui_menu_nav_find(cJSON* root, cJSON** out_array, const char** out_title)
{
    if (!root || !out_array || !out_title) return false;

    cJSON* node_array = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(node_array)) return false;

    const char* title = "Menú principal";
    cJSON* current = nullptr;

    if (g_path.empty()) {
        *out_array = node_array;
        *out_title = title;
        return true;
    }

    // Recorre la ruta: en cada nivel busca "id" y baja a "items"
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

const cJSON* ui_menu_nav_find_leaf(const cJSON* root, const char* leaf_id)
{
    if (!root || !leaf_id) return nullptr;
    const cJSON* cur = cJSON_GetObjectItem(root, "menu");
    if (!cJSON_IsArray(cur)) return nullptr;

    // Baja siguiendo g_path
    for (const auto& idp : g_path) {
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

    // Busca la hoja en el nivel actual
    const cJSON* it = nullptr;
    cJSON_ArrayForEach(it, cur) {
        const char* nid = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
        if(nid && std::strcmp(nid, leaf_id)==0) return it;
    }
    return nullptr;
}

}} // namespace Ui::Menu
