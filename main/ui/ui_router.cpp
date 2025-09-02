#include "ui_router.h"
#include "lvgl.h"
#include "esp_log.h"
#include "ui_view_menu_list.h"   // builders de pantallas (sin cambios)
#include "ui/ui_router_mount.h"

static const char* TAG = "UI_ROUTER";

static bool is_valid_container(lv_obj_t* obj) {
    return obj && lv_obj_is_valid(obj);
}

/**
 * @brief Elimina todos los hijos de un contenedor (seguro con LVGL).
 */
static void clear_container(lv_obj_t* container) {
    if (!is_valid_container(container)) return;
    while (lv_obj_get_child_cnt(container) > 0) {
        lv_obj_del(lv_obj_get_child(container, 0));
    }
}

/**
 * @brief Reubica a @p parent todos los hijos nuevos creados en la pantalla activa desde un índice dado.
 *
 * No hace nada si @p parent no es válido o si parent pertenece al screen activo (reparent innecesario).
 */
static void reparent_new_children_to(lv_obj_t* parent, uint32_t start_idx) {
    if (!is_valid_container(parent)) return;

    lv_obj_t* scr = lv_scr_act();

    // Si el parent ya cuelga del screen activo, no reparentamos.
    // (En nuestro caso parent suele ser el content del scaffold, que ya cuelga del scr)
    if (lv_obj_get_screen(parent) == scr) {
        // Aun así, si los builders pusieron cosas directamente en 'scr', podemos moverlas.
        uint32_t after_total = lv_obj_get_child_cnt(scr);
        if (after_total <= start_idx) return;

        // Recolecta y mueve siempre el hijo en posición 'start_idx' hasta agotar los nuevos.
        // Cada set_parent compacta la lista; por eso tomamos siempre 'start_idx'.
        for (uint32_t idx = start_idx; idx < after_total; ++idx) {
            lv_obj_t* child = lv_obj_get_child(scr, start_idx);
            if (!child) break;
            if (child == parent) continue; // seguridad
            lv_obj_set_parent(child, parent);
        }
        return;
    }

    // Si parent no cuelga del screen activo, no reparentar (evita crashes tras lv_scr_load()).
    // En ese caso, dejamos los hijos nuevos donde están (scr actual).
}

void ui_router_go(UiScreen s) {
    // Obtén el contenedor preferido (content del scaffold) o fallback a lv_scr_act() si no es usable
    lv_obj_t* parent = ui_router_mount_get_or(lv_scr_act());
    lv_obj_t* scr    = lv_scr_act(); // screen activo en este instante

    ESP_LOGI(TAG, "Navegar a pantalla %d", static_cast<int>(s));

    // Si el parent no es válido o ya no pertenece al screen activo, usa el scr como destino de limpieza.
    lv_obj_t* target_to_clear = (is_valid_container(parent) && lv_obj_get_screen(parent) == scr)
                                ? parent
                                : scr;

    // Limpia únicamente el contenedor objetivo (si es el scr, no afectará al scaffold de otra pantalla)
    clear_container(target_to_clear);

    // Snapshot de hijos del screen activo ANTES de construir
    uint32_t before_cnt = lv_obj_get_child_cnt(scr);

    // Construir vista con los builders existentes (sin cambiar su firma)
    switch (s) {
        case UiScreen::MAIN_MENU:
            ui_build_main_menu();
            break;
        case UiScreen::INFO_MENU:
            ui_build_info_menu();
            break;
        default:
            break;
    }

    // Re-ubicar sólo si 'parent' es válido y pertenece al screen activo
    if (is_valid_container(parent) && lv_obj_get_screen(parent) == scr) {
        reparent_new_children_to(parent, before_cnt);
    }
}
