/**
 * @file ui_mockup_menu.cpp
 * @brief Mockup 4x4 (icono + texto por botón) con adaptador para migrar luego a Ui::Button.
 *
 * - Grid 4x4 con gaps homogéneos
 * - Cada celda: botón LVGL con icono + etiqueta apilados en columna
 * - Color texto: azul (mockup)
 * - Punto único de integración futura: create_cell_button(...)
 */

#include "lvgl.h"
#include "esp_log.h"
#include <cstring>
#include <cstdio>   // std::snprintf

#include "ui/theme/ui_theme_styles.h"          // estilos/base
#include "ui/component/ui_component_button.h"  // ← AÑADIDO: componente Button
#include "ui/preset/ui_preset_button.h"

static const char* TAG = "UI_MOCKUP_MENU_GRID_4x4";

// Punteros de UI
static lv_obj_t* s_screen    = nullptr;
static lv_obj_t* s_container = nullptr;   // contenedor con layout GRID
static lv_obj_t* s_title     = nullptr;

// Prototipos
extern "C" void ui_mockup_menu_load(void);
extern "C" void ui_mockup_menu_unload(void);

// Prototipo de la fila de demo de variantes (debe existir antes de build_ui)
static void add_variant_demo_row(lv_obj_t* parent);

// Helpers mínimos
static void set_pad(lv_obj_t* obj, lv_coord_t pad)         { lv_obj_set_style_pad_all(obj, pad, 0); }
static void set_radius(lv_obj_t* obj, lv_coord_t r)        { lv_obj_set_style_radius(obj, r, 0); }
static void set_border(lv_obj_t* obj, lv_coord_t w)        { lv_obj_set_style_border_width(obj, w, 0); }
static void set_bg_opa(lv_obj_t* obj, lv_opa_t opa)        { lv_obj_set_style_bg_opa(obj, opa, 0); }
static void set_border_color(lv_obj_t* obj, lv_color_t c)  { lv_obj_set_style_border_color(obj, c, 0); }

// Callback click
static void on_button_clicked(lv_event_t* e)
{
    if (e->code != LV_EVENT_CLICKED) return;
    const char* tag = static_cast<const char*>(lv_event_get_user_data(e));
    ESP_LOGI(TAG, "Pulsado: %s", tag ? tag : "(sin etiqueta)");
}

// -----------------------------------------------------------------------------
// ADAPTADOR ÚNICO: ahora crea un lv_btn con icono+texto; más adelante,
// cuando tengamos el nombre correcto del componente, reemplazamos aquí
// por Ui::Button::create(...) sin tocar el resto del mockup.
// Devuelve el "root" del botón colocado dentro del 'parent'.
// -----------------------------------------------------------------------------
static lv_obj_t* create_cell_button(lv_obj_t* parent, const char* caption, lv_event_cb_t cb, void* user)
{
    // Botón-celda
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, LV_PCT(100), LV_PCT(100));
    set_radius(btn, 10);
    set_border(btn, 1);
    set_border_color(btn, lv_color_hex(0x5C6BC0));
    set_bg_opa(btn, LV_OPA_20);
    lv_obj_set_style_min_height(btn, 64, 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_40, LV_STATE_PRESSED);

    // Contenido interno con flex (columna): icono arriba, texto abajo
    lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(btn,
                          LV_FLEX_ALIGN_CENTER,   // main
                          LV_FLEX_ALIGN_CENTER,   // cross
                          LV_FLEX_ALIGN_CENTER);  // track cross
    lv_obj_set_style_pad_row(btn, 6, 0); // espacio entre icono y texto

    // Icono (mismo para todos por ahora)
    lv_obj_t* lbl_icon = lv_label_create(btn);
    lv_label_set_text(lbl_icon, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(lbl_icon, lv_color_hex(0x0000FF), 0);

    // Texto
    lv_obj_t* lbl_text = lv_label_create(btn);
    lv_label_set_text(lbl_text, caption);
    lv_obj_set_style_text_color(lbl_text, lv_color_hex(0x0000FF), 0);

    // Callback
    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, user);
    return btn;
}

