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

    // --- Badges ---
    lv_coord_t badgePadH;
    lv_coord_t badgePadV;
    lv_coord_t badgeRadius;
    lv_coord_t badgeOffsetX;
    lv_coord_t badgeOffsetY;

    // --- Iconos ---
    lv_coord_t iconSizeSm;
    lv_coord_t iconSizeMd;
    lv_coord_t iconSizeLg;
    lv_coord_t iconGap;
    lv_color_t iconColorOnSurface;
    lv_color_t iconColorOnPrimary;
    lv_color_t iconColorMuted;
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

    // Botones (bases)
    lv_style_t btnPrimary;
    lv_style_t btnSecondary;
    lv_style_t btnGhost;

    // Variantes extra
    lv_style_t btnDestructive;
    lv_style_t btnSuccess;
    lv_style_t btnWarning;

    // Iconos
    lv_style_t iconOnSurface;
    lv_style_t iconOnPrimary;
    lv_style_t iconMuted;

    bool initialized = false;
};

/*===================== Ciclo de vida =====================*/
void                 themeInitOnce();
UiThemeStyles&       getThemeStyles();
const UiThemeTokens& getThemeTokens();
/** Reconstruye snapshot de tokens + estilos */
void themeReload();

/*===================== Aplicadores de estilo =====================*/
// Barras / contenidos
void applyHeader (lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter (lv_obj_t* obj, UiThemeStyles& s);

// Listas
void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider);

// Botones (variantes)
void applyButtonPrimary    (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonSecondary  (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonGhost      (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonDestructive(lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonSuccess    (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonWarning    (lv_obj_t* btn, UiThemeStyles& s, bool setSize);

// Iconos (una sola familia de helpers; size_px opcional)
void applyIconOnSurface(lv_obj_t* obj, UiThemeStyles& s, lv_coord_t size_px = 0);
void applyIconOnPrimary(lv_obj_t* obj, UiThemeStyles& s, lv_coord_t size_px = 0);
void applyIconMuted    (lv_obj_t* obj, UiThemeStyles& s, lv_coord_t size_px = 0);

// (Opcional) Alias por comodidad/compatibilidad:
inline void applyIconDefault(lv_obj_t* obj, UiThemeStyles& s, lv_coord_t size_px = 0) {
    applyIconOnSurface(obj, s, size_px);
}

/*========================== Presets por bloque (lego) ========================*/
void applySensorTile     (lv_obj_t* obj, UiThemeStyles& s);
void applySensorGrid     (lv_obj_t* obj, UiThemeStyles& s);
void applySwitchGroupBox (lv_obj_t* obj, UiThemeStyles& s);
void applyPanelSurface   (lv_obj_t* obj, UiThemeStyles& s);

} // namespace Ui
