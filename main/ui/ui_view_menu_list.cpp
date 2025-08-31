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

static const char* TAG = "UI_MENU_JSON_TREE";

/* ======= forward del renderer genérico de lista (SE USA MÁS ABAJO) ======= */
static void ui_show_menu_generic();

/* ======= forward de la factoría de widgets de detalle ==================== */
static lv_obj_t* create_field_widget(lv_obj_t* parent, const cJSON* f);


/* ---------------- renderer detail (view:"detail") -------------- */
/*  Mejora contenida: dibuja widgets reales por field.widget_type (slider/dropdown).
    number/text siguen como etiquetas por ahora (cambio pequeño y seguro). */
static void ui_render_detail_from_node(const cJSON* node) {
    lv_obj_clean(lv_scr_act());

    const char* title_txt = cJSON_GetStringValue(cJSON_GetObjectItem(node, "title"));
    const cJSON* fields   = cJSON_GetObjectItem(node, "fields");

    lv_obj_t* cont = lv_obj_create(lv_scr_act());
    lv_obj_set_size(cont, 780, 440);
    lv_obj_center(cont);

    lv_obj_t* title = lv_label_create(cont);
    lv_label_set_text(title, title_txt ? title_txt : "Detalle");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    lv_obj_t* col = lv_obj_create(cont);
    lv_obj_set_size(col, 740, 300);
    lv_obj_align(col, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(col, 8, 0);
    lv_obj_set_style_pad_all(col, 8, 0);     // reemplazo de pad_col
    lv_obj_set_scroll_dir(col, LV_DIR_VER);

        // INICIO CAMBIO: bucle fields usando la factoría
        if (fields && cJSON_IsArray(fields)) {
            const cJSON* f = nullptr;
            cJSON_ArrayForEach(f, fields) {
                const char* label = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "label"));

                // Fila contenedora (una por campo)
                lv_obj_t* row = lv_obj_create(col);
                lv_obj_set_width(row, LV_PCT(100));
                lv_obj_set_style_pad_all(row, 6, 0);
                lv_obj_set_style_radius(row, 8, 0);
                lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
                lv_obj_set_style_pad_column(row, 8, 0);
                lv_obj_clear_flag(row, LV_OBJ_FLAG_SCROLLABLE);

                // Etiqueta (columna izquierda)
                lv_obj_t* l = lv_label_create(row);
                lv_label_set_text(l, label ? label : "-");
                lv_obj_set_width(l, 300);

                // Widget (columna derecha) creado por la factoría
                lv_obj_t* w = create_field_widget(row, f);
                if (w) {
                    // que el widget ocupe el resto de la línea
                    lv_obj_set_flex_grow(w, 1);
                }
            }
        } else {
            lv_obj_t* info = lv_label_create(col);
            lv_label_set_text(info, "(Sin campos definidos)");
        }
        // FIN CAMBIO


    lv_obj_t* back = lv_btn_create(cont);
    lv_obj_set_size(back, 120, 48);
    lv_obj_align(back, LV_ALIGN_BOTTOM_LEFT, 16, -16);
    lv_obj_t* bl = lv_label_create(back);
    lv_label_set_text(bl, "ATRAS");
    lv_obj_center(bl);
    lv_obj_add_event_cb(back, [](lv_event_t*){
        ui_show_menu_generic();
    }, LV_EVENT_CLICKED, nullptr);
}

