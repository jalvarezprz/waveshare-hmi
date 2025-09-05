#pragma once
/**
 * @file ui_component_label.h
 * @brief Componente Label (Title/Body/Caption) para LVGL.
 * @defgroup ui_component_label Componente Label
 * @ingroup ui_component
 * @{
 */

#include "lvgl.h"
#include <cstdint>

namespace Ui { struct UiThemeStyles; }

namespace Ui::Component::Label {

/** Variantes tipográficas disponibles. */
enum class Variant : std::uint8_t {
    Title,
    Body,
    Caption,
};

/** Propiedades declarativas. */
struct Props {
    const char* text   = nullptr;
    Variant     variant= Variant::Body;
    lv_text_align_t align = LV_TEXT_ALIGN_LEFT;  ///< Alineación del texto
};

/** Manejador del label creado. */
struct Handle {
    lv_obj_t* root = nullptr; ///< Es un lv_label.
};

/**
 * @brief Crea un label con estilo desde ThemeStyles.
 * @param parent Contenedor padre.
 * @param s      ThemeStyles activo.
 * @param p      Propiedades (texto, variante, alineación).
 * @return Handle con el lv_label creado.
 */
Handle create(lv_obj_t* parent, Ui::UiThemeStyles& s, const Props& p);

/** Cambia el texto (crea si es nullptr). */
void setText(Handle& h, const char* txt);

/** Cambia la variante (Title/Body/Caption). */
void setVariant(Handle& h, Ui::UiThemeStyles& s, Variant v);

/** Cambia la alineación del texto. */
void setAlign(Handle& h, lv_text_align_t align);

/** @} */ // end group
} // namespace Ui::Component::Label
