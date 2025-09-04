#pragma once
/**
 * @file ui_view_menu_grid.h
 * @brief Declaración de la API para crear un menú tipo grid (tarjetas).
 * @ingroup ui_menu
 */

#include "lvgl.h"

/**
 * @brief Estructura de un ítem en el grid.
 */
struct UiMenuGridItem {
    const char* id;    ///< Identificador único (usado para navegación / callbacks).
    const char* text;  ///< Texto visible (etiqueta del mosaico).
    const char* icon;  ///< Icono opcional (símbolo LVGL o glifo de fuente).
};

/**
 * @brief Callbacks para interacción con el grid.
 */
struct UiMenuGridCallbacks {
    void (*onItem)(const char* id, void* ctx); ///< Callback cuando se pulsa un ítem.
    void* ctx;                                 ///< Contexto de usuario.
};

/**
 * @brief Crea un contenedor grid con los ítems especificados.
 *
 * @param parent   Objeto LVGL donde montar el grid (ej. contenedor del scaffold).
 * @param items    Array de ítems a mostrar.
 * @param count    Número de ítems.
 * @param cbs      Callbacks de interacción.
 * @param cols     Número de columnas deseadas (si GRID está habilitado).
 *
 * @return lv_obj_t* puntero al contenedor creado (grid/flex).
 */
lv_obj_t* ui_create_menu_grid(lv_obj_t* parent,
                              const UiMenuGridItem* items,
                              size_t count,
                              const UiMenuGridCallbacks* cbs,
                              uint8_t cols);

/**
 * @brief Muestra un grid de demo con ítems predefinidos.
 *
 * Útil para pruebas o como fallback si el JSON está vacío.
 */
void ui_menu_grid_demo_show();
