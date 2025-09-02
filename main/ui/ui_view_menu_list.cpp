#include "ui_view_menu_list.h"
#include "esp_log.h"
#include "lvgl.h"

#include <cstring>     // strcmp
#include <cstdlib>     // malloc, free
#include <string.h>    // ::strdup
#include <vector>
#include <string>
#include <cmath>       // fabs, round, llround, pow

extern "C" {
#include "cJSON.h"
}

#include "ui_router.h"
#include "ui_pin.h"
#include "ui_menu_json_utilities.h"
#include "ui_menu_nav.h"
#include "ui_menu_render_detail.h"
#include "ui/theme/ui_theme_styles.h"

// ==== Prototipos de helpers locales (declaración adelantada) ====
static void decorate_list_item_icons(lv_obj_t* btn, bool has_children);
static void adjust_list_item_title_layout(lv_obj_t* btn, bool has_value);
static void style_and_decorate_list_item(lv_obj_t* btn, bool has_children, bool has_value);

static const char* TAG = "UI_MENU_JSON_TREE";

/* ======= forward del renderer genérico de lista (SE USA MÁS ABAJO) ======= */
static void ui_show_menu_generic();
static void ui_detail_back_bridge(void) {
    ui_show_menu_generic();
}

/* ======= forward de la factoría de widgets de detalle ==================== */
//static lv_obj_t* create_field_widget(lv_obj_t* parent, const cJSON* f);

/* ======================= Render genérico ======================== */
typedef struct {
    char* id;           // id del item
    bool  has_children; // si tiene "items"
} MenuItemUD;

/* forward ya declarado arriba */
// static void ui_show_menu_generic();

static void ui_show_menu_generic()
{
    lv_obj_clean(lv_scr_act());

    cJSON* root = ui_menu_json_load(); if (!root) return;

    cJSON* children = nullptr;
    const char* title_txt = "Menú";
    if (!Ui::Menu::ui_menu_nav_find(root, &children, &title_txt)) {
        ESP_LOGE(TAG, "Ruta inválida");
        cJSON_Delete(root);
        return;
    }

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Menú");
    lv_obj_set_style_text_font(title, Ui::getThemeTokens().fontBody, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    if (children && cJSON_IsArray(children)) {
        lv_obj_t* list = lv_list_create(cont);
        lv_obj_set_size(list, 740, 320);
        lv_obj_align(list, LV_ALIGN_CENTER, 0, 10);

        cJSON* it = nullptr;
        cJSON_ArrayForEach(it, children) {
            const char* id   = cJSON_GetStringValue(cJSON_GetObjectItem(it, "id"));
            const char* text = cJSON_GetStringValue(cJSON_GetObjectItem(it, "title"));
            cJSON* items     = cJSON_GetObjectItem(it, "items");
            bool has_children = items && cJSON_IsArray(items);

            lv_obj_t* btn = lv_list_add_btn(list, NULL, text ? text : "?");

            /* Representación dinámica en listas: si hay "value" en el item, muéstralo a la derecha */
            const char* val = cJSON_GetStringValue(cJSON_GetObjectItem(it, "value"));
            if (val) {
                lv_obj_set_width(btn, LV_PCT(100));
                lv_obj_t* lbl_val = lv_label_create(btn);
                lv_label_set_text(lbl_val, val);
                lv_obj_set_style_text_font(lbl_val, Ui::getThemeTokens().fontBody, LV_PART_MAIN);
                lv_label_set_long_mode(lbl_val, LV_LABEL_LONG_CLIP);
                lv_obj_set_width(lbl_val, LV_SIZE_CONTENT);
                lv_obj_align(lbl_val, LV_ALIGN_RIGHT_MID, -10, 0);
    

                /* Opcional: acotar ancho del label de título para evitar solapamientos */
                lv_obj_t* title_lbl = lv_obj_get_child(btn, 0);
                if (title_lbl) {
                    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_DOT);
                    lv_obj_set_style_text_font(title_lbl, Ui::getThemeTokens().fontBody, LV_PART_MAIN);
                    lv_obj_set_width(title_lbl, LV_PCT(70));
                }
            }

            MenuItemUD* ud = (MenuItemUD*) std::malloc(sizeof(MenuItemUD));
            ud->id = id ? ::strdup(id) : NULL;   // <- corregido
            ud->has_children = has_children;

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MenuItemUD* ud = (MenuItemUD*) lv_event_get_user_data(e);
                if (!ud || !ud->id) return;

                /* Si estamos en raíz y es protegido, pedimos PIN antes de entrar */
                if (Ui::Menu::ui_menu_nav_path().empty() && Ui::Menu::ui_menu_nav_is_protected_root_id(ud->id)) {
                    ui_show_pin_dialog(2410, [id_copy = std::string(ud->id)](bool ok){
                        if (!ok) return;
                        Ui::Menu::ui_menu_nav_push(id_copy);
                        ui_show_menu_generic();
                    });
                    return;
                }

                if (ud->has_children) {
                    Ui::Menu::ui_menu_nav_push(ud->id);
                    ui_show_menu_generic();
                } else {
                    /* Hoja: ¿es una vista "detail"? */
                    cJSON* root_local = ui_menu_json_load();
                    if (root_local) {
                        const cJSON* node = Ui::Menu::ui_menu_nav_find_leaf(root_local, ud->id);
                        const char* view = node ? cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)node, "view")) : nullptr;
                        if (node && view && std::strcmp(view, "detail")==0) {
                            ui_menu_render_detail_from_node(node, ui_detail_back_bridge);
                        } else {
                            ESP_LOGI(TAG, "Leaf selected: %s (sin view:\"detail\")", ud->id);
                        }
                        cJSON_Delete(root_local);
                    }
                }
            }, LV_EVENT_CLICKED, ud);

            lv_obj_add_event_cb(btn, [](lv_event_t* e){
                MenuItemUD* ud = (MenuItemUD*) lv_event_get_user_data(e);
                if (ud) {
                    if (ud->id) std::free(ud->id);
                    std::free(ud);
                }
            }, LV_EVENT_DELETE, ud);

            // === Estilizar y decorar este ítem ===
            const bool has_value = (val && *val);
            style_and_decorate_list_item(btn, has_children, has_value);
        }
        
        auto& styles = Ui::getThemeStyles();
        Ui::applyListStylesToChildren(list, styles, /*large=*/false, /*withDivider=*/true);
    } else {
        lv_obj_t* info = lv_label_create(cont);
        lv_label_set_text(info, "Elemento sin submenús (TODO: acción específica)");
        lv_obj_align(info, LV_ALIGN_CENTER, 0, 0);
    }

    if (!Ui::Menu::ui_menu_nav_path().empty()) {
        lv_obj_t* back = lv_btn_create(cont);
        lv_obj_set_size(back, 120, 48);
        lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
        lv_obj_t* l = lv_label_create(back);
        lv_label_set_text(l, LV_SYMBOL_LEFT " Atrás");
        lv_obj_set_style_text_font(l, Ui::getThemeTokens().fontBody, LV_PART_MAIN); // acentos OK
        lv_obj_center(l);
        
        lv_obj_add_event_cb(back, [](lv_event_t*){
            if (!Ui::Menu::ui_menu_nav_path().empty()) Ui::Menu::ui_menu_nav_pop();
            if (Ui::Menu::ui_menu_nav_path().empty()) {
                ui_router_go(UiScreen::MAIN_MENU);
            } else {
                ui_show_menu_generic();
            }
        }, LV_EVENT_CLICKED, nullptr);
    }

    cJSON_Delete(root);
}

