/**
 * @file ui_mockup_menu.cpp
 * @brief Mockup 4x4 (icono + texto por botón) usando Ui::Preset::ButtonMenu.
 *
 * - Grid 4x4 con gaps homogéneos
 * - Cada celda: MenuButton (icono + etiqueta apilados en columna)
 * - Primera fila de demo: variantes Destructive / Success / Warning
 */

#include "lvgl.h"
#include "esp_log.h"
#include <cstring>
#include <cstdio>   // std::snprintf

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"
#include "ui/preset/ui_preset_button.h"

static const char* TAG = "UI_MOCKUP_MENU_GRID_4x4";

// Punteros de UI
static lv_obj_t* s_screen    = nullptr;
static lv_obj_t* s_container = nullptr;   // contenedor con layout GRID
static lv_obj_t* s_title     = nullptr;

// Prototipos
extern "C" void ui_mockup_menu_load(void);
extern "C" void ui_mockup_menu_unload(void);

// Prototipo de la fila de demo de variantes
static void add_variant_demo_row(lv_obj_t* parent);

// -----------------------------------------------------------------------------
// Helpers de creación de celdas (Ítem 1, Ítem 2, genérico)
// Colocar este bloque antes de build_ui()
// -----------------------------------------------------------------------------
namespace {

static void place_in_grid(lv_obj_t* root, int r, int c) {
    lv_obj_set_grid_cell(root,
                         LV_GRID_ALIGN_STRETCH, c, 1,
                         LV_GRID_ALIGN_STRETCH, 2 + r, 1);
}

static void create_item1(lv_obj_t* container, int r, int c, int idx) {
    char caption[24];
    std::snprintf(caption, sizeof(caption), "Ítem %d", idx);

    Ui::Preset::ButtonMenu::Props p{};
    p.text    = strdup(caption);                  // vida útil > stack
    p.iconId  = "lv:settings";                    // ID semántico (el preset lo traduce)
    p.variant = Ui::Preset::ButtonMenu::Variant::Primary;

    // Callbacks (se propagan al componente base dentro del preset)
    const char* labelCopy = p.text;
    p.callbacks.onClick = [labelCopy](Ui::Component::Button::Handle&, void*) {
        ESP_LOGI("TEST", "CLICK en botón de prueba: '%s'", labelCopy);
    };
    p.callbacks.onLong  = [labelCopy](Ui::Component::Button::Handle&, void*) {
        ESP_LOGI("TEST", "LONG PRESS en botón de prueba: '%s'", labelCopy);
    };
    // Nota: NO fijamos p.action para que prevalezcan los callbacks.

    auto H = Ui::Preset::ButtonMenu::create(container, Ui::getThemeStyles(), p);
    place_in_grid(H.base.root, r, c);
}


static void create_item2(lv_obj_t* container, int r, int c, int idx) {
    char caption[24];
    std::snprintf(caption, sizeof(caption), "Ítem %d", idx);

    // Ítem 2: preset ButtonMenu con navegación por Router (action)
    Ui::Preset::ButtonMenu::Props p{};
    p.text    = strdup(caption);
    p.iconId  = "lv:settings";
    p.variant = Ui::Preset::ButtonMenu::Variant::Primary;
    p.action  = strdup("NAV:/demo");

    auto H = Ui::Preset::ButtonMenu::create(container, Ui::getThemeStyles(), p);
    place_in_grid(H.base.root, r, c);
}

static void create_item_generic(lv_obj_t* container, int r, int c, int idx) {
    char caption[24];
    std::snprintf(caption, sizeof(caption), "Ítem %d", idx);

    Ui::Preset::ButtonMenu::Props p{};
    p.text    = strdup(caption);
    p.iconId  = "lv:settings";
    p.variant = Ui::Preset::ButtonMenu::Variant::Primary;

    char actionBuf[32];
    std::snprintf(actionBuf, sizeof(actionBuf), "NAV:/item/%d", idx);
    p.action = strdup(actionBuf);

    auto H = Ui::Preset::ButtonMenu::create(container, Ui::getThemeStyles(), p);
    place_in_grid(H.base.root, r, c);
}

} // namespace


