#include "ui/view/ui_view_factory.h"
#include "esp_log.h"

#include <vector>
#include <string>
#include <cstring>   // memcpy, strcmp
#include <cstdlib>   // malloc
#include <cstdio>
#include <cmath>

// Tema/controles
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_switch.h"
#include "ui/component/ui_component_button.h"

// Router
#include "ui/router/ui_router.h"

// (Menús) Diagnóstico de comunicaciones
#include "comm/comm_diag.h"
#include "comm/rx/comm_rx_state.h"      // Recepción temperaturas

#include "hmi_command_sender.h"

static const char* TAG = "UI_VIEW_FACTORY";

/* ───────────────────────────── Helpers comunes ───────────────────────────── */

static const char* strdup_cxx(const std::string& s) {
    char* p = (char*)std::malloc(s.size() + 1);
    if (!p) return nullptr;
    std::memcpy(p, s.c_str(), s.size() + 1);
    return p;
}

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
    lv_obj_set_style_text_color(l, s.tokens.colorOnSurface, 0);
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

/* ───────────────────────────── Helper botón menú (sin presets) ───────────────────────────── */

static Ui::Button* make_menu_button(lv_obj_t* parent, Ui::UiThemeStyles& S,
                                    const std::string& text,
                                    const std::string& action) {
    auto* btn = new Ui::Button();
    btn->create(parent);
    btn->setText(text.c_str());

    // Estilos mínimos (mover a Theme cuando toque)
    lv_obj_set_width(btn->root(), LV_PCT(100));
    lv_obj_set_style_pad_hor(btn->root(), 10, 0);
    lv_obj_set_style_pad_ver(btn->root(), 8, 0);

    if (!action.empty()) {
        auto* act = new std::string(action);
        btn->setOnClick([](void* ud){
            auto* a = static_cast<std::string*>(ud);
            ui_router_dispatch(a->c_str());
        }, act);
    }
    return btn;
}

/* ───────────────────────────── Panel diag (menús) ─────────────────────────────
 * Se mantiene para 'menu_grid'. NO se usa en la vista de temperaturas.
 */
