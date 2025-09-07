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
//using Size    = Ui::Component::Button::Size;

/** Propiedades de creación. */
struct Props {
    const char* text    = nullptr;
    const char* iconId  = nullptr;
    Variant     variant = Variant::Primary;

    /** Acción declarativa opcional (si se define, tiene prioridad sobre onClick). */
    const char* action   = nullptr;

    /** Contexto opaco de usuario que se pasará a los callbacks. */
    void*       userData = nullptr;

    /** Callbacks a propagar al componente base. */
    Ui::Component::Button::Callbacks callbacks{};

    // Otros campos existentes de tu preset
    //Size        size     = Size::Normal;
    bool        disabled = false;
    bool        selected = false;
    bool        loading  = false;
    const char* hint     = nullptr;
    int         badgeCount = -1;
    bool        showDot    = false;
};

/** Manejador del preset. */
struct Handle {
    Ui::Component::Button::Handle base;
    lv_obj_t* badge   = nullptr;
    lv_obj_t* hintLbl = nullptr;
};

/*---------------------- API ----------------------*/

/** Crea el MenuButton. */
Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p);

/** Setters de conveniencia. */

bool setText    (Handle& h, UiThemeStyles& s, const char* text);
bool setIcon    (Handle& h, UiThemeStyles& s, const char* iconId);
bool setHint    (Handle& h, UiThemeStyles& s, const char* hint);
bool setBadge   (Handle& h, UiThemeStyles& s, int count, bool showDot);
bool setSelected(Handle& h, UiThemeStyles& s, bool v);
bool setDisabled(Handle& h, UiThemeStyles& s, bool v);
bool setVariant (Handle& h, UiThemeStyles& s, Variant v);
//bool setSize    (Handle& h, UiThemeStyles& s, Size sz);
void focus      (Handle& h);

} // namespace Ui::Preset::ButtonMenu
