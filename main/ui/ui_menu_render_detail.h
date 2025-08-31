/**
 * @file ui_menu_render_detail.h
 * @brief Renderiza una vista de detalle a partir de un nodo cJSON del árbol de menú.
 * @ingroup ui_menu
 */

#pragma once

struct cJSON;  ///< forward decl. (cJSON es C)

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Construye la pantalla de detalle para un nodo de menú con `view:"detail"`.
 *
 * @param node Nodo cJSON con campos "title" y, opcionalmente, "fields":[...].
 * @param on_back Callback a invocar cuando el usuario pulsa el botón "ATRÁS".
 *                Puede ser `NULL` si no se desea manejar esa acción.
 *
 * @note La función limpia la pantalla activa (lv_scr_act) y crea todos los widgets.
 */
void ui_menu_render_detail_from_node(const cJSON* node, void (*on_back)(void));

#ifdef __cplusplus
}
#endif
