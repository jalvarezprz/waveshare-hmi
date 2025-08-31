/**
 * @file ui_menu_json_utilities.h
 * @brief Utilidades para cargar e inspeccionar el árbol de menú embebido en JSON.
 *
 * Este módulo encapsula la carga del recurso `ui_menu_json_tree` (cadena JSON
 * embebida) y ofrece utilidades de diagnóstico. El llamante es responsable
 * de liberar el árbol cJSON obtenido con `cJSON_Delete()`.
 *
 * @ingroup ui_menu
 */

#pragma once

/// Declaración adelantada para evitar incluir cJSON.h en todos los consumidores.
struct cJSON;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Carga y parsea el menú embebido (recurso JSON).
 *
 * Lee el contenido de `ui_menu_json_tree` y lo parsea con cJSON.
 *
 * @return puntero a la raíz cJSON si tiene éxito, o `nullptr` en caso de error.
 * @post Si el retorno no es `nullptr`, el llamante debe ejecutar `cJSON_Delete(ptr)`.
 *
 * @note Esta función no realiza validación de esquema; solo parsea.
 * @warning El árbol devuelto es mutable desde cJSON; no lo compartas entre hilos sin protección.
 */
cJSON* ui_menu_json_load(void);

/**
 * @brief Imprime en log un resumen de la raíz del menú.
 *
 * Recorre el array `"menu"` (si existe) e informa de `id` y `title` de cada entrada.
 * Útil para diagnóstico durante el desarrollo.
 *
 * @post No devuelve valor. Libera internamente el árbol cJSON temporal.
 */
void ui_menu_json_print(void);

#ifdef __cplusplus
}
#endif
