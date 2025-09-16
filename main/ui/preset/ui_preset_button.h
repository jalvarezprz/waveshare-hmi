/**
 * @file ui_preset_button.h
 * @brief Presets de botones (Menu / Back) basados en Ui::Component::Button.
 * @ingroup ui_preset
 *
 * - Sin hardcode de colores/medidas: todo desde UiThemeStyles (tokens/styles).
 * - Composición (no herencia) con helpers internos para evitar duplicación.
 * - API pública minimal y estable para cada preset.
 */

#pragma once
#include "lvgl.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"
#include <cstdint>

namespace Ui::Preset {

/* -------------------------------------------------------------------------- */
/*  ButtonMenu                                                                */
/* -------------------------------------------------------------------------- */
namespace ButtonMenu {

/** Alias de variantes, delegadas al componente base. */
using Variant = Ui::Component::Button::Variant;

/**
 * @brief Propiedades del preset de botón de menú (icono + etiqueta en columna).
 */
struct Props {
    const char* text       = nullptr;        ///< Texto principal.
    const char* iconId     = "lv:settings";  ///< "lv:home", "lv:settings", ... o literal.
    Variant     variant    = Variant::Primary;
    bool        disabled   = false;
    bool        selected   = false;          ///< Modo toggle/checked inicial.
    bool        loading    = false;          ///< Estado de carga/spinner si el base lo soporta.

    // Contexto / router declarativo
    const char* action     = nullptr;        ///< p.ej., "NAV:/menu/parametros"
    void*       userData   = nullptr;

    // Adornos
    const char* hint       = nullptr;        ///< Texto auxiliar bajo la etiqueta.
    int         badgeCount = -1;             ///< >=0 muestra contador; -1 lo oculta.
    bool        showDot    = false;          ///< Muestra un punto en lugar de contador.

    Ui::Component::Button::Callbacks callbacks {}; ///< onClick, onLongPress, onToggle...
};

/** Handle del preset (raíz + elementos auxiliares). */
struct Handle {
    Ui::Component::Button::Handle base{}; ///< root/label/icon del botón base.
    lv_obj_t* hintLbl = nullptr;          ///< Etiqueta de hint (opcional).
    lv_obj_t* badge   = nullptr;          ///< Badge de contador (opcional).
};

/** Crea el botón de menú con layout vertical, hint y badge. */
Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p);

/** Setters específicos del preset. */
bool setText     (Handle& h, UiThemeStyles& s, const char* text);
bool setIcon     (Handle& h, UiThemeStyles& s, const char* iconId);
bool setHint     (Handle& h, UiThemeStyles& s, const char* hint);
bool setBadge    (Handle& h, UiThemeStyles& s, int count, bool showDot);
bool setSelected (Handle& h, UiThemeStyles& s, bool v);
bool setDisabled (Handle& h, UiThemeStyles& s, bool v);
bool setVariant  (Handle& h, UiThemeStyles& s, Variant v);

/** Mueve el foco al botón. */
void focus(Handle& h);

} // namespace ButtonMenu

/* -------------------------------------------------------------------------- */
/*  ButtonBack                                                                */
/* -------------------------------------------------------------------------- */
namespace ButtonBack {

/** Alias de variantes (recomendado Secondary o Ghost para “Back”). */
using Variant = Ui::Component::Button::Variant;

/** Props del botón “Atrás”. */
struct Props {
    const char* text       = "Atrás";
    const char* iconId     = "lv:left";             ///< Por defecto, símbolo de retroceso.
    Variant     variant    = Variant::Secondary;    ///< Apariencia por defecto para back.
    bool        disabled   = false;
    bool        loading    = false;

    // Routing / contexto
    const char* action     = "NAV:/back";           ///< Acción declarativa por defecto.
    void*       userData   = nullptr;

    Ui::Component::Button::Callbacks callbacks {};  ///< onClick, onLongPress, onToggle...
};

/** Handle del Back (solo el botón base). */
struct Handle {
    Ui::Component::Button::Handle base{};
};

/** Crea el botón "Back" (icono izquierdo + texto). */
Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p);

/** Setters básicos. */
bool setText    (Handle& h, UiThemeStyles& s, const char* text);
bool setIcon    (Handle& h, UiThemeStyles& s, const char* iconId);
bool setDisabled(Handle& h, UiThemeStyles& s, bool v);
bool setVariant (Handle& h, UiThemeStyles& s, Variant v);

/** Foco. */
void focus(Handle& h);

} // namespace ButtonBack

} // namespace Ui::Preset
