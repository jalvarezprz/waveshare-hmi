#pragma once
/**
 * @file ui_component_slider.h
 * @brief Componente Slider (rango entero) para LVGL, con label opcional de valor.
 * @defgroup ui_component_slider Componente Slider
 * @ingroup ui_component
 * @{
 *
 * Descripción:
 * Componente para seleccionar un valor entero dentro de un rango [min,max].
 * Construye un contenedor raíz (root) con flex-row: [slider] [valueLabel opcional].
 * Expone un Handle con punteros a root, slider y label de valor.
 *
 * No toca Tokens directamente; consume UiThemeStyles y helpers de Common si fuera necesario.
 */

#include "lvgl.h"
#include <cstdint>
#include <functional>

namespace Ui { struct UiThemeStyles; }

namespace Ui::Component::Slider {

/** Variante visual del slider (colores para indicador y knob). */
enum class Variant : std::uint8_t {
    Default,      ///< Usa color primario.
    Success,      ///< Verde.
    Destructive   ///< Rojo (alarma / peligro).
};

/** Propiedades declarativas del slider. */
struct Props {
    int32_t min        = 0;         ///< Valor mínimo (LVGL enteros).
    int32_t max        = 100;       ///< Valor máximo.
    int32_t value      = 0;         ///< Valor inicial.
    bool    enabled    = true;      ///< Habilitado/Deshabilitado.
    Variant variant    = Variant::Default;

    bool    showValue  = true;      ///< Mostrar label con el valor a la derecha.
    const char* suffix = nullptr;   ///< Sufijo opcional del valor (p. ej., "°C", "%").
    int32_t step       = 0;         ///< Paso opcional (>0). Si se indica, se redondea en cambios.
};

/** Manejador de objetos del slider. */
struct Handle {
    lv_obj_t* root   = nullptr; ///< Contenedor raíz (flex row).
    lv_obj_t* slider = nullptr; ///< Objeto lv_slider.
    lv_obj_t* valueL = nullptr; ///< Label del valor (opcional).
};

/** Callbacks del slider. */
using ChangeCb  = std::function<void(Handle&, int32_t value, void* user)>;
using ReleaseCb = std::function<void(Handle&, int32_t value, void* user)>;

struct Callbacks {
    ChangeCb  onChange  = nullptr;  ///< Se llama en LV_EVENT_VALUE_CHANGED (mientras se arrastra).
    ReleaseCb onRelease = nullptr;  ///< Se llama en LV_EVENT_RELEASED (al soltar).
    void*     userData  = nullptr;  ///< Payload del usuario (no gestionado).
};

/**
 * @brief Crea un slider con label opcional de valor.
 * @param parent Contenedor padre.
 * @param s      Styles activos.
 * @param p      Propiedades declarativas.
 * @param cb     Callbacks (opcionales).
 * @return Handle con punteros a root, slider y label de valor.
 */
Handle create(lv_obj_t* parent, Ui::UiThemeStyles& s, const Props& p, const Callbacks& cb = {});

/* =============================== Mutadores =============================== */

/** Fija el valor (clampa a [min,max], aplica step si procede y actualiza label). */
void setValue(Handle& h, int32_t v);

/** Lee el valor actual. */
int32_t getValue(const Handle& h);

/** Cambia el rango. Si value queda fuera, se clampa. */
void setRange(Handle& h, int32_t min, int32_t max);

/** Habilita/Deshabilita el slider. */
void setEnabled(Handle& h, bool enabled);

/** Cambia la visibilidad del label de valor. */
void setShowValue(Handle& h, bool show);

/** Cambia el sufijo del valor (no se copia; mantener vivo el literal). */
void setSuffix(Handle& h, const char* suffix);

/** Cambia el paso (0 = sin cuantización). */
void setStep(Handle& h, int32_t step);

/** Cambia la variante cromática. */
void setVariant(Handle& h, Ui::UiThemeStyles& s, Variant v);

/** @} */ // end group
} // namespace Ui::Component::Slider
