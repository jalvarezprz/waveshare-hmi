#pragma once
#include "lvgl.h"
#include <stdint.h>

namespace Ui {

/**
 * Snapshot de tokens consumible por los estilos.
 * (Rellenado en themeInitOnce() a partir de Ui::Tokens::*)
 */
struct UiThemeTokens {
    // --- Colores base ---
    lv_color_t colorSurface;
    lv_color_t colorSurfaceVariant;
    lv_color_t colorOnSurface;
    lv_color_t colorOutline;
    lv_color_t colorMuted;

    // Marca / acciones
    lv_color_t colorPrimary;
    lv_color_t colorOnPrimary;
    lv_color_t colorSecondary;
    lv_color_t colorOnSecondary;

    // Semáforo / estado
    lv_color_t colorSuccess;
    lv_color_t colorOnSuccess;
    lv_color_t colorWarning;
    lv_color_t colorOnWarning;
    lv_color_t colorError;
    lv_color_t colorOnError;

    // Overlay
    lv_color_t colorOverlayBg;

    // --- Opacidades ---
    lv_opa_t opaEnabled;
    lv_opa_t opaDisabled;
    lv_opa_t opaHover;
    lv_opa_t opaPressed;
    lv_opa_t opaFocus;
    lv_opa_t opaOverlay;

    // --- Tipografía ---
    const lv_font_t* fontTitle;
    const lv_font_t* fontBody;
    const lv_font_t* fontCaption;
    const lv_font_t* fontIcon;

    // --- Focus ring ---
    lv_coord_t focusOutlineW;
    lv_coord_t focusOutlinePad;
    lv_color_t focusOutlineColor;

    // --- Layout / densidad ---
    lv_coord_t gapRow;
    lv_coord_t gapCol;
    lv_coord_t minTouch;

    // --- Superficies / paneles / tarjetas ---
    lv_coord_t panelPadAll;
    lv_coord_t panelBorderW;

    lv_coord_t cardRadius;
    lv_coord_t cardPadAll;
    lv_coord_t cardBorderW;
    lv_coord_t cardShadowW;
    lv_coord_t cardShadowOfsY;
    lv_color_t cardShadowColor;

    // --- Listas ---
    lv_coord_t listItemH_md;
    lv_coord_t listItemH_lg;
    lv_coord_t listPadLR;
    lv_coord_t listPadTB;
    lv_coord_t listGapRow;
    lv_coord_t listDividerW;
    lv_opa_t   listDividerOpa;

    // --- Controles: botón geom. ---
    lv_coord_t btnWidth;
    lv_coord_t btnHeight;
    lv_coord_t btnRadius;
    lv_coord_t btnPadLR;
    lv_coord_t btnPadTB;
    lv_coord_t btnIconGap;
};

/**
 * Estilos precompuestos del tema (consumibles por la UI).
 * Se construyen desde UiThemeTokens en themeInitOnce().
 */
struct UiThemeStyles {
    // Snapshot
    UiThemeTokens tokens;

    // Estilos globales
    lv_style_t base;

    // Barras y contenedores
    lv_style_t header;
    lv_style_t footer;
    lv_style_t content;

    // Superficies
    lv_style_t card;

    // Tipografía
    lv_style_t labelTitle;
    lv_style_t labelBody;
    lv_style_t labelCaption;

    // Listas
    lv_style_t listContainer;
    lv_style_t listItem;
    lv_style_t listItemPressed;
    lv_style_t listItemFocused;
    lv_style_t listItemDisabled;
    lv_style_t listDivider;

    // Botones
    lv_style_t btnPrimary;
    lv_style_t btnSecondary;
    lv_style_t btnGhost;

    bool initialized = false;
};

/*===================== Ciclo de vida =====================*/
void           themeInitOnce();
UiThemeStyles& getThemeStyles();
const UiThemeTokens& getThemeTokens();

/*===================== Aplicadores de estilo =====================*/
// Barras / contenidos
void applyHeader (lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter (lv_obj_t* obj, UiThemeStyles& s);

// Listas
void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider);

// Botones
void applyButtonPrimary  (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonSecondary(lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonGhost    (lv_obj_t* btn, UiThemeStyles& s, bool setSize);

} // namespace Ui