/* ===================== Entry points públicos ==================== */
void ui_build_main_menu() {
    Ui::Menu::ui_menu_nav_clear();
    ui_show_menu_generic();
}

void ui_build_info_menu() {
    Ui::Menu::ui_menu_nav_clear();
    Ui::Menu::ui_menu_nav_push("info");
    ui_show_menu_generic();
}

// ==== Helpers locales de decoración de ítems de lista (refactor) ====

/** Crea icono izquierdo (según si hay hijos) y chevron derecho (si hay hijos). */
static void decorate_list_item_icons(lv_obj_t* btn, bool has_children) {
    if (!btn) return;

    // Izquierda
    lv_obj_t* icon_left = lv_label_create(btn);
    lv_label_set_text(icon_left, has_children ? LV_SYMBOL_SETTINGS : LV_SYMBOL_EDIT);

    // ⬇️ usa la fuente built-in de LVGL, que trae los símbolos
    lv_obj_set_style_text_font(icon_left, LV_FONT_DEFAULT, LV_PART_MAIN);

    // Color apagado igual que antes (si quieres mantenerlo)
    lv_obj_set_style_text_color(icon_left, Ui::getThemeTokens().colorMuted, LV_PART_MAIN);
    lv_obj_align(icon_left, LV_ALIGN_LEFT_MID, +8, 0);

    // Derecha (chevron)
    if (has_children) {
        lv_obj_t* chevron = lv_label_create(btn);
        lv_label_set_text(chevron, LV_SYMBOL_RIGHT);
        lv_obj_set_style_text_font(chevron, LV_FONT_DEFAULT, LV_PART_MAIN);  // ⬅️ built-in
        lv_obj_set_style_text_color(chevron, Ui::getThemeTokens().colorMuted, LV_PART_MAIN);
        lv_obj_align(chevron, LV_ALIGN_RIGHT_MID, -8, 0);
    }
}

/** Ajusta título (primer hijo del btn) y, si hay valor a la derecha, recorta anchos. */
static void adjust_list_item_title_layout(lv_obj_t* btn, bool has_value) {
    if (!btn) return;
    lv_obj_t* title_lbl = lv_obj_get_child(btn, 0); // label creado por lv_list_add_btn
    if (!title_lbl) return;

    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_DOT);
    // Deja más ancho si NO hay valor; menos si sí lo hay
    lv_obj_set_width(title_lbl, has_value ? LV_PCT(58) : LV_PCT(70));
    // Desplaza a la derecha para no solaparse con el icono izquierdo
    lv_obj_align(title_lbl, LV_ALIGN_LEFT_MID, +28, 0);
}

/** Helper integral: aplica estilos de ítem (altura/padding) y decora iconos+chevron+título. */
static void style_and_decorate_list_item(lv_obj_t* btn, bool has_children, bool has_value) {
    if (!btn) return;
    auto& styles = Ui::getThemeStyles();
    Ui::applyListItem(btn, styles, /*large=*/false, /*withDivider=*/true);
    decorate_list_item_icons(btn, has_children);
    adjust_list_item_title_layout(btn, has_value);
}

/* (helpers opcionales)
void ui_build_tend_menu() { g_path = {"tend"}; ui_show_menu_generic(); }
void ui_build_params_menu() { g_path = {"params"}; ui_show_menu_generic(); }
*/

