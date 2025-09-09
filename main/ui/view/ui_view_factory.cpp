#include "ui/view/ui_view_factory.h"
#include "esp_log.h"
#include <vector>
#include <string>
#include <cstring>   // <-- necesario para memcpy
#include <cstdlib>   // <-- necesario para malloc

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
// Devuelve std::string para evitar punteros colgantes.
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

// Engancha NAV/DO a un objeto LVGL sin depender del preset.
// Copia 'act' en heap y la libera al borrar el objeto.
static void bind_action_click(lv_obj_t* obj, const std::string& act) {
    if (!obj || act.empty()) return;

    // Copia estable en heap (liberada en LV_EVENT_DELETE)
    char* payload = (char*)lv_mem_alloc(act.size() + 1);
    if (!payload) return;
    std::memcpy(payload, act.c_str(), act.size() + 1);

    lv_obj_add_event_cb(obj, [](lv_event_t* e){
        auto* s = (const char*)lv_event_get_user_data(e);
        if (!s) return;

        switch (lv_event_get_code(e)) {
            case LV_EVENT_CLICKED:
                ui_router_dispatch(s);     // ← dispara NAV:/... o DO:/...
                break;
            case LV_EVENT_DELETE:
                lv_mem_free((void*)s);     // liberar copia
                break;
            default:
                break;
        }
    }, LV_EVENT_ALL, payload);
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
lv_obj_t* ui_view_build(lv_obj_t* parent, const ScreenSpecification& spec) {
    if (spec.view == "menu_grid") return build_menu_grid(parent, spec);
    if (spec.view == "menu_list") return build_menu_list(parent, spec);

    // Fallback
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
