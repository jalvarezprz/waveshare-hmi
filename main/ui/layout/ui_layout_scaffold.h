#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Crea (si no existe) y carga el Scaffold como pantalla activa. */
void ui_layout_scaffold_load(void);

/** Devuelve el contenedor de contenido donde se montan las vistas. */
lv_obj_t* ui_layout_scaffold_get_content(void);

/** Cambia el título de la barra superior. */
void ui_layout_scaffold_set_title(const char* title);

/** Habilita/deshabilita el botón Back. */
void ui_layout_scaffold_set_back_enabled(bool enabled);

/** Callback para el botón Back (desacoplado del router). */
typedef void (*ui_back_cb_t)(void* user);
void ui_layout_scaffold_set_back_handler(ui_back_cb_t cb, void* user);

void ui_layout_scaffold_show_back(bool show);

#ifdef __cplusplus
}
#endif