// -----------------------------------------------------------------------------
// Construcción de la UI
// -----------------------------------------------------------------------------
static void build_ui()
{
    // Parámetros del grid
    constexpr int COLS = 4;
    constexpr int ROWS = 4;
    const lv_coord_t GAP = 16;        // separación prudencial
    const lv_coord_t OUTER_PAD = 16;  // margen interno

    // Screen
    s_screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    set_pad(s_screen, 12);

    // Contenedor principal (layout grid)
    s_container = lv_obj_create(s_screen);
    lv_obj_set_size(s_container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(s_container);
    lv_obj_set_layout(s_container, LV_LAYOUT_GRID);

    // Padding externo y ligera visualización (mockup)
    set_pad(s_container, OUTER_PAD);
    set_bg_opa(s_container, LV_OPA_10);
    set_border(s_container, 1);
    set_border_color(s_container, lv_color_hex(0x7BAAF7));

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
        LV_GRID_CONTENT,          // fila 1: fila de demo (Destructive/Success/Warning)
        LV_GRID_FR(1),            // fila 2
        LV_GRID_FR(1),            // fila 3
        LV_GRID_FR(1),            // fila 4
        LV_GRID_FR(1),            // fila 5
        LV_GRID_TEMPLATE_LAST
    };
    lv_obj_set_grid_dsc_array(s_container, col_dsc, row_dsc);

    // ---------------- Título (fila 0, span 4 columnas) ----------------
    s_title = lv_label_create(s_container);
    lv_label_set_text(s_title, "Menú principal (Mockup 4×4)");
    lv_obj_set_grid_cell(s_title,
                         LV_GRID_ALIGN_START, 0, 4,   // col 0, span 4
                         LV_GRID_ALIGN_START, 0, 1);  // fila 0
    lv_obj_set_style_text_letter_space(s_title, 1, 0);
    lv_obj_set_style_pad_bottom(s_title, 4, 0);
    lv_obj_set_style_text_color(s_title, lv_color_hex(0x0000FF), 0);

    // ---------------- Fila de demo (fila 1, span 4 columnas) ----------
    add_variant_demo_row(s_container);

    // ---------------- Celdas 4x4 (filas 2..5, cols 0..3) --------------
    // ---------------- Celdas 4x4 (filas 2..5, cols 0..3) --------------
    int idx = 0;
    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            ++idx;

            // ←—— Usa el preset REAL solo en la primera celda (idx==1) para validar
            // ←—— Usa el preset REAL solo en la primera celda (idx==1) para validar
            if (idx == 1) {
                Ui::Preset::ButtonMenu::Props p;
                p.text    = "Parámetros";
                p.iconId  = "lv:settings";
                p.variant = Ui::Preset::ButtonMenu::Variant::Primary;

                // Callbacks para ver logs
                p.callbacks.onClick = [](Ui::Component::Button::Handle&, void*) {
                    ESP_LOGI(TAG, "MenuButton: CLICK en 'Parámetros'");
                };
                p.callbacks.onLong = [](Ui::Component::Button::Handle&, void*) {
                    ESP_LOGI(TAG, "MenuButton: LONG PRESS en 'Parámetros'");
                };
                p.callbacks.onToggle = [](Ui::Component::Button::Handle&, bool checked, void*) {
                    ESP_LOGI(TAG, "MenuButton: TOGGLE -> %s", checked ? "ON" : "OFF");
                };

                auto H = Ui::Preset::ButtonMenu::create(s_container, Ui::getThemeStyles(), p);
                lv_obj_t* root = H.base.root;

                // Ubicar en grid: fila base ahora es 2 (0=título, 1=fila demo)
                lv_obj_set_grid_cell(root,
                                    LV_GRID_ALIGN_STRETCH, c, 1,      // columna c
                                    LV_GRID_ALIGN_STRETCH, 2 + r, 1); // fila 2+r
                continue; // ← importante
}


            // Resto de celdas siguen con el botón “mock”
            char caption[24];
            std::snprintf(caption, sizeof(caption), "Item %d", idx);

            lv_obj_t* root = create_cell_button(s_container, caption, on_button_clicked, (void*)strdup(caption));

            lv_obj_set_grid_cell(root,
                                LV_GRID_ALIGN_STRETCH, c, 1,
                                LV_GRID_ALIGN_STRETCH, 2 + r, 1);
        }
    }

        // Cargar pantalla
        lv_scr_load(s_screen);
    }

// API pública
void ui_mockup_menu_load(void)
{
    if (s_screen) {
        lv_scr_load(s_screen);
        ESP_LOGW(TAG, "ui_mockup_menu_load(): ya creada; recargando.");
        return;
    }
    ESP_LOGI(TAG, "Cargando mockup 4x4 (icono+texto por botón)...");
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
// Fila de demo con los 3 botones de nuevas variantes (Destructive/Success/Warning)
// -----------------------------------------------------------------------------
static void add_variant_demo_row(lv_obj_t* parent)
{
    auto& S = Ui::getThemeStyles();

    // Contenedor horizontal para agrupar los 3 botones
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(row, S.tokens.btnIconGap, 0);

    // Colocación en la grid: fila 1 (debajo del título), span 4 columnas
    lv_obj_set_grid_cell(
        row,
        LV_GRID_ALIGN_STRETCH, 0, 4,   // col 0, span 4
        LV_GRID_ALIGN_START,   1, 1    // fila 1
    );

    // Botón Destructive
    {
        Ui::Component::Button::Props p;
        p.text    = "Eliminar";
        p.icon    = LV_SYMBOL_TRASH;
        p.variant = Ui::Component::Button::Variant::Destructive;
        auto h = Ui::Component::Button::create(row, S, p);
        (void)h;
    }

    // Botón Success
    {
        Ui::Component::Button::Props p;
        p.text    = "Aceptar";
        p.icon    = LV_SYMBOL_OK;
        p.variant = Ui::Component::Button::Variant::Success;
        auto h = Ui::Component::Button::create(row, S, p);
        (void)h;
    }

    // Botón Warning
    {
        Ui::Component::Button::Props p;
        p.text    = "Atención";
        p.icon    = LV_SYMBOL_WARNING;
        p.variant = Ui::Component::Button::Variant::Warning;
        auto h = Ui::Component::Button::create(row, S, p);
        (void)h;
    }
}
