#pragma once
#include "lvgl.h"
#include "ui/layout/ui_layout_scaffold.h"

/**
 * @file ui_view_welcome.h
 * @brief Vista simple de bienvenida para validar el scaffold y los componentes.
 */

namespace Ui {

/**
 * @brief Monta una vista de bienvenida dentro del Content del scaffold.
 * @param scaffold Referencia al scaffold ya construido (header/content/footer).
 */
void ui_view_welcome_mount(UiLayoutScaffold& scaffold);

} // namespace Ui
