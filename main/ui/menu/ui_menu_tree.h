// ui/menu/ui_menu_tree.h
#pragma once
#include <cstddef>

/**
 * @file ui_menu_tree.h
 * @brief Fuentes de pantallas (ScreenSpecification en JSON) embebidas y utilidades
 *        para recuperar el JSON de una pantalla por id, sin escaneos ni índices.
 *
 * Contrato (JSON):
 *  - ScreenSpecification: { schemaVersion, id, view, title, elements[], layout? }
 *  - ElementSpecification: { id, title, icon?, action?, enabled?, visible?, badge?, confirm?, children? }
 */

/* === IDs canónicos de pantallas embebidas === */
inline constexpr const char* UI_SCREEN_ID_MAIN   = "main";
inline constexpr const char* UI_SCREEN_ID_TEND   = "tend";
inline constexpr const char* UI_SCREEN_ID_INFO   = "info";
inline constexpr const char* UI_SCREEN_ID_PARAMS = "params";
inline constexpr const char* UI_SCREEN_ID_HW     = "hw";
inline constexpr const char* UI_SCREEN_ID_DDC    = "ddc";
inline constexpr const char* UI_SCREEN_ID_BUS    = "bus";
inline constexpr const char* UI_SCREEN_ID_PANEL = "panel";

/* === JSON embebido por pantalla (extern: definido en el .cpp) === */
extern const char UI_SCREEN_MAIN[];    // main
extern const char UI_SCREEN_TEND[];    // tend
extern const char UI_SCREEN_INFO[];    // info
extern const char UI_SCREEN_PARAMS[];  // params
extern const char UI_SCREEN_HW[];      // hw
extern const char UI_SCREEN_DDC[];     // ddc
extern const char UI_SCREEN_BUS[];     // bus
extern const char UI_SCREEN_PANEL[];   // panel

/**
 * @brief Devuelve el JSON de la pantalla solicitada por id, o nullptr si no existe.
 *        No realiza escaneos ni lecturas de FS: solo consulta el catálogo embebido.
 */
const char* ui_menu_tree_get(const char* screen_id);

/**
 * @brief Devuelve el array estático de ids disponibles y su tamaño.
 *        Útil para depuración o tests (no modificar el array devuelto).
 */
const char* const* ui_menu_tree_ids(size_t* out_count);

/** @brief Número de pantallas embebidas. */
size_t ui_menu_tree_count();

/** @brief true si existe una pantalla embebida con ese id. */
bool ui_menu_tree_exists(const char* screen_id);

/* --- (Opcional) Alias legacy si necesitas compat temporal con código antiguo --- */
// extern const char ui_menu_json_tree[]; // <- si lo necesitas, descomenta y mapea a UI_SCREEN_MAIN en el .cpp
