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
#include "ui/component/ui_component_button_presets.h"
#include "ui/actions/ui_actions.h"
#include "ui/view/ui_menu_render_grid.h"

// Forward declaration para que el handler pueda llamar a esta función
void ui_show_menu_generic();

// ==== Prototipos de helpers locales (declaración adelantada) ====
static void decorate_list_item_icons(lv_obj_t* btn, bool has_children);
static void adjust_list_item_title_layout(lv_obj_t* btn, bool has_value);
static void style_and_decorate_list_item(lv_obj_t* btn, bool has_children, bool has_value);

static const char* TAG = "UI_MENU_JSON_TREE";

/* ======= forward del renderer genérico de lista (SE USA MÁS ABAJO) ======= */
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

void ui_show_menu_generic()
{
    // Limpiamos pantalla actual (mantén esta semántica si el router realoca luego al scaffold)
    // lv_obj_clean(lv_scr_act());

    // Cargamos el árbol JSON completo
    cJSON* root = ui_menu_json_load();
    if (!root) return;

    // Localizamos, según la ruta actual, el array de hijos y el título
    cJSON* children = nullptr;
    const char* title_txt = "Menú";
    if (!Ui::Menu::ui_menu_nav_find(root, &children, &title_txt)) {
        ESP_LOGE(TAG, "Ruta inválida");
        cJSON_Delete(root);
        return;
    }

    // ─────────────────────────────────────────────────────────────────────
    // NUEVO: construir un nodo temporal { "title": ..., "items": [...] }
    // para alimentar al renderer grid.
    // Usamos cJSON_Duplicate(children, 1) (deep copy) para no modificar root.
    // ─────────────────────────────────────────────────────────────────────
    if (children && cJSON_IsArray(children)) {
        cJSON* tmp_node = cJSON_CreateObject();
        cJSON_AddStringToObject(tmp_node, "title", title_txt ? title_txt : "Menú");
        cJSON_AddItemToObject(tmp_node, "items", cJSON_Duplicate(children, /*recurse=*/1));

        // Render grid (monta en el contenedor configurado por ui_router_mount_set())
        ui_menu_render_grid_from_node(tmp_node);

        // Limpiamos temporales
        cJSON_Delete(tmp_node);
    } else {
        // Nodo sin hijos: usa la representación existente de "detalle"
        ui_menu_render_detail_from_node(root, ui_detail_back_bridge);
    }

    // Liberar el árbol original
    cJSON_Delete(root);

    // IMPORTANTE: terminamos aquí para no ejecutar el código de la lista anterior
    return;
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
