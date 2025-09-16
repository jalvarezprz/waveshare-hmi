#include "ui_view_menu_grid.h"
#include <vector>
#include <string>

// (opcional) fuente con acentos — solo si está disponible
#if defined(LV_FONT_MONTSERRAT_20) && LV_FONT_MONTSERRAT_20
LV_FONT_DECLARE(lv_font_montserrat_20)
lv_obj_set_style_text_font(lbl, &lv_font_montserrat_20, 0);
#endif

/* ───────────────────── Estilo tarjeta (alto contraste) ───────────────────── */
static void style_button_card(lv_obj_t* btn)
{
    lv_obj_set_style_radius(btn, 16, 0);
    lv_obj_set_style_pad_all(btn, 12, 0);

    // Fondo y borde alto contraste
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x1E1E1E), 0); // gris oscuro
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_border_color(btn, lv_color_white(), 0);
    lv_obj_set_style_border_opa(btn, LV_OPA_COVER, 0);

    // Sombra suave
    lv_obj_set_style_shadow_width(btn, 12, 0);
    lv_obj_set_style_shadow_opa(btn, LV_OPA_40, 0);
    lv_obj_set_style_shadow_ofs_y(btn, 6, 0);
}

/* ───────────────────────────── Callback pulsación ─────────────────────────── */
static void on_btn_clicked(lv_event_t* e)
{
    const char* id_cstr = static_cast<const char*>(lv_event_get_user_data(e));
    if (!id_cstr) return;
    lv_obj_t* btn  = lv_event_get_target(e);
    lv_obj_t* grid = lv_obj_get_parent(btn);
    auto* cbs = reinterpret_cast<const UiMenuGridCallbacks*>(lv_obj_get_user_data(grid));
    if (cbs && cbs->onItem) cbs->onItem(id_cstr, cbs->ctx);
}

/* ───────────────────────────── API principal ──────────────────────────────── */
lv_obj_t* ui_create_menu_grid(lv_obj_t* parent,
                              const UiMenuGridItem* items,
                              size_t count,
                              const UiMenuGridCallbacks* cbs,
                              uint8_t cols)
{
    (void)cols; // con FLEX no lo usamos
    if (!parent || !items || count == 0) return nullptr;

    // Contenedor del “grid” → FLEX ROW WRAP (siempre visible)
    lv_obj_t* grid = lv_obj_create(parent);
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_all(grid, 16, 0);
    lv_obj_set_style_pad_row(grid, 16, 0);
    lv_obj_set_style_pad_column(grid, 16, 0);
    lv_obj_set_user_data(grid, (void*)cbs);

    // Layout FLEX
    lv_obj_set_layout(grid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Borde suave para ver el área (útil en depuración)
    lv_obj_set_style_bg_opa(grid, LV_OPA_20, 0);
    lv_obj_set_style_bg_color(grid, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(grid, 1, 0);
    lv_obj_set_style_border_color(grid, lv_color_hex(0xCCCCCC), 0);

    LV_LOG_USER("GRID DEBUG (FLEX): count=%d", (int)count);

    for (size_t i = 0; i < count; ++i) {
        const auto& it = items[i];

        lv_obj_t* btn = lv_btn_create(grid);
        style_button_card(btn);

        // 3 columnas → 33% de ancho; altura auto con un mínimo para “tarjeta”
        lv_obj_set_style_width(btn, LV_PCT(33), 0);
        lv_obj_set_style_height(btn, LV_SIZE_CONTENT, 0);
        lv_obj_set_style_min_height(btn, 120, 0);
        lv_obj_set_flex_grow(btn, 0);

        // Estructura interna de la tarjeta: columna centrada
        lv_obj_set_flex_flow(btn, LV_FLEX_FLOW_COLUMN);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

        // Label combinado (icono + texto)
        std::string label_txt;
        if (it.icon && it.icon[0] != '\0') {
            label_txt += it.icon;
            label_txt += "\n";
        }
        label_txt += (it.text ? it.text : "");

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, label_txt.c_str());
        lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_top(lbl, 6, 0);
        lv_obj_set_style_pad_bottom(lbl, 6, 0);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0); // alto contraste

        lv_obj_add_event_cb(btn, on_btn_clicked, LV_EVENT_CLICKED, (void*)it.id);

        LV_LOG_USER("GRID DEBUG (FLEX): creado botón %d id='%s'", (int)i, it.id ? it.id : "(null)");
    }
    return grid;
}

/* ───────────────────────────── Demo opcional ──────────────────────────────── */
static void demo_on_item_selected(const char* id)
{
    LV_LOG_USER("Grid item pressed: %s", id);
}

void ui_menu_grid_demo_show()
{
    static const UiMenuGridItem DEMO_ITEMS[] = {
        { "menu_settings",  "Ajustes",      LV_SYMBOL_SETTINGS },
        { "menu_stats",     "Estadísticas", LV_SYMBOL_DIRECTORY },
        { "menu_alarms",    "Alarmas",      LV_SYMBOL_WARNING },
        { "menu_network",   "Red",          LV_SYMBOL_WIFI },
        { "menu_devices",   "Dispositivos", LV_SYMBOL_LIST },
        { "menu_power",     "Energía",      LV_SYMBOL_POWER },
    };
    lv_obj_clean(lv_scr_act());
    static const UiMenuGridCallbacks DEMO_CBS{
        /*onItem=*/[](const char* id, void*){ demo_on_item_selected(id); },
        /*ctx=*/nullptr
    };
    (void)ui_create_menu_grid(lv_scr_act(), DEMO_ITEMS, sizeof(DEMO_ITEMS)/sizeof(DEMO_ITEMS[0]),
                              &DEMO_CBS, /*cols=*/3);
}
