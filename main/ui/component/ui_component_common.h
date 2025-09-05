#pragma once
/**
 * @file ui_component_common.h
 * @brief Utilidades comunes para componentes LVGL (header-only).
 * @defgroup ui_component_common Componentes: Common
 * @ingroup ui_component
 * @{
 *
 * Objetivo:
 * - Evitar duplicar helpers entre componentes (Button, Switch, Slider...).
 * - Mantener dependencia ZERO con Styles/Tokens (no hay includes del tema).
 * - Facilitar patrones de layout, eventos y user_data de LVGL.
 *
 * Notas:
 * - Todo es inline/header-only para no crear más TUs ni enlazado.
 * - No usar aquí Ui::Tokens ni Ui::UiThemeStyles para evitar ciclos.
 */

#include "lvgl.h"
#include <cstdint>
#include <functional>
#include <type_traits>

namespace Ui::Component::Common {

/* ───────────────────── Selectores tipados (part|state) ───────────────────── */

/**
 * @brief Selector tipado para combinar parte y estado sin warnings.
 * @param part  Parte (LV_PART_*)
 * @param state Estado (LV_STATE_*)
 */
static inline lv_style_selector_t sel(lv_part_t part, lv_state_t state) {
    return static_cast<lv_style_selector_t>(part | state);
}

/* ───────────────────────────── Layout helpers ───────────────────────────── */

/**
 * @brief Configura la altura de fila con ancho al 100% y alto contenido.
 * @param obj Objeto LVGL.
 * @param h   Altura mínima de la fila (px).
 */
static inline void set_row_height(lv_obj_t* obj, std::uint16_t h) {
    if (!obj) return;
    lv_obj_set_style_min_height(obj, static_cast<lv_coord_t>(h), LV_PART_MAIN);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_width(obj, LV_PCT(100));
}

/**
 * @brief Garantiza tamaño táctil mínimo (accesibilidad).
 * @param obj        Objeto LVGL.
 * @param min_touch  Mínimo recomendado (px). Por defecto 44 px.
 */
static inline void set_min_touch(lv_obj_t* obj, std::uint16_t min_touch = 44) {
    if (!obj) return;
    lv_coord_t w = lv_obj_get_width(obj);
    lv_coord_t h = lv_obj_get_height(obj);

    // Si el objeto tiene SIZE_CONTENT, establecer mínimo en estilos
    lv_obj_set_style_min_width (obj, static_cast<lv_coord_t>(min_touch), LV_PART_MAIN);
    lv_obj_set_style_min_height(obj, static_cast<lv_coord_t>(min_touch), LV_PART_MAIN);

    // Si ya tenía tamaño explícito menor, ajusta al mínimo
    if (w > 0 && w < min_touch) lv_obj_set_width (obj, static_cast<lv_coord_t>(min_touch));
    if (h > 0 && h < min_touch) lv_obj_set_height(obj, static_cast<lv_coord_t>(min_touch));
}

/**
 * @brief Configura flujo flex como fila.
 * @param parent Contenedor LVGL.
 * @param main   Alineación principal (por defecto START).
 * @param cross  Alineación cruzada (por defecto CENTER).
 * @param track  Alineación de líneas (por defecto CENTER).
 * @param gap_px Espacio entre hijos (px).
 */
static inline void flex_row(lv_obj_t* parent,
                            lv_flex_align_t main = LV_FLEX_ALIGN_START,
                            lv_flex_align_t cross = LV_FLEX_ALIGN_CENTER,
                            lv_flex_align_t track = LV_FLEX_ALIGN_CENTER,
                            std::uint16_t gap_px = 0) {
    if (!parent) return;
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(parent, main, cross, track);
    if (gap_px) lv_obj_set_style_pad_gap(parent, static_cast<lv_coord_t>(gap_px), LV_PART_MAIN);
}

/**
 * @brief Configura flujo flex como columna.
 * @see flex_row
 */
static inline void flex_column(lv_obj_t* parent,
                               lv_flex_align_t main = LV_FLEX_ALIGN_START,
                               lv_flex_align_t cross = LV_FLEX_ALIGN_CENTER,
                               lv_flex_align_t track = LV_FLEX_ALIGN_CENTER,
                               std::uint16_t gap_px = 0) {
    if (!parent) return;
    lv_obj_set_flex_flow(parent, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(parent, main, cross, track);
    if (gap_px) lv_obj_set_style_pad_gap(parent, static_cast<lv_coord_t>(gap_px), LV_PART_MAIN);
}

/* ─────────────────────────── User data tipado ───────────────────────────── */

/**
 * @brief Guarda un puntero tipado en user_data del objeto.
 * @tparam T Tipo del puntero a guardar.
 * @param obj Objeto LVGL.
 * @param ptr Puntero a T (no se gestiona propiedad).
 */
template <class T>
static inline void set_user_ptr(lv_obj_t* obj, T* ptr) {
    if (!obj) return;
    lv_obj_set_user_data(obj, static_cast<void*>(ptr));
}

/**
 * @brief Recupera un puntero tipado desde user_data del objeto.
 * @tparam T Tipo del puntero almacenado.
 */
template <class T>
static inline T* get_user_ptr(lv_obj_t* obj) {
    if (!obj) return nullptr;
    return static_cast<T*>(lv_obj_get_user_data(obj));
}

/* ───────────────────────────── Event helpers ────────────────────────────── */

/** Callback de evento genérico (recibe lv_event_t*). */
using EventCb = std::function<void(lv_event_t*)>;

/**
 * @brief Registra un callback de evento std::function en un objeto.
 * @param obj   Objeto LVGL.
 * @param code  Código de evento (LV_EVENT_*).
 * @param cb    Lambda/callable que recibe lv_event_t*.
 *
 * Nota: El std::function se captura por valor en el user_data del evento,
 * por lo que debe vivir al menos tanto como el objeto o usarse una lambda sin estado.
 */
static inline void add_event(lv_obj_t* obj, lv_event_code_t code, EventCb cb) {
    if (!obj || !cb) return;
    // Copiamos cb en heap para que viva tras la llamada
    auto* holder = new EventCb(std::move(cb));
    lv_obj_add_event_cb(obj,
        [](lv_event_t* e) {
            auto* fun = static_cast<EventCb*>(lv_event_get_user_data(e));
            if (fun && *fun) (*fun)(e);
        },
        code,
        holder);

    // Liberación automática cuando se borre el objeto
    lv_obj_add_event_cb(obj,
        [](lv_event_t* e) {
            // Este handler se llama en DELETE
            auto* fun = static_cast<EventCb*>(lv_event_get_user_data(e));
            delete fun;
        },
        LV_EVENT_DELETE,
        holder);
}

/** Adaptador simple para clicks: void() */
using ClickFn = std::function<void()>;

/**
 * @brief Añade callback de click corto (LV_EVENT_CLICKED) a un objeto.
 */
static inline void add_click(lv_obj_t* obj, ClickFn cb) {
    if (!obj || !cb) return;
    add_event(obj, LV_EVENT_CLICKED, [cb = std::move(cb)](lv_event_t*) { cb(); });
}

/* ─────────────────────────── Árbol y utilidades ─────────────────────────── */

/**
 * @brief Elimina un hijo si no es nullptr (no falla si ya está borrado).
 */
static inline void safe_del(lv_obj_t*& child) {
    if (child) { lv_obj_del(child); child = nullptr; }
}

/**
 * @brief Mueve un hijo a un índice si el índice es válido.
 */
static inline void move_to_index_safe(lv_obj_t* child, std::uint32_t idx) {
    if (!child) return;
    lv_obj_t* parent = lv_obj_get_parent(child);
    if (!parent) return;
    std::uint32_t cnt = lv_obj_get_child_cnt(parent);
    if (idx > cnt) idx = cnt;
    lv_obj_move_to_index(child, idx);
}

/** @} */ // end of group ui_component_common
} // namespace Ui::Component::Common
