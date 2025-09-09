#pragma once
#include "lvgl.h"
#include "ui/menu/ui_menu_loader.h"  // ScreenSpecification

/** Construye la vista indicada por spec.view dentro de 'parent'.
 *  Devuelve el contenedor raíz creado (hijo de 'parent'), o nullptr si no reconoce la vista. */
lv_obj_t* ui_view_build(lv_obj_t* parent, const ScreenSpecification& spec);
