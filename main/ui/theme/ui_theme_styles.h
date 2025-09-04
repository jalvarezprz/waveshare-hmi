#pragma once
#include "lvgl.h"

namespace Ui {

/** ─────────────────────  TOKENS (valores atómicos)  ───────────────────── **/
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

/** Devuelve tokens por defecto (paleta/fuentes reales). Implementado en .cpp */
UiThemeTokens makeDefaultTokens();

/** ─────────────────────  STYLES (recetas cacheadas)  ───────────────────── **/
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

/** Inicializa todos los styles a partir de los tokens (implementado en .cpp) */
void initThemeStyles(UiThemeStyles& styles, const UiThemeTokens& t);

/** Acceso global (definidos en .cpp) */
UiThemeStyles&       getThemeStyles();
const UiThemeTokens& getThemeTokens();

/** Inicializa una vez (tokens + styles). Llamar al arranque. */
void themeInitOnce();

/** Helpers de aplicación de estilos (implementados en .cpp) */
void applyHeader (lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter (lv_obj_t* obj, UiThemeStyles& s);

// List helpers
void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider);

/** ────────────────  Helpers de Botón (tokens tamaño + estados) ───────────────
 * Estos helpers:
 *  - Añaden el style base correspondiente (primary/secondary/ghost).
 *  - Ajustan tamaño desde tokens Ui::Tokens::button_width/height().
 *  - Configuran colores de fondo para PRESSED/FOCUSED sin crear nuevos styles.
 */
void applyButtonPrimary  (lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);
void applyButtonSecondary(lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);
void applyButtonGhost    (lv_obj_t* btn, UiThemeStyles& s, bool setSize = true);

} // namespace Ui
