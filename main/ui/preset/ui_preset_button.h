#pragma once
/**
 * @file ui_preset_button.h
 * @brief Preset “MenuButton”: receta de alto nivel sobre Ui::Component::Button.
 *
 * Objetivo:
 *  - Botón de menú con anatomía icono (arriba) + texto (debajo) + hint opcional + badge.
 *  - Consumo exclusivo de UiThemeStyles (no Tokens directos).
 *  - Reutiliza el núcleo del componente Button (estados/variants/size/callbacks).
 *
 * Notas:
 *  - Este preset reconfigura el layout del Button a columna (icono encima del texto).
 *  - Badge en esquina superior derecha del root.
 *  - Si Props::action está definido, el botón despacha esa acción vía Router
 *    en lugar de usar callbacks.
 */

#include "lvgl.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"
#include <cstdint>

namespace Ui::Preset::ButtonMenu {

/** API pública del preset (reutiliza enums del componente Button). */
using Variant = Ui::Component::Button::Variant;
using Size    = Ui::Component::Button::Size;

/** Propiedades de creación. */
struct Props {
    const char* text       = nullptr;   ///< Etiqueta principal (1–2 líneas).
    const char* iconId     = nullptr;   ///< “lv:settings”, “lv:home”, … o texto simbólico.
    const char* hint       = nullptr;   ///< Subtítulo (opcional, 1 línea).
    int         badgeCount = -1;        ///< -1 sin badge; 0..99; >=100 → "99+".
    bool        showDot    = false;     ///< Si badgeCount < 0 y showDot=true → punto •.

    Variant     variant    = Variant::Primary;
    Size        size       = Size::Medium;
    bool        selected   = false;
    bool        disabled   = false;
    bool        loading    = false;

    /// Acción declarativa para Router; prioridad sobre callbacks.
    const char* action     = nullptr;

    // Callbacks del componente base (usados solo si action == nullptr)
    Ui::Component::Button::Callbacks callbacks{};
};

/** Manejador del preset. */
struct Handle {
    // Subyacente (componente Button)
    Ui::Component::Button::Handle base{};

    // Elementos extra del preset
    lv_obj_t* hintLbl = nullptr;  ///< Label de hint (opcional)
    lv_obj_t* badge   = nullptr;  ///< Badge / dot (opcional)
};

/*---------------------- API ----------------------*/

/** Crea el MenuButton. */
Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p);

/** Setters de conveniencia. */
bool setText     (Handle& h, UiThemeStyles& s, const char* text);
bool setIcon     (Handle& h, UiThemeStyles& s, const char* iconId);
bool setHint     (Handle& h, UiThemeStyles& s, const char* hint);
bool setBadge    (Handle& h, UiThemeStyles& s, int count, bool showDot);
bool setSelected (Handle& h, UiThemeStyles& s, bool v);
bool setDisabled (Handle& h, UiThemeStyles& s, bool v);
bool setVariant  (Handle& h, UiThemeStyles& s, Variant v);
bool setSize     (Handle& h, UiThemeStyles& s, Size sz);

/** Enfoque de accesibilidad. */
void focus(Handle& h);

} // namespace Ui::Preset::ButtonMenu
