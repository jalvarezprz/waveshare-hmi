#pragma once
#include "lvgl.h"

namespace Ui {

/**
 * @brief Tokens de diseño (paleta, tipografías, spacing, radios, alturas).
 *        A partir de estos tokens derivan los estilos LVGL cacheados.
 */
struct UiThemeTokens {
    // Paleta base
    lv_color_t colorBg;
    lv_color_t colorSurface;
    lv_color_t colorPrimary;
    lv_color_t colorSecondary;
    lv_color_t colorText;
    lv_color_t colorMuted;
    lv_color_t colorSuccess;
    lv_color_t colorWarning;
    lv_color_t colorError;

    // Opacidades
    lv_opa_t  opaEnabled;
    lv_opa_t  opaDisabled;

    // Radios y spacing
    uint16_t radiusSm;
    uint16_t radiusMd;
    uint16_t radiusLg;

    uint8_t  spaceXs;
    uint8_t  spaceSm;
    uint8_t  spaceMd;
    uint8_t  spaceLg;

    // Tipografías
    const lv_font_t* fontTitle;
    const lv_font_t* fontBody;
    const lv_font_t* fontCaption;

    // Alturas estándar de filas para listas
    uint16_t itemHeightMd;   ///< Fila normal, táctil (~56 px)
    uint16_t itemHeightLg;   ///< Fila grande (~64 px)
};

/** @brief Devuelve tokens por defecto (contraste adecuado, seguros). */
UiThemeTokens makeDefaultTokens();

} // namespace Ui
