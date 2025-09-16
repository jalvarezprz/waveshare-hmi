#pragma once
/**
 * @file ui_component_switch.h
 * @brief Componente Switch (toggle) para LVGL: API declarativa y callbacks tipados.
 * @defgroup ui_component_switch Componente Switch
 * @ingroup ui_component
 * @{
 *
 * Descripcion:
 * Componente de conmutacion con labels opcionales a izquierda y derecha.
 * Construye un contenedor raiz (root) con flex-row: [labelLeft] [switch] [labelRight].
 * Expone un Handle con punteros a root, al propio lv_switch y a las etiquetas.
 *
 * No depende de Tokens directamente; consume ThemeStyles y Common helpers.
 */

#include "lvgl.h"
#include <cstdint>
#include <functional>

namespace Ui { struct UiThemeStyles; }

namespace Ui::Component::Switch {

/** Variante visual del switch. */
enum class Variant : std::uint8_t {
    Default,      ///< Usa color primario para activo; superficie para inactivo.
    Success,      ///< Verde para activo.
    Destructive   ///< Rojo para activo.
};

/** Propiedades declarativas del switch. */
struct Props {
    bool checked      = false;          ///< Estado inicial.
    bool enabled      = true;           ///< Habilitado / deshabilitado.
    Variant variant   = Variant::Default;
    const char* labelLeft  = nullptr;   ///< Texto opcional a la izquierda.
    const char* labelRight = nullptr;   ///< Texto opcional a la derecha.
    void* userData    = nullptr;        ///< Payload opcional del cliente.
};

/** Manejador del switch creado. */
struct Handle {
    lv_obj_t* root  = nullptr;  ///< Contenedor raiz (flex row).
    lv_obj_t* sw    = nullptr;  ///< Objeto lv_switch.
    lv_obj_t* lblL  = nullptr;  ///< Etiqueta izquierda (opcional).
    lv_obj_t* lblR  = nullptr;  ///< Etiqueta derecha (opcional).
};

/** Callback de cambio de estado. */
using ChangeCb = std::function<void(Handle&, bool checked, void* userData)>;

/** Conjunto de callbacks. */
struct Callbacks {
    ChangeCb onChange = nullptr;
};

/**
 * @brief Crea un switch con labels opcionales.
 * @param parent Contenedor padre.
 * @param styles ThemeStyles activo.
 * @param p      Propiedades del switch.
 * @param cb     Callbacks (onChange).
 * @return Handle con punteros a root, switch y etiquetas.
 */
Handle create(lv_obj_t* parent, Ui::UiThemeStyles& styles, const Props& p, const Callbacks& cb = {});

/** Cambia estado checked. */
void setChecked(Handle& h, bool v);

/** Lee estado checked. */
bool isChecked(const Handle& h);

/** Habilita o deshabilita el switch. */
void setEnabled(Handle& h, bool v);

/** Cambia labels (cadenas pueden ser nullptr para ocultar). */
void setLabels(Handle& h, Ui::UiThemeStyles& s, const char* leftTxt, const char* rightTxt);

/** @} */ // end group
} // namespace Ui::Component::Switch
