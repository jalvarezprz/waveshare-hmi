#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/** Despacha una acción. Soporta:
 *   - NAV:/screen/<id>  → navega y pinta la pantalla <id>
 *   - DO:/...           → (por ahora) loguea; no cambia de pantalla
 */
void ui_router_dispatch(const char* action);

/** Navega explícitamente a una pantalla por id (equivale a NAV:/screen/<id>). */
void ui_router_go_screen(const char* screen_id);

/** Vuelve a la pantalla anterior si existe. */
void ui_router_back(void);

#ifdef __cplusplus
}
#endif
