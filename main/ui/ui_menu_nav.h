/**
 * @file ui_menu_nav.h
 * @brief Gestión de ruta de navegación del menú y búsquedas en el árbol JSON.
 * @ingroup ui_menu
 */

#pragma once

#include <vector>
#include <string>

struct cJSON;  ///< forward decl. (cJSON es C)

namespace Ui { namespace Menu {

/**
 * @brief Devuelve la ruta actual (lista de IDs) desde la raíz hasta el nodo activo.
 */
const std::vector<std::string>& ui_menu_nav_path();

/**
 * @brief Limpia la ruta de navegación (vuelve a la raíz).
 */
void ui_menu_nav_clear();

/**
 * @brief Empuja un ID a la ruta de navegación (baja un nivel).
 */
void ui_menu_nav_push(const std::string& id);

/**
 * @brief Sube un nivel en la ruta. No sube si ya estás en raíz.
 * @return true si se ha hecho pop; false si ya estaba en raíz.
 */
bool ui_menu_nav_pop();

/**
 * @brief Indica si un ID de nivel raíz es protegido (requiere PIN).
 */
bool ui_menu_nav_is_protected_root_id(const char* id);

/**
 * @brief Localiza, según la ruta actual, el array de hijos y el título del nodo.
 * @param root Árbol cJSON del menú (propiedad del llamante).
 * @param out_array Salida: puntero al array "menu" o "items" del nodo actual, o nullptr si es hoja.
 * @param out_title Salida: título del nodo actual (cadena interna del cJSON).
 * @return true si la ruta es válida; false si no se encontró.
 */
bool ui_menu_nav_find(cJSON* root, cJSON** out_array, const char** out_title);

/**
 * @brief Busca un nodo hoja con un ID concreto, respetando la ruta actual como prefijo.
 * @param root Árbol cJSON del menú.
 * @param leaf_id ID del elemento hoja a buscar en el nivel actual.
 * @return puntero al nodo cJSON si existe; nullptr si no se encuentra.
 */
const cJSON* ui_menu_nav_find_leaf(const cJSON* root, const char* leaf_id);

}} // namespace Ui::Menu
