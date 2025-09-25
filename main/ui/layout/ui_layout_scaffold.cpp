#include "ui/layout/ui_layout_scaffold.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"   // ← sustituye al preset ButtonBack
#include "esp_log.h"

static const char* TAG = "UI_SCAFFOLD";

// Punteros globales del Scaffold
static lv_obj_t*  s_screen  = nullptr;
static lv_obj_t*  s_header  = nullptr;
static Ui::Button* s_btnBack = nullptr;        // ← ahora es Ui::Button*
static lv_obj_t*  s_title   = nullptr;
static lv_obj_t*  s_content = nullptr;

// Back handler (inyectado desde fuera; nada de router aquí)
static ui_back_cb_t s_back_cb = nullptr;
static void*        s_back_ud = nullptr;

static void on_back_event(lv_event_t* e) {
    (void)e;
    if (s_back_cb) s_back_cb(s_back_ud);
}

static void build_once() {
    if (s_screen) return;

    auto& S = Ui::getThemeStyles();

    s_screen = lv_obj_create(nullptr);
    lv_obj_clear_flag(s_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_screen, 8, 0);

    // Layout: grid 1 columna, 2 filas (header + content)
    static lv_coord_t col[] = { LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    static lv_coord_t row[] = { LV_GRID_CONTENT, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };

    lv_obj_set_layout(s_screen, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(s_screen, col, row);
    lv_obj_set_style_pad_row(s_screen, 8, 0);

    // Header
    s_header = lv_obj_create(s_screen);
    lv_obj_set_size(s_header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_grid_cell(s_header, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_set_style_pad_all(s_header, 8, 0);
    lv_obj_clear_flag(s_header, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_flex_flow(s_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(s_header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Botón Back (Ui::Button)
    s_btnBack = new Ui::Button();
    s_btnBack->create(s_header);
    s_btnBack->setText("Atrás");             // Montserrat Body + color onPrimary
    s_btnBack->setIcon(LV_SYMBOL_LEFT);      // Icono a la izquierda por defecto
    // s_btnBack->setIcon("");               // Si tu fontIcon incluye este glifo
    // s_btnBack->setIconLeft(false);         // Si lo quieres a la derecha
    s_btnBack->setOnClick([](void*){ on_back_event(nullptr); }, nullptr);
    lv_obj_align(s_btnBack->root(), LV_ALIGN_LEFT_MID, 8, 0);

    // Título
    s_title = lv_label_create(s_header);
    lv_label_set_text(s_title, "");  // se rellena en runtime
    lv_obj_add_style(s_title, &S.labelTitle, 0);
    lv_obj_set_style_pad_left(s_title, 8, 0);

    // Content
    s_content = lv_obj_create(s_screen);
    lv_obj_set_size(s_content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_grid_cell(s_content, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_clear_flag(s_content, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(s_content, 8, 0);
    lv_obj_set_flex_flow(s_content, LV_FLEX_FLOW_COLUMN);
}

extern "C" {

void ui_layout_scaffold_load(void) {
    build_once();
    lv_scr_load(s_screen);
}

lv_obj_t* ui_layout_scaffold_get_content(void) {
    build_once();
    return s_content;
}

void ui_layout_scaffold_set_title(const char* title) {
    if (!s_title) return;
    lv_label_set_text(s_title, title ? title : "");
}

void ui_layout_scaffold_set_back_enabled(bool enabled)
{
    if (!s_btnBack || !s_btnBack->root()) return;
    s_btnBack->setEnabled(enabled);
}

void ui_layout_scaffold_set_back_handler(ui_back_cb_t cb, void* user) {
    s_back_cb = cb;
    s_back_ud = user;
    if (s_btnBack) {
        s_btnBack->setOnClick([](void*) { if (s_back_cb) s_back_cb(s_back_ud); }, nullptr);
    }
}

void ui_layout_scaffold_show_back(bool show)
{
    if (!s_btnBack || !s_btnBack->root()) return;

    if (show) {
        lv_obj_clear_flag(s_btnBack->root(), LV_OBJ_FLAG_HIDDEN);
        s_btnBack->setEnabled(true);
    } else {
        // Evita que el foco navegue a un control oculto
        s_btnBack->setEnabled(false);
        lv_obj_add_flag(s_btnBack->root(), LV_OBJ_FLAG_HIDDEN);
    }
}

} // extern "C"
