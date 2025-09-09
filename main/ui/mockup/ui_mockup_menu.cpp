/**
 * @file ui_mockup_menu.cpp
 * @brief Mockup dinámico: pinta una pantalla a partir de ScreenSpecification (JSON).
 *
 * - Carga SOLO la pantalla solicitada (ahora: "main") desde ui_menu_tree_get()
 * - Parseo con ui_menu_loader (schemaVersion/view/elements)
 * - Render según spec.view:
 *      - "menu_grid" → rejilla de botones (Ui::Preset::ButtonMenu)
 *
 * NOTA: no se indexa ni recorre FS. El Router se encarga de NAV:/…; aquí sólo pintamos.
 */

#include "lvgl.h"
#include "esp_log.h"
#include <cstring>
#include <cstdio>   // std::snprintf
#include <vector>
#include <string>
#include <cmath>

// Tema/controles
#include "ui/theme/ui_theme_styles.h"
#include "ui/preset/ui_preset_button.h"

// Loader + fuente de pantallas
#include "ui/menu/ui_menu_loader.h"
#include "ui/menu/ui_menu_tree.h"

#include "ui/router/ui_router.h"

static const char* TAG = "UI_MOCKUP_MENU";

// Punteros de UI
static lv_obj_t* s_screen    = nullptr;
static lv_obj_t* s_container = nullptr;   // contenedor con layout GRID
static lv_obj_t* s_title     = nullptr;

// Descriptores de grid persistentes (vida global para LVGL)
static std::vector<lv_coord_t> g_col_dsc;
static std::vector<lv_coord_t> g_row_dsc;

// API pública
extern "C" void ui_mockup_menu_load(void);
extern "C" void ui_mockup_menu_unload(void);

/* =========================================================================================
 * Helpers
 * ========================================================================================= */