// ---------------------------------------------------------------------------
// Factoría de widgets de un campo de "view:detail"
// Crea el control adecuado y aplica min/max/step/mock/options si están en el JSON.
// Soporta: slider, dropdown, number, text (read-only), button (placeholder).
// ---------------------------------------------------------------------------
static lv_obj_t* create_field_widget(lv_obj_t* parent, const cJSON* f) {
    if (!f) return nullptr;

    const char* widget_type = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "widget_type"));
    const char* unit        = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "unit"));
    const char* mock_str    = cJSON_GetStringValue(cJSON_GetObjectItem((cJSON*)f, "mock"));
    const cJSON* editableJS = cJSON_GetObjectItem((cJSON*)f, "editable");
    bool editable = cJSON_IsBool(editableJS) ? cJSON_IsTrue(editableJS) : false;

    // Helpers numéricos
    auto json_get_number = [&](const char* key, double def) -> double {
        const cJSON* it = cJSON_GetObjectItem((cJSON*)f, key);
        return (it && cJSON_IsNumber(it)) ? it->valuedouble : def;
    };

    // Formato de etiqueta "valor [unidad]" manejando enteros bonitos
    auto set_value_label = [&](lv_obj_t* lbl, double v){
        char buf[48];
        if (unit && *unit) {
            if (std::fabs(v - std::round(v)) < 0.0005) std::snprintf(buf, sizeof(buf), "%d %s", (int)std::llround(v), unit);
            else                                       std::snprintf(buf, sizeof(buf), "%.3f %s", v, unit);
        } else {
            if (std::fabs(v - std::round(v)) < 0.0005) std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(v));
            else                                       std::snprintf(buf, sizeof(buf), "%.3f", v);
        }
        lv_label_set_text(lbl, buf);
    };

    // ===================== SLIDER =====================
    if (widget_type && std::strcmp(widget_type, "slider") == 0) {
        // Leemos min/max/step/mock
        double min   = json_get_number("min",   0.0);
        double max   = json_get_number("max", 100.0);
        double step  = json_get_number("step",  1.0);
        double mock  = 0.0;
        if (mock_str) {
            char* endp=nullptr;
            mock = std::strtod(mock_str, &endp);
            if (endp==mock_str) mock = min; // si no parsea, usa min
        }

        // Decimales derivados de step (p.ej. 0.1 -> 1 decimal, 0.01 -> 2)
        int decimals = 0;
        double x = step;
        while (decimals < 6 && std::fabs(x - std::round(x)) > 1e-9) { x *= 10.0; decimals++; }
        int scale = (decimals > 0) ? (int)std::llround(std::pow(10.0, decimals)) : 1;

        // Rango entero escalado
        int32_t smin = (int32_t)std::llround(min  * scale);
        int32_t smax = (int32_t)std::llround(max  * scale);
        int32_t sval = (int32_t)std::llround(mock * scale);

        // Contenedor del control + etiqueta de valor
        lv_obj_t* wrap = lv_obj_create(parent);
        lv_obj_set_width(wrap, LV_PCT(100));
        lv_obj_set_flex_flow(wrap, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_pad_all(wrap, 0, 0);
        lv_obj_clear_flag(wrap, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_t* slider = lv_slider_create(wrap);
        lv_obj_set_flex_grow(slider, 1);
        lv_obj_set_height(slider, 22);           
        lv_slider_set_range(slider, smin, smax);
        lv_slider_set_value(slider, sval, LV_ANIM_OFF);

        lv_obj_t* lbl_val = lv_label_create(wrap);
        set_value_label(lbl_val, (double)sval / (double)scale);

        // Evento: arrastre
        lv_obj_add_event_cb(slider, +[](lv_event_t* e){
            lv_obj_t* slider = lv_event_get_target(e);
            if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED) return;

            // Guardamos los pointers que necesitamos como "user data" para no capturar lambdas complejas
            struct U {
                lv_obj_t* label;
                int       scale;
                double    min;
                double    max;
                double    step;
            };
            U* u = (U*)lv_event_get_user_data(e);
            if (!u) return;

            int sval = lv_slider_get_value(slider);
            double real = (double)sval / (double)u->scale;

            // Quantize al múltiplo de step dentro de [min,max]
            double t = (real - u->min) / (u->step > 0 ? u->step : 1.0);
            if (t < 0) t = 0;
            if (t > (u->max - u->min) / (u->step > 0 ? u->step : 1.0)) t = (u->max - u->min) / (u->step > 0 ? u->step : 1.0);
            double snapped = u->min + std::round(t) * (u->step > 0 ? u->step : 1.0);

            // Actualiza label
            char buf[48];
            if (u->label) {
                if (std::fabs(snapped - std::round(snapped)) < 0.0005) std::snprintf(buf, sizeof(buf), "%d", (int)std::llround(snapped));
                else                                                   std::snprintf(buf, sizeof(buf), "%.3f", snapped);
                // Si tienes unidad, puedes guardarla en U y añadirla aquí; para mantener compat, dejamos sólo número.
                lv_label_set_text(u->label, buf);
            }
        }, LV_EVENT_VALUE_CHANGED,
        // user data: empaquetamos label, scale y límites
        [lbl_val, scale, min, max, step](){
            struct U {
                lv_obj_t* label; int scale; double min, max, step;
            };
            static U u_static; // estático para evitar stack dangling (es 1 por llamada; suficiente aquí)
            u_static.label = lbl_val;
            u_static.scale = scale;
            u_static.min   = min;
            u_static.max   = max;
            u_static.step  = step;
            return (void*)&u_static;
        }());

        if (!editable) {
            lv_obj_add_state(slider, LV_STATE_DISABLED);
        }
        return wrap;
    }

    // ===================== DROPDOWN =====================
    if (widget_type && std::strcmp(widget_type, "dropdown") == 0) {
        lv_obj_t* dd = lv_dropdown_create(parent);

        // Construimos options si hay "options": [...]
        const cJSON* opts = cJSON_GetObjectItem((cJSON*)f, "options");
        if (opts && cJSON_IsArray(opts)) {
            // Convertimos a string con '\n'
            std::string opt_text;
            const cJSON* o = nullptr;
            bool first=true;
            cJSON_ArrayForEach(o, opts) {
                const char* s = cJSON_GetStringValue(o);
                if (!s) continue;
                if (!first) opt_text.push_back('\n');
                opt_text += s;
                first=false;
            }
            if (!opt_text.empty()) {
                lv_dropdown_set_options(dd, opt_text.c_str());
            }
        }

        // Valor inicial (si coincide con alguna opción, LVGL lo seleccionará)
        if (mock_str && *mock_str) {
            lv_dropdown_set_text(dd, mock_str);
        }

        if (!editable) {
            lv_obj_add_state(dd, LV_STATE_DISABLED);
        }
        return dd;
    }

    // ===================== NUMBER (read-only) =====================
    if (widget_type && std::strcmp(widget_type, "number") == 0) {
        lv_obj_t* lbl = lv_label_create(parent);
        if (mock_str) {
            std::string txt = mock_str;
            if (unit && *unit) { txt += " "; txt += unit; }
            lv_label_set_text(lbl, txt.c_str());
        } else {
            lv_label_set_text(lbl, "--");
        }
        return lbl;
    }

    // ===================== TEXT (read-only) =====================
    if (widget_type && std::strcmp(widget_type, "text") == 0) {
        lv_obj_t* lbl = lv_label_create(parent);
        lv_label_set_text(lbl, mock_str ? mock_str : "--");
        return lbl;
    }

    // ===================== BUTTON (placeholder) ==================
    if (widget_type && std::strcmp(widget_type, "button") == 0) {
        lv_obj_t* btn = lv_btn_create(parent);
        lv_obj_t* l   = lv_label_create(btn);
        lv_label_set_text(l, "OK");
        lv_obj_center(l);
        if (!editable) lv_obj_add_state(btn, LV_STATE_DISABLED);
        return btn;
    }

    // Desconocido: devolvemos un label neutro y logeamos
    ESP_LOGW(TAG, "Widget no soportado en detail: %s", widget_type ? widget_type : "(null)");
    lv_obj_t* fallback = lv_label_create(parent);
    lv_label_set_text(fallback, "--");
    return fallback;
}


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
                lv_label_set_long_mode(lbl_val, LV_LABEL_LONG_CLIP);
                lv_obj_set_width(lbl_val, LV_SIZE_CONTENT);
                lv_obj_align(lbl_val, LV_ALIGN_RIGHT_MID, -10, 0);

                /* Opcional: acotar ancho del label de título para evitar solapamientos */
                lv_obj_t* title_lbl = lv_obj_get_child(btn, 0);
                if (title_lbl) {
                    lv_label_set_long_mode(title_lbl, LV_LABEL_LONG_DOT);
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
                            ui_render_detail_from_node(node);
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
        }
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
        lv_label_set_text(l, "ATRAS");
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

/* (helpers opcionales)
void ui_build_tend_menu() { g_path = {"tend"}; ui_show_menu_generic(); }
void ui_build_params_menu() { g_path = {"params"}; ui_show_menu_generic(); }
*/
