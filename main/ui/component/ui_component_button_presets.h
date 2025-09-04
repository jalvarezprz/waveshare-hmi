#pragma once
/**
 * @file ui_component_button_presets.h
 * @brief Catalogo de presets de boton (nombres de negocio).
 * @defgroup ui_component_button_presets Presets Button
 * @ingroup ui_component_button
 * @{
 *
 * Descripcion:
 * Mapea casos de uso de la HMI (Si/No/Atras/Siguiente) a combinaciones
 * de Props ya preparadas. No toca ThemeStyles; delega en Button::create.
 *
 * Se ofrecen dos niveles de API:
 * 1) Completa: recibe UiThemeStyles y Button::Callbacks.
 * 2) Conveniencia: overloads que no requieren Styles y aceptan
 *    un callback simple (void(*)()) o ninguno.
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"

namespace Ui::Component::ButtonPresets {

/* ===== API completa (flexible) ===== */

/**
 * @brief Boton "Si" (Success + icono OK).
 */
Button::Handle Yes   (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb = {});

/**
 * @brief Boton "No" (Destructive + icono cerrar).
 */
Button::Handle No    (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb = {});

/**
 * @brief Boton "Atras" (Ghost + icono flecha izquierda).
 */
Button::Handle Back  (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb = {});

/**
 * @brief Boton "Siguiente" (Primary + icono flecha derecha).
 */
Button::Handle Next  (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb = {});

/* ===== Overloads de conveniencia (simples) ===== */

/** Sin callback */
Button::Handle Yes   (lv_obj_t* parent);
Button::Handle No    (lv_obj_t* parent);
Button::Handle Back  (lv_obj_t* parent);
Button::Handle Next  (lv_obj_t* parent);

/** Con callback simple (void(*)()) */
Button::Handle Yes   (lv_obj_t* parent, void (*onClick)(void));
Button::Handle No    (lv_obj_t* parent, void (*onClick)(void));
Button::Handle Back  (lv_obj_t* parent, void (*onClick)(void));
Button::Handle Next  (lv_obj_t* parent, void (*onClick)(void));

/** @} */ // end of group ui_component_button_presets
} // namespace Ui::Component::ButtonPresets