static const char* strdup_cxx(const std::string& s) {
    char* p = (char*)std::malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

// Mapas sencillos de iconos “semánticos” → ids del preset (ajusta a tus iconos reales)
static const char* resolve_icon_id(const std::string& token) {
    if (token == "gear")    return "lv:settings";
    if (token == "info")    return "lv:info";
    if (token == "repeat")  return "lv:refresh";
    if (token == "trend")   return "lv:directory";   // placeholder
    if (token == "chip")    return "lv:chip";        // si no existe, el preset hará fallback
    if (token == "bus")     return "lv:list";        // placeholder
    if (token == "number")  return "lv:warning";     // placeholder
    if (token == "temp")    return "lv:charging";    // placeholder
    return token.c_str(); // deja pasar tal cual; el preset decidirá
}

static void grid_place(lv_obj_t* obj, int body_row, int col) {
    // fila 0 es el título; las celdas comienzan en fila 1
    lv_obj_set_grid_cell(obj,
                         LV_GRID_ALIGN_STRETCH, col, 1,
                         LV_GRID_ALIGN_STRETCH, 1 + body_row, 1);
}

static void build_grid_descriptors(int cols, int rows) {
    g_col_dsc.clear();
    g_row_dsc.clear();

    // columnas
    for (int c = 0; c < cols; ++c) g_col_dsc.push_back(LV_GRID_FR(1));
    g_col_dsc.push_back(LV_GRID_TEMPLATE_LAST);

    // fila 0: título (contenido)
    g_row_dsc.push_back(LV_GRID_CONTENT);
    // filas de cuerpo
    for (int r = 0; r < rows; ++r) g_row_dsc.push_back(LV_GRID_FR(1));
    g_row_dsc.push_back(LV_GRID_TEMPLATE_LAST);
}

/* =========================================================================================
 * Builders de vistas
 * ========================================================================================= */

static lv_obj_t* build_menu_grid(lv_obj_t* parent, const ScreenSpecification& spec) {
    auto& S = Ui::getThemeStyles();

    // Parámetros del grid (ajusta si quieres tokens/layout desde spec.layout)
    const int COLS = 3;
    const lv_coord_t GAP = 16;
    const lv_coord_t OUTER_PAD = 16;

    const int n = static_cast<int>(spec.elements.size());
    const int body_rows = n > 0 ? ( (n + COLS - 1) / COLS ) : 1;

    // Contenedor principal (layout grid)
    s_container = lv_obj_create(parent);
    lv_obj_set_size(s_container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(s_container);
    lv_obj_set_layout(s_container, LV_LAYOUT_GRID);

    lv_obj_clear_flag(s_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_container, OUTER_PAD, 0);
    lv_obj_set_style_pad_column(s_container, GAP, 0);
    lv_obj_set_style_pad_row   (s_container, GAP, 0);
    lv_obj_set_style_bg_opa(s_container, LV_OPA_10, 0);
    lv_obj_set_style_border_width(s_container, 1, 0);
    lv_obj_set_style_border_color(s_container, lv_color_hex(0x7BAAF7), 0);

    // Descriptores de grid persistentes
    build_grid_descriptors(COLS, body_rows);
    lv_obj_set_grid_dsc_array(s_container, g_col_dsc.data(), g_row_dsc.data());

    // Título (fila 0, span todas las columnas)
    s_title = lv_label_create(s_container);
    lv_label_set_text(s_title, spec.title.empty() ? "Pantalla" : spec.title.c_str());
    lv_obj_set_grid_cell(s_title, LV_GRID_ALIGN_START, 0, COLS,
                                   LV_GRID_ALIGN_START, 0, 1);
    lv_obj_add_style(s_title, &S.labelTitle, 0);
    lv_obj_set_style_text_letter_space(s_title, 1, 0);
    lv_obj_set_style_pad_bottom(s_title, 4, 0);

    // Elementos → botones
    for (int i = 0; i < n; ++i) {
        const auto& el = spec.elements[i];
        const int r = i / COLS;
        const int c = i % COLS;

        Ui::Preset::ButtonMenu::Props p{};
        p.text    = strdup_cxx(el.title.empty() ? el.id : el.title);
        p.iconId  = strdup_cxx(resolve_icon_id(el.icon.empty() ? std::string("info") : el.icon));
        p.variant = Ui::Preset::ButtonMenu::Variant::Primary;

        auto act = el.action;  // copia por valor: vida del lambda asegurada
        p.callbacks.onClick = [act](Ui::Component::Button::Handle&, void*) {
            if (!act.empty()) {
                ui_router_dispatch(act.c_str());   // "NAV:/..." o "DO:/..."
            } else {
                ESP_LOGW(TAG, "Elemento sin action");
            }
        };

        auto H = Ui::Preset::ButtonMenu::create(s_container, S, p);
        grid_place(H.base.root, r, c);
    }

    return s_container;
}

/* =========================================================================================
 * Construcción de la pantalla (según view)
 * ========================================================================================= */

static void build_ui_from_spec(const ScreenSpecification& spec)
{
    // Screen raíz
    s_screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_screen, 12, 0);

    // Selección por tipo de vista
    if (spec.view == "menu_grid") {
        (void)build_menu_grid(s_screen, spec);
    } else if (spec.view == "menu_list") {
        // TODO: implementar lista; por ahora, fallback:
        s_container = lv_obj_create(s_screen);
        lv_obj_set_size(s_container, LV_PCT(100), LV_PCT(100));
        lv_obj_center(s_container);
        lv_obj_set_flex_flow(s_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(s_container, 16, 0);

        lv_obj_t* lbl = lv_label_create(s_container);
        lv_label_set_text_fmt(lbl, "Vista '%s' aún no implementada", spec.view.c_str());
    } else {
        // Fallback de view desconocida
        s_container = lv_obj_create(s_screen);
        lv_obj_set_size(s_container, LV_PCT(100), LV_PCT(100));
        lv_obj_center(s_container);
        lv_obj_set_flex_flow(s_container, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_all(s_container, 16, 0);

        lv_obj_t* lbl = lv_label_create(s_container);
        lv_label_set_text_fmt(lbl, "view desconocido: %s", spec.view.c_str());
    }

    // Cargar pantalla
    lv_scr_load(s_screen);
}

/* =========================================================================================
 * API pública
 * ========================================================================================= */

void ui_mockup_menu_load(void)
{
    // Refrescar tema
    Ui::themeReload();

    // Destruir pantalla previa si existe
    if (s_screen) {
        lv_obj_del(s_screen);
        s_screen = nullptr;
        s_container = nullptr;
        s_title = nullptr;
        ESP_LOGW(TAG, "Pantalla previa destruida; reconstruyendo.");
    }

    // 1) Obtener SOLO el JSON de la pantalla "main"
    const char* json = ui_menu_tree_get(UI_SCREEN_ID_MAIN);
    if (!json) {
        ESP_LOGE(TAG, "No se encontró JSON para 'main'");
        // Fallback mínimo para no dejar la pantalla en negro
        s_screen = lv_obj_create(nullptr);
        lv_scr_load(s_screen);
        return;
    }

    // 2) Parsear a ScreenSpecification
    ScreenSpecification spec; char err[128];
    if (!loadScreen(json, spec, err, sizeof(err))) {
        ESP_LOGE(TAG, "loadScreen(main) falló: %s", err);
        // Fallback mínimo
        s_screen = lv_obj_create(nullptr);
        lv_scr_load(s_screen);
        return;
    }

    ESP_LOGI(TAG, "Pintando '%s' (view=%s, elements=%u)",
             spec.id.c_str(), spec.view.c_str(), (unsigned)spec.elements.size());

    // 3) Construir UI según spec.view
    build_ui_from_spec(spec);
}

void ui_mockup_menu_unload(void)
{
    if (!s_screen) return;
    ESP_LOGI(TAG, "Descargando mockup.");
    lv_obj_del(s_screen);
    s_screen    = nullptr;
    s_container = nullptr;
    s_title     = nullptr;
}
