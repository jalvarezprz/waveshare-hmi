#pragma once
#include "lvgl.h"

namespace Ui {

// Tokens: constantes de diseño (paleta, radios, spacing, tipografías)
struct UiThemeTokens {
    // Paleta base
    lv_color_t colorBg;         // Fondo de pantallas
    lv_color_t colorSurface;    // Tarjetas / contenedores
    lv_color_t colorPrimary;    // Color principal (acciones)
    lv_color_t colorSecondary;  // Color secundario
    lv_color_t colorText;       // Texto principal
    lv_color_t colorMuted;      // Texto secundario / bordes suaves
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

    // Tipografías (usa las que tengas habilitadas en LVGL; por defecto, la global)
    const lv_font_t* fontTitle;
    const lv_font_t* fontBody;
    const lv_font_t* fontCaption;
};

// Devuelve un set de tokens por defecto (ligero, contrastado)
UiThemeTokens makeDefaultTokens();

} // namespace Ui
