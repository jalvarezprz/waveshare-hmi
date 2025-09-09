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

static const char* TAG = "UI_ROUTER";

static lv_obj_t* g_screen = nullptr;            // pantalla LVGL actual
static std::string g_current_id;                // id de pantalla actual
static std::vector<std::string> g_history;      // pila simple de historial

static bool extract_nav_screen_id(const char* action, std::string& out_id) {
    if (!action) return false;
    static constexpr const char* kPrefix = "NAV:/screen/";
    const size_t L = std::strlen(kPrefix);
    if (std::strncmp(action, kPrefix, L) != 0) return false;
    out_id.assign(action + L);
    return !out_id.empty();
}

static void show_screen_spec(const ScreenSpecification& spec) {
    // destruir pantalla anterior
    if (g_screen) {
        lv_obj_del(g_screen);
        g_screen = nullptr;
    }

    // Crear nueva pantalla y construir vista
    g_screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(g_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(g_screen, 12, 0);

    (void)ui_view_build(g_screen, spec);

    // Cargar
    lv_scr_load(g_screen);
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

    // Historial: push si hay current
    if (!g_current_id.empty()) g_history.push_back(g_current_id);
    g_current_id = spec.id.empty() ? screen_id : spec.id;

    ESP_LOGI(TAG, "Navegando a '%s' (view=%s, elements=%u)",
             g_current_id.c_str(), spec.view.c_str(), (unsigned)spec.elements.size());

    Ui::themeReload();   // por si el tema cambió
    show_screen_spec(spec);
}

void ui_router_back(void) {
    if (g_history.empty()) {
        ESP_LOGW(TAG, "Historial vacío");
        return;
    }
    const std::string prev = g_history.back();
    g_history.pop_back();
    g_current_id.clear();  // evitar re-push en go_screen
    ui_router_go_screen(prev.c_str());
}

void ui_router_dispatch(const char* action) {
    if (!action) return;

    std::string id;
    if (extract_nav_screen_id(action, id)) {
        ui_router_go_screen(id.c_str());
        return;
    }

    if (std::strncmp(action, "DO:/", 4) == 0) {
        ESP_LOGI(TAG, "DO acción: %s (sin implementación; permanece en pantalla)", action);
        return;
    }

    ESP_LOGW(TAG, "Acción desconocida: %s", action);
}
