#include "ui/view/ui_view_factory.h"
#include "esp_log.h"
#include <vector>
#include <string>
#include <cstring>   // memcpy
#include <cstdlib>   // malloc

// Tema/controles
#include "ui/theme/ui_theme_styles.h"
#include "ui/preset/ui_preset_button.h"

// Router: el preset invoca ui_router_dispatch si p.action está definido
#include "ui/router/ui_router.h"

static const char* TAG = "UI_VIEW_FACTORY";

// dup de std::string → char* (lifetime heap). Devuelve const char* por comodidad.
static const char* strdup_cxx(const std::string& s) {
    char* p = (char*)std::malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

// Mapea tokens semánticos → id de icono del preset.
static std::string resolve_icon_id(const std::string& token) {
    if (token == "gear")    return "lv:settings";
    if (token == "info")    return "lv:info";
    if (token == "repeat")  return "lv:refresh";
    if (token == "trend")   return "lv:directory";
    if (token == "chip")    return "lv:chip";
    if (token == "bus")     return "lv:list";
    if (token == "number")  return "lv:warning";
    if (token == "temp")    return "lv:charging";
    return token; // deja pasar tal cual
}

// --- helpers muy pequeños ---
static lv_obj_t* make_row(lv_obj_t* parent, int pad_col) {
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_remove_style_all(row);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_pad_column(row, pad_col, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, 0);
    lv_obj_set_width(row, LV_PCT(100));
    return row;
}

static lv_obj_t* make_label(lv_obj_t* parent, const char* txt, Ui::UiThemeStyles& s, bool bold=false) {
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, txt ? txt : "");
    lv_obj_set_style_text_color(l, s.tokens.colorSurface, 0);
    lv_obj_set_style_text_font(l, bold ? s.tokens.fontTitle : s.tokens.fontBody, 0);
    return l;
}

static void enable_event_bubble_recursive(lv_obj_t* obj) {
    if (!obj) return;
    uint32_t cnt = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < cnt; ++i) {
        lv_obj_t* ch = lv_obj_get_child(obj, i);
        lv_obj_add_flag(ch, LV_OBJ_FLAG_EVENT_BUBBLE);
        enable_event_bubble_recursive(ch);
    }
}

// Ata NAV:/… / DO:/… al objeto LVGL. Copia la acción en heap y la libera en DELETE.
static void bind_action_click(lv_obj_t* obj, const std::string& act) {
    if (!obj || act.empty()) return;

    char* payload = (char*)lv_mem_alloc(act.size() + 1);
    if (!payload) return;
    std::memcpy(payload, act.c_str(), act.size() + 1);

    // Asegura que los clics en los hijos burbujeen hasta este objeto
    enable_event_bubble_recursive(obj);

    lv_obj_add_event_cb(obj, [](lv_event_t* e){
        const char* s = (const char*)lv_event_get_user_data(e);
        if (!s) return;

        switch (lv_event_get_code(e)) {
            case LV_EVENT_CLICKED:
                ESP_LOGI("VIEW", "CLICK → dispatch: %s", s);
                ui_router_dispatch(s);
                break;
            case LV_EVENT_DELETE:
                lv_mem_free((void*)s);
                break;
            default:
                break;
        }
    }, LV_EVENT_ALL, payload);
}

/* ========================= VISTA: sw_temp_panel ========================= */

// Contexto del panel para gestionar vida útil de labels + timer
struct PanelCtx {
    lv_obj_t* temp_val[9];   // hasta 9 lecturas (3x3)
    int temp_count;          // cuántas hay realmente
    lv_timer_t* timer;
};

// Limpieza segura al destruir el root de la vista
static void panel_ctx_on_delete(lv_event_t* e) {
    PanelCtx* ctx = (PanelCtx*)lv_event_get_user_data(e);
    if (!ctx) return;
    if (ctx->timer) {
        lv_timer_del(ctx->timer);
        ctx->timer = nullptr;
    }
    for (int i = 0; i < 9; ++i) ctx->temp_val[i] = nullptr;
    lv_mem_free(ctx);
}

// Timer: actualiza lecturas si los objetos siguen vivos
static void panel_timer_cb(lv_timer_t* t) {
    PanelCtx* ctx = (PanelCtx*)t->user_data;   // compat LVGL 8.x
    if (!ctx) return;

    static float base = 20.0f;
    base += 0.1f; if (base > 25.0f) base = 20.0f;

    for (int i = 0; i < ctx->temp_count; ++i) {
        lv_obj_t* lab = ctx->temp_val[i];
        if (!lab) continue;
        if (!lv_obj_is_valid(lab)) { ctx->temp_val[i] = nullptr; continue; }

        char buf[32];
        float val = base + (i % 3) * 0.5f + (i / 3) * 0.2f; // ligera variación por celda
        std::snprintf(buf, sizeof(buf), "%.1f °C", val);
        lv_label_set_text(lab, buf);
    }
}

