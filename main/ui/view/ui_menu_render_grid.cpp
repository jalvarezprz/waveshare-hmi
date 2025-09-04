#include "ui_menu_render_grid.h"
#include "ui/view/ui_view_menu_grid.h"
#include "ui/ui_router_mount.h"
#include "ui/ui_router_adapter.h"
#include "ui/theme/ui_theme_styles.h"

#include <vector>
#include <string>

extern "C" {
#include "cJSON.h"
}

/* Helpers JSON */
static const cJSON* get_array_any(const cJSON* obj, const char* a, const char* b)
{
    if (!obj) return nullptr;
    const cJSON* n = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, a);
    if (!cJSON_IsArray(n)) n = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, b);
    return cJSON_IsArray(n) ? n : nullptr;
}
static const char* get_str_any(const cJSON* obj, const char* a, const char* b, const char* c = nullptr)
{
    if (!obj) return nullptr;
    const cJSON* n = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, a);
    if (!cJSON_IsString(n) || !n->valuestring || !*n->valuestring) {
        n = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, b);
        if ((!cJSON_IsString(n) || !n->valuestring || !*n->valuestring) && c) {
            n = cJSON_GetObjectItemCaseSensitive((cJSON*)obj, c);
        }
    }
    return (cJSON_IsString(n) && n->valuestring && *n->valuestring) ? n->valuestring : nullptr;
}

/* Contexto id → cJSON* */
struct GridCtx {
    std::vector<std::pair<std::string, const cJSON*>> map;
};

static void on_grid_item(const char* id, void* ctx_void)
{
    auto* ctx = static_cast<GridCtx*>(ctx_void);
    if (!ctx || !id) return;
    const cJSON* item = nullptr;
    for (auto& kv : ctx->map) {
        if (kv.first == id) { item = kv.second; break; }
    }
    if (!item) {
        LV_LOG_WARN("Grid: id '%s' no encontrado", id);
        return;
    }
    ui_router_on_menu_item_selected(item);
}

void ui_menu_render_grid_from_node(const cJSON* node)
{
    if (!node) return;
    lv_obj_t* parent = ui_router_mount_get();
    if (!parent) parent = lv_scr_act();

    // Limpiamos SOLO el contenedor de contenido
    lv_obj_clean(parent);

    // Título (opcional)
    const char* title = get_str_any(node, "title", "name");
    if (title && *title) {
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, title);
        lv_obj_add_style(lbl, &Ui::getThemeStyles().base, LV_PART_MAIN);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 8);
    }

    // Items
    const cJSON* items = get_array_any(node, "items", "children");

    auto show_demo_in = [](lv_obj_t* pr, bool leave_top_gap) {
        static const UiMenuGridItem DEMO_ITEMS[] = {
            { "menu_settings",  "Ajustes",      LV_SYMBOL_SETTINGS },
            { "menu_stats",     "Estadísticas", LV_SYMBOL_DIRECTORY },
            { "menu_alarms",    "Alarmas",      LV_SYMBOL_WARNING },
            { "menu_network",   "Red",          LV_SYMBOL_WIFI },
            { "menu_devices",   "Dispositivos", LV_SYMBOL_LIST },
            { "menu_power",     "Energía",      LV_SYMBOL_POWER },
        };
        static UiMenuGridCallbacks DEMO_CBS{
            /*onItem=*/[](const char* id, void*){ LV_LOG_USER("DEMO pressed: %s", id); },
            /*ctx=*/nullptr
        };

        lv_obj_t* cont_demo = pr;
        if (leave_top_gap) {
            cont_demo = lv_obj_create(pr);
            lv_obj_remove_style_all(cont_demo);
            lv_obj_set_size(cont_demo, LV_PCT(100), LV_PCT(100));
            lv_obj_set_style_pad_top(cont_demo, 56, 0);
            lv_obj_set_style_pad_left(cont_demo, 8, 0);
            lv_obj_set_style_pad_right(cont_demo, 8, 0);
            lv_obj_set_style_pad_bottom(cont_demo, 8, 0);
            lv_obj_set_style_bg_opa(cont_demo, LV_OPA_TRANSP, 0);
        }
        (void)ui_create_menu_grid(cont_demo, DEMO_ITEMS,
                                  sizeof(DEMO_ITEMS)/sizeof(DEMO_ITEMS[0]), &DEMO_CBS, 3);

        lv_obj_t* dbg = lv_label_create(pr);
        lv_label_set_text(dbg, "DEBUG: demo grid mostrado");
        lv_obj_set_style_text_color(dbg, lv_color_white(), 0);
        lv_obj_align(dbg, LV_ALIGN_BOTTOM_MID, 0, -8);
    };

    if (!items) {
        LV_LOG_WARN("Grid: no hay 'items' ni 'children' en el nodo; mostrando demo local");
        show_demo_in(parent, title && *title);
        return;
    }

    // 1) Parsear a strings
    std::vector<std::string> ids, texts, icons;
    auto* ctx = new GridCtx();

    int parsed = 0;
    const cJSON* it = nullptr;
    cJSON_ArrayForEach(it, items)
    {
        const char* id   = get_str_any(it, "id", "path", "key");
        const char* text = get_str_any(it, "title", "text", "name");
        const char* icon = get_str_any(it, "icon",  "symbol");
        if (!id || !text) continue;
        ids.emplace_back(id);
        texts.emplace_back(text);
        icons.emplace_back(icon ? icon : "");
        ctx->map.emplace_back(ids.back(), it);
        parsed++;
    }
    LV_LOG_USER("Grid: items parseados = %d", parsed);

    if (ids.empty()) {
        LV_LOG_WARN("Grid: lista de items vacía; mostrando demo local");
        show_demo_in(parent, title && *title);
        delete ctx;
        return;
    }

    // 2) Construir UiMenuGridItem con punteros estables
    std::vector<UiMenuGridItem> v;
    v.reserve(ids.size());
    for (size_t i = 0; i < ids.size(); ++i) {
        v.push_back(UiMenuGridItem{ ids[i].c_str(), texts[i].c_str(), icons[i].c_str() });
        LV_LOG_USER("GRID BUILD: item %d id='%s' text='%s'", (int)i, v.back().id, v.back().text);
    }

    // Contenedor para el grid (si hay título, deja hueco arriba)
    lv_obj_t* cont = parent;
    if (title && *title) {
        cont = lv_obj_create(parent);
        lv_obj_remove_style_all(cont);
        lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
        lv_obj_set_style_pad_top(cont, 56, 0);
        lv_obj_set_style_pad_left(cont, 8, 0);
        lv_obj_set_style_pad_right(cont, 8, 0);
        lv_obj_set_style_pad_bottom(cont, 8, 0);
        lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    }

    static UiMenuGridCallbacks CBS{
        /*onItem=*/on_grid_item,
        /*ctx=*/nullptr
    };
    CBS.ctx = ctx;

    (void)ui_create_menu_grid(cont, v.data(), v.size(), &CBS, 3);

    // Depuración visual
    lv_obj_t* dbg = lv_label_create(parent);
    lv_label_set_text(dbg, "DEBUG: grid real mostrado");
    lv_obj_set_style_text_color(dbg, lv_color_white(), 0);
    lv_obj_align(dbg, LV_ALIGN_BOTTOM_MID, 0, -8);
}
