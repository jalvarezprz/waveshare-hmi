#include "ui/router/ui_router.h"
#include "esp_log.h"
#include <vector>
#include <string>
#include <cstring>

#include "lvgl.h"
#include "ui/menu/ui_menu_tree.h"
#include "ui/menu/ui_menu_loader.h"
#include "ui/view/ui_view_factory.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/layout/ui_layout_scaffold.h"

static const char* TAG = "UI_ROUTER";

static lv_obj_t* g_screen = nullptr;            // pantalla LVGL actual (reservado)
static std::string g_current_id;                // id de pantalla actual
static std::vector<std::string> g_history;      // pila simple de historial

// NAV:/screen/<id>
static bool extract_nav_screen_id(const char* action, std::string& out_id) {
    if (!action) return false;
    static constexpr const char* kPrefix = "NAV:/screen/";
    const size_t L = std::strlen(kPrefix);
    if (std::strncmp(action, kPrefix, L) != 0) return false;
    out_id.assign(action + L);
    return !out_id.empty();
}

// NAV:/back
static inline bool is_nav_back(const char* action) {
    return action && std::strcmp(action, "NAV:/back") == 0;
}

void ui_router_go_screen(const char* screen_id) {
    if (!screen_id || !*screen_id) {
        ESP_LOGE(TAG, "screen_id vacío");
        return;
    }

    // Obtener JSON (hoy, embebido)
    const char* json = ui_menu_tree_get(screen_id);
    if (!json) {
        ESP_LOGE(TAG, "Pantalla '%s' no registrada", screen_id);
        return;
    }

    // Parsear
    ScreenSpecification spec; char err[128];
    if (!loadScreen(json, spec, err, sizeof(err))) {
        ESP_LOGE(TAG, "loadScreen('%s') falló: %s", screen_id, err);
        return;
    }

    // 1) Asegura Scaffold cargado una vez
    static bool s_scaffold_ready = false;
    if (!s_scaffold_ready) {
        ui_layout_scaffold_load();
        ui_layout_scaffold_set_back_handler([](void*) { ui_router_back(); }, nullptr);
        // Arrancamos en raíz: ocultar Back
        ui_layout_scaffold_show_back(false);
        s_scaffold_ready = true;
    }

    // 2) Historial: push ANTES de actualizar header/estado
    if (!g_current_id.empty()) g_history.push_back(g_current_id);
    g_current_id = spec.id.empty() ? screen_id : spec.id;

    // 3) Header (título) y botón Back (mostrar si hay historial)
    ui_layout_scaffold_set_title(spec.title.empty() ? screen_id : spec.title.c_str());
    ui_layout_scaffold_show_back(!g_history.empty());

    // 4) Monta SOLO el contenido dentro del Scaffold
    Ui::themeReload(); // si procede
    lv_obj_t* content = ui_layout_scaffold_get_content();
    lv_obj_clean(content);
    (void)ui_view_build(content, spec);

    ESP_LOGI(TAG, "Navegando a '%s' (view=%s, elements=%u)",
             g_current_id.c_str(), spec.view.c_str(), (unsigned)spec.elements.size());
}

void ui_router_back(void) {
    if (g_history.empty()) {
        ESP_LOGW(TAG, "Historial vacío");
        // En raíz: asegúrate de que el Back no se muestre
        ui_layout_scaffold_show_back(false);
        return;
    }
    const std::string prev = g_history.back();
    g_history.pop_back();
    g_current_id.clear();  // evitar re-push en go_screen
    ui_router_go_screen(prev.c_str());
}

void ui_router_dispatch(const char* action) {

    ESP_LOGI("UI_ROUTER", "dispatch: %s", action ? action : "(null)");
    if (!action) return;

    // Soporte explícito de NAV:/back (para ButtonBack.action por defecto)
    if (is_nav_back(action)) {
        ui_router_back();
        return;
    }

    // NAV:/screen/<id>
    std::string id;
    if (extract_nav_screen_id(action, id)) {
        ui_router_go_screen(id.c_str());
        return;
    }

    // DO:/... (no navega)
    if (std::strncmp(action, "DO:/", 4) == 0) {
        ESP_LOGI(TAG, "DO acción: %s (sin implementación; permanece en pantalla)", action);
        return;
    }

    ESP_LOGW(TAG, "Acción desconocida: %s", action);
}
