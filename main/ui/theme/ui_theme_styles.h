#pragma once
/**
 * @file ui_theme_styles.h
 * @brief Estilos cacheados construidos a partir de tokens de tema.
 * @defgroup ui_theme_styles Styles de UI
 * @ingroup ui_theme
 * @{
 */

#include "lvgl.h"

namespace Ui {

/** @brief Snapshot de tokens atómicos para construir estilos. */
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
    uint16_t itemHeightMd;   ///< ~56 px
    uint16_t itemHeightLg;   ///< ~64 px
};

/** @brief Crea tokens por defecto a partir de Ui::Tokens (tema actual). */
UiThemeTokens makeDefaultTokens();

/** @brief Conjunto de estilos LVGL cacheados (recetas). */
struct UiThemeStyles {
    // Base layout
    lv_style_t base;
    lv_style_t header;
    lv_style_t content;
    lv_style_t footer;
    lv_style_t card;

    // Labels
    lv_style_t labelTitle;
    lv_style_t labelBody;
    lv_style_t labelCaption;

    // Botones
    lv_style_t btnPrimary;
    lv_style_t btnSecondary;
    lv_style_t btnGhost;

    // Listas
    lv_style_t listContainer;
    lv_style_t listItem;
    lv_style_t listItemPressed;
    lv_style_t listItemFocused;
    lv_style_t listItemDisabled;
    lv_style_t listDivider;

    bool initialized = false;
};

/** @brief Inicializa todos los styles a partir de los tokens. */
void initThemeStyles(UiThemeStyles& styles, const UiThemeTokens& t);

/** @name Acceso global */
/** @{ */
UiThemeStyles&       getThemeStyles();
const UiThemeTokens& getThemeTokens();
/** @} */

/** @brief Inicializa tema una vez (tokens + styles). Llamar en arranque. */
void themeInitOnce();

/** @name Helpers de aplicación de estilos */
/** @{ */
void applyHeader (lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter (lv_obj_t* obj, UiThemeStyles& s);

void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider);
/** @} */

/** @name Helpers de Botón (tamaño por tokens + estados) */
/** @{
 * Estos helpers:
 *  - Añaden el style base (primary/secondary/ghost).
 *  - Ajustan tamaño desde Ui::Tokens::button_width/height().
 *  - Configuran estados PRESSED/FOCUSED sin ampliar UiThemeStyles.
 */
void applyButtonPrimary  (lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);
void applyButtonSecondary(lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);
void applyButtonGhost    (lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);
/** @} */

} // namespace Ui

/** @} */ // end of ui_theme_styles