// builder: 3 columnas, 4 filas (fila 0 con 3 switches; filas 1..3 con 3 temps c/u)
static lv_obj_t* build_sw_temp_panel(lv_obj_t* parent, const ScreenSpecification& spec)
{
    Ui::UiThemeStyles& s = Ui::getThemeStyles();

    // Root propio del panel (hijo de parent)
    lv_obj_t* root = lv_obj_create(parent);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    // Valores locales
    static constexpr lv_coord_t OUTER_PAD = 16;
    static constexpr lv_coord_t GRID_GAP  = 12;
    static constexpr lv_coord_t ROW_GAP   = 8;  // separación interna en cada celda

    // Grid: 3 columnas (todas usadas), 4 filas
    static const lv_coord_t col_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST
    };

    static const lv_coord_t row_dsc[] = {
        LV_GRID_FR(1),  // fila 0: switches
        LV_GRID_FR(1),  // fila 1: temperaturas
        LV_GRID_FR(1),  // fila 2: temperaturas
        LV_GRID_FR(1),  // fila 3: temperaturas
        LV_GRID_TEMPLATE_LAST
    };

    lv_obj_set_layout(root, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(root, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(root, OUTER_PAD, 0);
    lv_obj_set_style_pad_row(root, GRID_GAP, 0);
    lv_obj_set_style_pad_column(root, GRID_GAP, 0);

    // Contexto del panel y hook de borrado en root
    PanelCtx* ctx = (PanelCtx*)lv_mem_alloc(sizeof(PanelCtx));
    if (ctx) {
        ctx->timer = nullptr;
        ctx->temp_count = 0;
        for (int i = 0; i < 9; ++i) ctx->temp_val[i] = nullptr;
        lv_obj_add_event_cb(root, panel_ctx_on_delete, LV_EVENT_DELETE, ctx);
    }

    // Recolectar elementos del JSON: 1) switches sw*, 2) temps t*
    std::vector<const ElementSpecification*> sws;
    std::vector<const ElementSpecification*> temps;
    sws.reserve(3);
    temps.reserve(9);

    for (const auto& e : spec.elements) {
        if (!e.visible) continue;
        if (e.id.rfind("sw", 0) == 0) {
            if (sws.size() < 3) sws.push_back(&e);
        } else if (e.id.rfind("t", 0) == 0) {
            if (temps.size() < 9) temps.push_back(&e);
        }
    }

    ESP_LOGI(TAG, "sw_temp_panel: temps=%d, sws=%d", (int)temps.size(), (int)sws.size());

    // --- Fila 0: hasta 3 switches, columnas 0..2 ---
    for (int c = 0; c < (int)sws.size(); ++c) {
        const auto* e = sws[c];

        // Celda contenedora en (col=c, row=0)
        lv_obj_t* cell = make_row(root, ROW_GAP);
        lv_obj_set_grid_cell(cell,
            LV_GRID_ALIGN_STRETCH, c, 1,
            LV_GRID_ALIGN_STRETCH, 0, 1);

        // [Label] [Switch]
        lv_obj_t* l = make_label(cell, e->title.c_str(), s, false);
        // Alto contraste para texto
        lv_obj_set_style_text_color(l, lv_color_black(), 0);

        lv_obj_t* sw = lv_switch_create(cell);
        if (!e->enabled) lv_obj_add_state(sw, LV_STATE_DISABLED);
        lv_obj_set_size(sw, 60, 30);
        lv_obj_set_style_radius(sw, 15, 0);
        lv_obj_set_flex_grow(sw, 0);
        // NO aplicar s.tokens.minTouch aquí


        // Evento → router
        if (!e->action.empty()) {
            char* act = (char*)lv_mem_alloc(e->action.size() + 1);
            if (act) {
                std::strcpy(act, e->action.c_str());
                lv_obj_add_event_cb(sw, [](lv_event_t* ev){
                    const char* act = (const char*)lv_event_get_user_data(ev);
                    ui_router_dispatch(act);
                }, LV_EVENT_VALUE_CHANGED, act);
            }
        }

        // Grow para empujar switch a la derecha
        lv_obj_set_flex_grow(l, 1);
    }

    // --- Filas 1..3: indicadores de temperatura, 3 por fila (total hasta 9) ---
    for (int i = 0; i < (int)temps.size(); ++i) {
        const auto* e = temps[i];
        const int row = 1 + i / 3; // filas 1..3
        const int col = i % 3;     // columnas 0..2

        lv_obj_t* cell = make_row(root, ROW_GAP);
        lv_obj_set_grid_cell(cell,
            LV_GRID_ALIGN_STRETCH, col, 1,
            LV_GRID_ALIGN_STRETCH, row, 1);

        (void)make_label(cell, e->title.c_str(), s, false);

        lv_obj_t* v = make_label(cell, "--.- °C", s, true);
        // Valor grande y con alto contraste
        lv_obj_set_style_text_font(v, s.tokens.fontTitle, 0);
        lv_obj_set_style_text_color(v, lv_color_black(), 0);
        lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_width(v, LV_PCT(40));

        if (ctx && ctx->temp_count < 9) {
            ctx->temp_val[ctx->temp_count++] = v;
        }
    }

    // Timer DEMO para actualizar lecturas (reemplaza con sensores reales)
    if (ctx && ctx->temp_count > 0) {
        ctx->timer = lv_timer_create(panel_timer_cb, 1000, ctx);
    }

    return root;
}

/* -------------------- menu_grid -------------------- */
static lv_obj_t* build_menu_grid(lv_obj_t* parent, const ScreenSpecification& spec) {
    auto& S = Ui::getThemeStyles();

    const int COLS = 3;
    const lv_coord_t GAP = 16;
    const lv_coord_t OUTER_PAD = 16;

    const int n = (int)spec.elements.size();
    const int body_rows = n > 0 ? ((n + COLS - 1) / COLS) : 1;

    // Contenedor
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, OUTER_PAD, 0);
    lv_obj_set_style_pad_column(cont, GAP, 0);
    lv_obj_set_style_pad_row(cont, GAP, 0);

    static std::vector<lv_coord_t> col_dsc, row_dsc;
    col_dsc.clear(); row_dsc.clear();
    for (int c = 0; c < COLS; ++c) col_dsc.push_back(LV_GRID_FR(1));
    col_dsc.push_back(LV_GRID_TEMPLATE_LAST);
    row_dsc.push_back(LV_GRID_CONTENT); // título
    for (int r = 0; r < body_rows; ++r) row_dsc.push_back(LV_GRID_FR(1));
    row_dsc.push_back(LV_GRID_TEMPLATE_LAST);
    lv_obj_set_grid_dsc_array(cont, col_dsc.data(), row_dsc.data());

    // Título
    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, spec.title.empty() ? "Pantalla" : spec.title.c_str());
    lv_obj_set_grid_cell(title, LV_GRID_ALIGN_START, 0, COLS, LV_GRID_ALIGN_START, 0, 1);
    lv_obj_add_style(title, &S.labelTitle, 0);

    // Tiles
    for (int i = 0; i < n; ++i) {
        const auto& el = spec.elements[i];
        const int r = i / COLS;
        const int c = i % COLS;

        Ui::Preset::ButtonMenu::Props p{};
        p.text    = strdup_cxx(el.title.empty() ? el.id : el.title);
        p.iconId  = strdup_cxx(resolve_icon_id(el.icon.empty() ? std::string("info") : el.icon));
        p.variant = Ui::Preset::ButtonMenu::Variant::Primary;

        if (!el.action.empty()) p.action = strdup_cxx(el.action);

        auto H = Ui::Preset::ButtonMenu::create(cont, S, p);
        bind_action_click(H.base.root, el.action);

        lv_obj_set_grid_cell(H.base.root,
            LV_GRID_ALIGN_STRETCH, c, 1,
            LV_GRID_ALIGN_STRETCH, 1 + r, 1);
    }

    return cont;
}

