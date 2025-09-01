#pragma once
#include "lvgl.h"
#include "ui/theme/ui_theme_styles.h"  // Usa el theme C++ (tokens+styles)

/**
 * @file ui_layout_scaffold.h
 * @brief Contenedor base (Header/Content/Footer) para las vistas de la HMI.
 *
 * Construye un esqueleto de pantalla compuesto por tres contenedores:
 * - Header: barra superior (app bar).
 * - Content: área central para la vista activa.
 * - Footer: barra inferior (nav/estado/acciones).
 *
 * Aplica estilos desde el Theme (tokens+styles) y configura el layout
 * con flex para que Content ocupe el espacio restante entre Header y Footer.
 */

namespace Ui {

/**
 * @class UiLayoutScaffold
 * @brief Clase responsable de crear y estilizar Header/Content/Footer sobre un root.
 *
 * Uso típico:
 * @code
 * Ui::themeInitOnce();
 * Ui::UiLayoutScaffold scaffold;
 * scaffold.build(lv_scr_act());
 * lv_obj_t* content = scaffold.content();
 * // ... montar widgets de la vista dentro de 'content'
 * @endcode
 */
class UiLayoutScaffold {
public:
    /// Constructor por defecto (no construye objetos; llamar a build()).
    UiLayoutScaffold() = default;

    /**
     * @brief Crea Header/Content/Footer como hijos de @p root y aplica estilos/lay­out.
     * @param root Objeto raíz (normalmente, la pantalla activa `lv_scr_act()`).
     *
     * Idempotente por instancia: si ya estaba construido, no vuelve a crear los objetos.
     */
    void build(lv_obj_t* root);

    /// @return Contenedor superior (Header). Puede ser nullptr hasta que se llame a build().
    lv_obj_t* header()  const { return header_;  }
    /// @return Contenedor central (Content). Puede ser nullptr hasta que se llame a build().
    lv_obj_t* content() const { return content_; }
    /// @return Contenedor inferior (Footer). Puede ser nullptr hasta que se llame a build().
    lv_obj_t* footer()  const { return footer_;  }

    /**
     * @brief Define alturas fijas para Header y Footer antes/después de build().
     * @param headerHeight Altura en píxeles para el Header.
     * @param footerHeight Altura en píxeles para el Footer.
     *
     * Si se invoca tras build(), ajusta tamaños inmediatamente.
     */
    void setBarsHeight(int16_t headerHeight, int16_t footerHeight);

private:
    // Punteros a los contenedores creados
    lv_obj_t* header_  {nullptr};
    lv_obj_t* content_ {nullptr};
    lv_obj_t* footer_  {nullptr};

    // Parámetros de layout (configurables)
    int16_t headerHeightPx_ {56};  ///< Altura por defecto del Header (px).
    int16_t footerHeightPx_ {48};  ///< Altura por defecto del Footer (px).

    // Métodos auxiliares
    void applyLayout_(lv_obj_t* root);
    void applyStyles_();
};

} // namespace Ui