static lv_obj_t* add_comm_diag_panel(lv_obj_t* parent)
{
    lv_obj_t* panel = lv_obj_create(parent);
    lv_obj_set_size(panel, 260, LV_PCT(100));
    lv_obj_set_style_pad_all(panel, 12, 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(panel, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_set_style_radius(panel, 10, 0);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(panel, 6, 0);

    // Título
    lv_obj_t* title = lv_label_create(panel);
    lv_label_set_text(title, "Comm · Diag");
    lv_obj_set_style_text_font(title, Ui::getThemeStyles().tokens.fontTitle, 0);

    // Labels
    auto mk = [&](const char* txt){
        lv_obj_t* l = lv_label_create(panel);
        lv_label_set_text(l, txt);
        lv_obj_set_style_text_font(l, Ui::getThemeStyles().tokens.fontBody, 0);
        return l;
    };
    lv_obj_t* l_txq   = mk("TX queued: 0");
    lv_obj_t* l_txlb  = mk("TX loopback: 0");
    lv_obj_t* l_txok  = mk("TX ok: 0   / TX fail: 0");
    lv_obj_t* l_rxpk  = mk("RX pkts: 0 / RX dropped: 0");
    lv_obj_t* l_rxack = mk("RX ack: 0  / last seq: 0");

    struct State { lv_obj_t* a; lv_obj_t* b; lv_obj_t* c; lv_obj_t* d; lv_obj_t* e; lv_timer_t* timer; };
    State* st = (State*)lv_mem_alloc(sizeof(State));
    *st = { l_txq, l_txlb, l_txok, l_rxpk, l_rxack, nullptr };

    st->timer = lv_timer_create([](lv_timer_t* t){
        auto* s = (State*)t->user_data;
        auto d = CommDiag::get();
        lv_label_set_text_fmt(s->a, "TX queued: %lu",   (unsigned long)d.txQueued);
        lv_label_set_text_fmt(s->b, "TX loopback: %lu", (unsigned long)d.txLoopback);
        lv_label_set_text_fmt(s->c, "TX ok: %lu / TX fail: %lu",
                              (unsigned long)d.txOk, (unsigned long)d.txFail);
        lv_label_set_text_fmt(s->d, "RX pkts: %lu / RX dropped: %lu",
                              (unsigned long)d.rxPkt, (unsigned long)d.rxDropped);
        lv_label_set_text_fmt(s->e, "RX ack: %lu  / last seq: %u",
                              (unsigned long)d.rxAck, d.lastSeq);
    }, 1000, st);

    lv_obj_add_event_cb(panel, [](lv_event_t* e){
        auto* s = (State*)lv_event_get_user_data(e);
        if (!s) return;
        if (s->timer) lv_timer_del(s->timer);
        lv_mem_free(s);
    }, LV_EVENT_DELETE, st);

    return panel;
}

/* ─────────────────────────── Vista: temps_2x5 ───────────────────────────
 * 10 sensores (2 filas × 5 columnas) + footer con 3 switches.
 * NO muestra panel de comunicaciones aquí (solo en pantalla principal/menús).
 */

// Contexto (solo para demo de refresco hasta conectar provider real)
struct Temps2x5Ctx {
    lv_obj_t* temp_val[10];
    int       temp_count;
    lv_timer_t* timer;
};

static void temps2x5_ctx_on_delete(lv_event_t* e) {
    Temps2x5Ctx* ctx = (Temps2x5Ctx*)lv_event_get_user_data(e);
    if (!ctx) return;
    if (ctx->timer) { lv_timer_del(ctx->timer); ctx->timer = nullptr; }
    for (int i = 0; i < 10; ++i) ctx->temp_val[i] = nullptr;
    lv_mem_free(ctx);
}

static void temps2x5_timer_cb(lv_timer_t* t) {
    Temps2x5Ctx* ctx = (Temps2x5Ctx*)t->user_data;
    static const int kSensorMap[10] = { 1, 3, 5, 7, 9, 0, 2, 4, 6, 8 };
    if (!ctx) return;

    auto vals = CommRxState::getTemps();
    for (int i = 0; i < ctx->temp_count; ++i) {
        lv_obj_t* lab = ctx->temp_val[i];
        if (!lab || !lv_obj_is_valid(lab)) continue;

        int srcIdx = kSensorMap[i];  // índice real en vals[]
        if (srcIdx >= 0 && !std::isnan(vals[srcIdx])) {
            lv_label_set_text_fmt(lab, "%.1f °C", vals[srcIdx]);
        } else {
            lv_label_set_text(lab, "--.- °C");
        }
    }
}

/* Variante basada en TÍTULOS (pintamos SIEMPRE 10 tiles en orden canónico)
 * Valor (temperatura) en la línea superior, Título en la inferior.
 */
static lv_obj_t* create_sensors_grid_2x5_titles(lv_obj_t* parent,
                                                const std::vector<std::string>& titles,
                                                Temps2x5Ctx* ctx)
{
    Ui::UiThemeStyles& s = Ui::getThemeStyles();

    lv_obj_t* grid = lv_obj_create(parent);
    Ui::applySensorGrid(grid, Ui::getThemeStyles());

    static lv_coord_t col_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
        LV_GRID_TEMPLATE_LAST
    };
    static lv_coord_t row_dsc[] = { LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST };
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(grid, col_dsc, row_dsc);
    lv_obj_set_style_pad_row(grid, 8, 0);
    lv_obj_set_style_pad_column(grid, 8, 0);

    const lv_color_t kCellBg = lv_color_make(0x2f, 0xad, 0x32); // verde claro

    for (int i = 0; i < 10; ++i) {
        const int r = (i < 5) ? 0 : 1;
        const int c = (i < 5) ? i : i - 5;

        // Caja de la celda
        lv_obj_t* cell = lv_obj_create(grid);
        lv_obj_remove_style_all(cell);
        lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_STRETCH, c, 1,
                                   LV_GRID_ALIGN_STRETCH, r, 1);
        lv_obj_set_style_bg_opa(cell, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(cell, kCellBg, 0);
        lv_obj_set_style_radius(cell, 8, 0);
        lv_obj_set_style_pad_all(cell, 10, 0);
        lv_obj_set_style_min_height(cell, 96, 0);

        // Layout interno: COLUMNA → [ Valor (arriba, alineado a derecha) | Título (abajo) ]
        lv_obj_set_flex_flow(cell, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_pad_row(cell, 4, 0);   // separación vertical interna

        // Valor (arriba)
        lv_obj_t* v = lv_label_create(cell);
        lv_label_set_text(v, "--.- °C");
        lv_obj_set_style_text_color(v, lv_color_white(), 0);
        lv_obj_set_style_text_font(v, s.tokens.fontTitle, 0);
        lv_obj_set_style_text_align(v, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_width(v, LV_PCT(100));       // para que el align RIGHT tenga efecto

        // Título (abajo)
        const char* titleTxt = (i < (int)titles.size()) ? titles[i].c_str() : "—";
        lv_obj_t* title = lv_label_create(cell);
        lv_label_set_text(title, titleTxt);
        lv_label_set_long_mode(title, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(title, LV_PCT(100));
        lv_obj_set_style_text_color(title, lv_color_white(), 0);
        lv_obj_set_style_text_font(title, s.tokens.fontBody, 0);
        lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, 0);

        if (ctx && ctx->temp_count < 10) ctx->temp_val[ctx->temp_count++] = v;
    }
    return grid;
}


// Footer con 3 switches (fondo transparente para evitar “caja blanca”)
static lv_obj_t* create_footer_switches(lv_obj_t* parent,
                                        const std::vector<const ElementSpecification*>& sws)
{
    Ui::UiThemeStyles& s = Ui::getThemeStyles();

    lv_obj_t* footer = lv_obj_create(parent);
    lv_obj_remove_style_all(footer);
    lv_obj_set_width(footer, LV_PCT(100));
    lv_obj_set_height(footer, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(footer, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(footer,
                          LV_FLEX_ALIGN_SPACE_EVENLY,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(footer, 6, 0);

    const int N = (int)sws.size();
    const int M = (N > 3) ? 3 : N;

    for (int i = 0; i < M; ++i) {
        const auto* e = sws[i];

        lv_obj_t* cont = lv_obj_create(footer);

        lv_obj_remove_style_all(cont);
        lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(cont, Ui::getThemeStyles().tokens.colorSurfaceVariant, 0);
        lv_obj_set_style_radius(cont, 8, 0);
        lv_obj_set_style_pad_all(cont, 8, 0);
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_column(cont, 6, 0);
        lv_obj_set_height(cont, 50);

        lv_obj_t* lbl = lv_label_create(cont);
        lv_label_set_text(lbl, e->title.empty() ? e->id.c_str() : e->title.c_str());
        lv_obj_set_style_text_font(lbl, s.tokens.fontBody, 0);
        lv_obj_set_style_text_color(lbl, s.tokens.colorOnSurface, 0);

        auto* sw = new Ui::Switch();
        sw->create(cont);

        // enabled/disabled
        if (!e->enabled) lv_obj_add_state(sw->root(), LV_STATE_DISABLED);

        // (opcional) tamaño/radio -> mejor en applyTheme(), pero si urge:
        lv_obj_set_size(sw->root(), 60, 30);
        lv_obj_set_style_radius(sw->root(), 15, 0);

        // acción
        if (!e->action.empty()) {
            auto* action = new std::string(e->action);
            sw->setOnToggle([](bool state, void* ud){
                auto* act = static_cast<std::string*>(ud);
                ESP_LOGI("UI_ROUTER", "dispatch: %s", act->c_str());

                if (*act == "DO:/io/sw1") {
                    const char* op = state ? "set_true" : "set_false";
                    bool ok = Hmi::CommandSender::send_do("io/led_builtin", op, true);
                    ESP_LOGI("UI_ROUTER", "DO:/io/sw1 -> %s", ok ? "OK" : "FAIL");
                } else {
                    ui_router_dispatch(act->c_str());
                }
            }, action);
        }
    }
    return footer;
}


static lv_obj_t* build_temps2x5_panel(lv_obj_t* parent, const ScreenSpecification& spec)
{
    // Contenedor raíz en columna
    lv_obj_t* root = lv_obj_create(parent);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root, 16, 0);
    lv_obj_set_style_pad_row(root, 12, 0);

    // Contexto + cleanup
    Temps2x5Ctx* ctx = (Temps2x5Ctx*)lv_mem_alloc(sizeof(Temps2x5Ctx));
    if (ctx) {
        ctx->timer = nullptr;
        ctx->temp_count = 0;
        for (int i = 0; i < 10; ++i) ctx->temp_val[i] = nullptr;
        lv_obj_add_event_cb(root, temps2x5_ctx_on_delete, LV_EVENT_DELETE, ctx);
    }

    // 1) Switches (ids que empiezan por "sw")
    std::vector<const ElementSpecification*> sws;
    sws.reserve(3);
    for (const auto& e : spec.elements) {
        if (!e.visible) continue;
        if (e.id.rfind("sw", 0) == 0 && (int)sws.size() < 3) sws.push_back(&e);
    }

    // 2) Títulos canónicos (10 sensores en orden fijo en pantalla)
    static const char* kSensorTitles[10] = {
        "Caldera OUT",  "Depósito TOP",      "Suelo OUT",   "Fancoil OUT",  "Exterior",
        "Caldera IN",   "Depósito BOTTOM",   "Suelo IN",    "Fancoil IN",   "Libre"
    };

    std::vector<std::string> sensorTitles;
    sensorTitles.reserve(10);
    for (int i = 0; i < 10; ++i) sensorTitles.emplace_back(kSensorTitles[i]);

    // Grid 2×5 (10 tiles) + footer
    (void)create_sensors_grid_2x5_titles(root, sensorTitles, ctx);
    (void)create_footer_switches(root, sws);

    // Timer DEMO (quitar al conectar provider real)
    if (ctx && ctx->temp_count > 0) {
        ctx->timer = lv_timer_create(temps2x5_timer_cb, 500, ctx);
    }

    // NO añadir add_comm_diag_panel(root) aquí (se pidió ocultarlo en esta pantalla)
    return root;
}

/* ───────────────────────────── Vistas de menú ───────────────────────────── */

static lv_obj_t* build_menu_grid(lv_obj_t* parent, const ScreenSpecification& spec) {
    auto& S = Ui::getThemeStyles();

    const int COLS = 3;
    const lv_coord_t GAP = 16;
    const lv_coord_t OUTER_PAD = 16;

    const int n = (int)spec.elements.size();
    const int body_rows = n > 0 ? ((n + COLS - 1) / COLS) : 1;

    // Contenedor fila: [grid | panel diag]
    lv_obj_t* row = lv_obj_create(parent);
    lv_obj_set_size(row, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(row, 12, 0);

    // Contenedor grid
    lv_obj_t* cont = lv_obj_create(row);
    lv_obj_set_flex_grow(cont, 1);
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

        auto* B = make_menu_button(cont, S,
                                   el.title.empty() ? el.id : el.title,
                                   el.action);

        lv_obj_set_grid_cell(B->root(),
            LV_GRID_ALIGN_STRETCH, c, 1,
            LV_GRID_ALIGN_STRETCH, 1 + r, 1);
    }

    add_comm_diag_panel(row); // Mantener diag en menú
    return row;
}

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
        auto* B = make_menu_button(cont, S,
                                   el.title.empty() ? el.id : el.title,
                                   el.action);
        lv_obj_set_width(B->root(), LV_PCT(100));

        Ui::applyMenuButtonBox(B->root(), S);
    }

    return cont;
}

/* ───────────────────────────── Entrypoint ───────────────────────────── */

lv_obj_t* ui_view_build(lv_obj_t* parent, const ScreenSpecification& spec)
{
    if (!parent) {
        ESP_LOGE(TAG, "ui_view_build: parent nulo");
        return nullptr;
    }

    if (spec.view == "menu_grid")      return build_menu_grid(parent, spec);
    if (spec.view == "menu_list")      return build_menu_list(parent, spec);

    // Vista de temperaturas 2×5 + footer 3 switches (sin estado de comunicaciones)
    if (spec.view == "temps_2x5" ||
        spec.view == "screen/temps_2x5" ||
        spec.view == "temperaturas_2x5" ||
        spec.view == "sw_temp_panel" /* alias legado para compatibilidad */) {
        return build_temps2x5_panel(parent, spec);
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
