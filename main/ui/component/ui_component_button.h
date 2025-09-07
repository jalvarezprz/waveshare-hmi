#pragma once
/**
 * @file ui_component_button.h
 * @brief Componente Button: API pública declarativa para crear y gestionar botones LVGL.
 * @defgroup ui_component_button Componente Button
 * @ingroup ui_component
 * @{
 *
 * Descripción:
 * Capa delgada sobre lv_btn que construye un botón con texto e icono opcional,
 * aplica estilos desde ThemeStyles y expone una API declarativa mediante Props
 * (propiedades) y Callbacks (eventos).
 *
 * - No accede a Tokens directamente; consume ThemeStyles.
 * - Los estados visuales (pressed/focused/disabled) los calcula ThemeStyles.
 *
 * Ejemplo de uso:
 * \code{.cpp}
 * Ui::themeInitOnce();
 * auto& s = Ui::getThemeStyles();
 *
 * using namespace Ui::Component::Button;
 * Props p{
 *   .text    = "Guardar",
 *   .icon    = LV_SYMBOL_OK,
 *   .variant = Variant::Primary,
 *   .iconPos = IconPos::Left,
 *   .action  = "NAV:/save"   // acción declarativa
 * };
 * Handle h = create(parent, s, p);
 * \endcode
 */

#include "lvgl.h"
#include <cstdint>
#include <functional>

namespace Ui { struct UiThemeStyles; }

namespace Ui::Component::Button {

/**
 * @brief Variantes visuales del botón (recetas de ThemeStyles).
 */
enum class Variant : std::uint8_t {
    Primary,      ///< Acción principal.
    Secondary,    ///< Acción secundaria.
    Ghost,        ///< Botón con fondo transparente y borde ligero.
    Destructive,  ///< Acción destructiva (error).
    Success,      ///< Confirmación o éxito.
    Warning       ///< Advertencia o precaución.
};

/**
 * @brief Tallas del botón.
 */
enum class Size : std::uint8_t {
    Small,   ///< Compacto (si procede).
    Medium,  ///< Por defecto.
    Large    ///< Aumenta dimensiones según tokens.
};

/**
 * @brief Posición del icono respecto al texto.
 */
enum class IconPos : std::uint8_t {
    None,   ///< Sin icono.
    Left,   ///< Icono a la izquierda.
    Right,  ///< Icono a la derecha.
    Only    ///< Solo icono (sin texto).
};

/**
 * @brief Propiedades declarativas del botón.
 */
struct Props {
    const char*   text      = nullptr;   ///< Texto (literal o clave i18n).
    const char*   icon      = nullptr;   ///< Código/icono (p. ej., LV_SYMBOL_*).
    Variant       variant   = Variant::Primary; ///< Apariencia base.
    Size          size      = Size::Medium;     ///< Talla visual.
    IconPos       iconPos   = IconPos::Left;    ///< Disposición icono/texto.
    bool          enabled   = true;             ///< Habilitado / deshabilitado.
    bool          toggle    = false;            ///< Conmutador (checkable).
    bool          checked   = false;            ///< Estado inicial si toggle=true.
    bool          loading   = false;            ///< Modo "cargando" (bloquea click).
    void*         userData  = nullptr;          ///< Payload del cliente (opcional).

    /// Acción declarativa (si se rellena, tiene prioridad sobre callbacks).
    const char*   action    = nullptr;
};

/**
 * @brief Manejador del botón creado.
 */
struct Handle {
    lv_obj_t* root   = nullptr;  ///< Objeto raíz (lv_btn).
    lv_obj_t* label  = nullptr;  ///< Etiqueta de texto (puede ser null).
    lv_obj_t* icon   = nullptr;  ///< Etiqueta de icono (puede ser null).
};

/** @name Callbacks tipados
 *  Eventos de interacción expuestos por el componente.
 *  @{ */
using ClickCb  = std::function<void(Handle&, void* userData)>;
using LongCb   = std::function<void(Handle&, void* userData)>;
using ToggleCb = std::function<void(Handle&, bool checked, void* userData)>;
/** @} */

/**
 * @brief Conjunto de callbacks opcionales.
 */
struct Callbacks {
    ClickCb  onClick  = nullptr;  ///< Click corto.
    LongCb   onLong   = nullptr;  ///< Pulsación larga.
    ToggleCb onToggle = nullptr;  ///< Cambio de estado, si toggle=true.
};

/**
 * @brief Crea un botón LVGL configurado según Props.
 * @param parent  Contenedor LVGL padre.
 * @param styles  ThemeStyles activo (recetas y snapshot de tokens).
 * @param p       Propiedades del botón.
 * @param cb      Callbacks opcionales.
 * @return Handle con punteros LVGL del botón.
 *
 * Prioridad:
 * - Si p.action != nullptr → se despacha al Router.
 * - Si no hay action → se usan los callbacks.
 */
Handle create(lv_obj_t* parent, UiThemeStyles& styles, const Props& p, const Callbacks& cb = {});

/**
 * @brief Activa o desactiva el botón.
 */
void setEnabled (Handle& h, UiThemeStyles& styles, bool enabled);

/**
 * @brief Cambia el estado conmutado (si toggle=true).
 */
void setChecked (Handle& h, UiThemeStyles& styles, bool checked);

/**
 * @brief Activa o desactiva el modo "cargando".
 */
void setLoading (Handle& h, UiThemeStyles& styles, bool loading);

/**
 * @brief Cambia el texto del botón (crea label si no existe).
 */
void setText    (Handle& h, UiThemeStyles& styles, const char* txt);

/**
 * @brief Cambia o añade el icono y su posición.
 */
void setIcon    (Handle& h, UiThemeStyles& styles, const char* icon, IconPos pos);

/**
 * @brief Consulta el estado conmutado del botón.
 */
bool isChecked(const Handle& h);

/**
 * @brief Lleva el foco de entrada al botón.
 */
void focus(Handle& h);

/** @} */ // end of group ui_component_button
} // namespace Ui::Component::Button
