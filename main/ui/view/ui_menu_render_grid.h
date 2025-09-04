#pragma once
/**
 * @file ui_menu_render_grid.h
 * @brief Renderizador de menú en formato grid a partir de un nodo JSON.
 * @ingroup ui_menu
 */

#include "lvgl.h"

// Forward declaration de cJSON para no arrastrar dependencias en el header.
// La implementación (.cpp) sí debe #include "cJSON.h".
extern "C" { struct cJSON; }

/**
 * @brief Renderiza un menú grid a partir de un nodo JSON.
 *
 * Estructura JSON aceptada (flexible):
 * {
 *   "title": "Menú principal",        // opcional; también puede ser "name"
 *   "items": [                         // o "children"
 *     { "id": "menu_1", "title": "Ajustes", "icon": "⚙", ... },
 *     { "key": "menu_2", "text": "Alarmas", "symbol": "!", ... },
 *     { "path": "menu_3", "name": "Red", "icon": "\uf1eb", ... }
 *   ]
 * }
 *
 * Campos tolerados por ítem:
 *   - Identificador: "id" | "path" | "key"
 *   - Texto visible: "title" | "text" | "name"
 *   - Icono opcional: "icon" | "symbol"  (recomendado: símbolos LVGL o glifos de tu fuente)
 *
 * El renderer monta el grid en el contenedor devuelto por ui_router_mount_get()
 * (si existe), o en lv_scr_act() en su defecto. No destruye el scaffold: limpia
 * únicamente el contenedor de contenido antes de dibujar.
 *
 * @param node  Nodo JSON del menú a renderizar (no se libera dentro de la función).
 */
void ui_menu_render_grid_from_node(const cJSON* node);