// -----------------------------------------------------------------------------
// Construcción de la UI
// -----------------------------------------------------------------------------
static void build_ui()
{
    // Parámetros del grid
    constexpr int COLS = 4;
    constexpr int ROWS = 4;
    const lv_coord_t GAP = 16;
    const lv_coord_t OUTER_PAD = 16;

    // Screen
    s_screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_screen, 12, 0);

    // Contenedor principal (layout grid)
    s_container = lv_obj_create(s_screen);
    lv_obj_set_size(s_container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(s_container);
    lv_obj_set_layout(s_container, LV_LAYOUT_GRID);

    // Padding externo y ligera visualización (mockup)
    lv_obj_set_style_pad_all(s_container, OUTER_PAD, 0);
    lv_obj_set_style_bg_opa(s_container, LV_OPA_10, 0);
    lv_obj_set_style_border_width(s_container, 1, 0);
    lv_obj_set_style_border_color(s_container, lv_color_hex(0x7BAAF7), 0);

    // Gaps del grid
    lv_obj_set_style_pad_column(s_container, GAP, 0);
    lv_obj_set_style_pad_row   (s_container, GAP, 0);

    // Descripción columnas/filas
    static lv_coord_t col_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t row_dsc[] = {
        LV_GRID_CONTENT,          // fila 0: título
        LV_GRID_CONTENT,          // fila 1: fila de demo
        LV_GRID_FR(1),            // filas 2..5
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(s_container, col_dsc, row_dsc);

    // ---------------- Título (fila 0, span 4 columnas) ----------------
    s_title = lv_label_create(s_container);
    lv_label_set_text(s_title, "Menú principal (Mockup 4×4)");
    lv_obj_set_grid_cell(s_title,
                         LV_GRID_ALIGN_START, 0, 4,
                         LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_style_text_letter_space(s_title, 1, 0);
    lv_obj_set_style_pad_bottom(s_title, 4, 0);
    lv_obj_set_style_text_color(s_title, lv_color_hex(0x0000FF), 0);

    lv_obj_add_style(s_title, &Ui::getThemeStyles().labelTitle, 0);


    // ---------------- Fila de demo (fila 1) ----------------
    add_variant_demo_row(s_container);

    // ---------------- Celdas 4x4 (filas 2..5, cols 0..3) --------------

    int idx = 0;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            ++idx;
            if (idx == 1) { create_item1(s_container, r, c, idx); continue; }
            if (idx == 2) { create_item2(s_container, r, c, idx); continue; }
            create_item_generic(s_container, r, c, idx);
        }
    }

    // Cargar pantalla
    lv_scr_load(s_screen);
}

// API pública
void ui_mockup_menu_load(void)
{
    // 1) Forzar que el snapshot (tokens->styles) refleje los cambios recientes (p. ej., Primary)
    Ui::themeReload();

    // 2) Si la pantalla ya existía, destruirla para que al reconstruir se apliquen los estilos nuevos
    if (s_screen) {
        lv_obj_del(s_screen);
        s_screen = nullptr;
        ESP_LOGW(TAG, "ui_mockup_menu_load(): pantalla previa destruida; reconstruyendo con tema recargado.");
    }

    ESP_LOGI(TAG, "Cargando mockup 4x4 (MenuButton)...");
    build_ui();
}

void ui_mockup_menu_unload(void)
{
    if (!s_screen) return;
    ESP_LOGI(TAG, "Descargando mockup 4x4.");
    lv_obj_del(s_screen);
    s_screen    = nullptr;
    s_container = nullptr;
    s_title     = nullptr;
}

// -----------------------------------------------------------------------------
// Fila de demo con los 3 botones de nuevas variantes
// -----------------------------------------------------------------------------
static void add_variant_demo_row(lv_obj_t* parent)
{
    auto& S = Ui::getThemeStyles();

    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(row, S.tokens.btnIconGap, 0);

    lv_obj_set_grid_cell(row,
                         LV_GRID_ALIGN_STRETCH, 0, 4,
                         LV_GRID_ALIGN_START,   1, 1);

    // Botón Destructive
    {
        Ui::Component::Button::Props p;
        p.text    = "Eliminar";
        p.icon    = LV_SYMBOL_TRASH;
        p.variant = Ui::Component::Button::Variant::Destructive;
        (void)Ui::Component::Button::create(row, S, p);
    }
    // Botón Success
    {
        Ui::Component::Button::Props p;
        p.text    = "Aceptar";
        p.icon    = LV_SYMBOL_OK;
        p.variant = Ui::Component::Button::Variant::Success;
        (void)Ui::Component::Button::create(row, S, p);
    }
    // Botón Warning
    {
        Ui::Component::Button::Props p;
        p.text    = "Atención";
        p.icon    = LV_SYMBOL_WARNING;
        p.variant = Ui::Component::Button::Variant::Warning;
        (void)Ui::Component::Button::create(row, S, p);
    }
}