/* -------------------- menu_list -------------------- */
static lv_obj_t* build_menu_list(lv_obj_t* parent, const ScreenSpecification& spec) {
    auto& S = Ui::getThemeStyles();

    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(cont, 16, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(cont, 12, 0);

    // Título
    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, spec.title.empty() ? "Lista" : spec.title.c_str());
    lv_obj_add_style(title, &Ui::getThemeStyles().labelTitle, 0);

    // Fila por elemento
    for (const auto& el : spec.elements) {
        Ui::Preset::ButtonMenu::Props p{};
        p.text    = strdup_cxx(el.title.empty() ? el.id : el.title);
        p.iconId  = strdup_cxx(resolve_icon_id(el.icon.empty() ? std::string("info") : el.icon));
        p.variant = Ui::Preset::ButtonMenu::Variant::Primary;
        if (!el.action.empty()) p.action = strdup_cxx(el.action);

        auto H = Ui::Preset::ButtonMenu::create(cont, S, p);
        lv_obj_set_width(H.base.root, LV_PCT(100));
        bind_action_click(H.base.root, el.action);
    }

    return cont;
}

/* -------------------- entrypoint -------------------- */
lv_obj_t* ui_view_build(lv_obj_t* parent, const ScreenSpecification& spec)
{
    if (!parent) {
        ESP_LOGE(TAG, "ui_view_build: parent nulo");
        return nullptr;
    }

    if (spec.view == "menu_grid")      return build_menu_grid(parent, spec);
    if (spec.view == "menu_list")      return build_menu_list(parent, spec);

    if (spec.view == "sw_temp_panel") {
        return build_sw_temp_panel(parent, spec); // devuelve root propio del panel
    }

    // Fallback: contenedor simple con mensaje
    lv_obj_t* cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_center(cont);
    lv_obj_set_style_pad_all(cont, 16, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);

    lv_obj_t* lbl = lv_label_create(cont);
    lv_label_set_text_fmt(lbl, "view desconocido: %s", spec.view.c_str());
    ESP_LOGE(TAG, "view desconocido: %s", spec.view.c_str());
    return cont;
}
