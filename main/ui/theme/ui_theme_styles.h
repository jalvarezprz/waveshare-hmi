#pragma once
/**
 * @file ui_theme_styles.h
 * @brief ThemeStyles: recetas LVGL construidas desde Ui::Tokens (snapshot).
 * @ingroup ui_theme_styles
 *
 * Notas:
 *  - No accede a Ui::Tokens directamente fuera de la creación del snapshot.
 *  - Expone aplicadores de estilos para superficies, listas y botones (base).
 */

#include "lvgl.h"

namespace Ui {

/*========================== Snapshot de Tokens ========================*/
struct UiThemeTokens {
    /* Colores (roles) */
    lv_color_t colorSurface{};
    lv_color_t colorOnSurface{};
    lv_color_t colorSurfaceVariant{};
    lv_color_t colorOutline{};
    lv_color_t colorMuted{};
    lv_color_t colorPrimary{};
    lv_color_t colorOnPrimary{};
    lv_color_t colorSecondary{};
    lv_color_t colorOnSecondary{};
    lv_color_t colorSuccess{};
    lv_color_t colorOnSuccess{};
    lv_color_t colorWarning{};
    lv_color_t colorOnWarning{};
    lv_color_t colorError{};
    lv_color_t colorOnError{};
    lv_color_t colorOverlayBg{};

    /* Opacidades (roles) */
    lv_opa_t opaEnabled{};
    lv_opa_t opaDisabled{};
    lv_opa_t opaHover{};
    lv_opa_t opaPressed{};
    lv_opa_t opaFocus{};
    lv_opa_t opaOverlay{};

    /* Tipografía */
    const lv_font_t* fontTitle{};
    const lv_font_t* fontBody{};
    const lv_font_t* fontCaption{};
    const lv_font_t* fontIcon{};

    /* Shape / layout / density */
    lv_coord_t focusOutlineW{};
    lv_coord_t focusOutlinePad{};
    lv_color_t focusOutlineColor{};

    lv_coord_t gapRow{};
    lv_coord_t gapCol{};
    lv_coord_t minTouch{};

    /* Superficies / listas (geométricos) */
    lv_coord_t panelPadAll{};
    lv_coord_t panelBorderW{};

    lv_coord_t cardRadius{};
    lv_coord_t cardPadAll{};
    lv_coord_t cardBorderW{};
    lv_coord_t cardShadowW{};
    lv_coord_t cardShadowOfsY{};
    lv_color_t cardShadowColor{};

    lv_coord_t listItemH_md{};
    lv_coord_t listItemH_lg{};
    lv_coord_t listPadLR{};
    lv_coord_t listPadTB{};
    lv_coord_t listGapRow{};
    lv_coord_t listDividerW{};
    lv_opa_t   listDividerOpa{};

    /* Controles (geométricos) */
    lv_coord_t btnWidth{};
    lv_coord_t btnHeight{};
    lv_coord_t btnRadius{};
    lv_coord_t btnPadLR{};
    lv_coord_t btnPadTB{};
    lv_coord_t btnIconGap{};
};

/*========================== Styles cacheados ==========================*/
struct UiThemeStyles {
    bool initialized{false};

    // Superficies
    lv_style_t base;
    lv_style_t header;
    lv_style_t content;
    lv_style_t footer;
    lv_style_t card;

    // Texto
    lv_style_t labelTitle;
    lv_style_t labelBody;
    lv_style_t labelCaption;

    // Lista
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

    // Snapshot activo (para cálculos de estados)
    UiThemeTokens tokens;
};

/*====================== API pública (igual que tenías) =================*/
UiThemeStyles&       getThemeStyles();
const UiThemeTokens& getThemeTokens();

/* Inicialización */
void themeInitOnce();

/* Superficies */
void applyHeader (lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter (lv_obj_t* obj, UiThemeStyles& s);
void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider);

/* Botones (variantes base) */
void applyButtonPrimary  (lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonSecondary(lv_obj_t* btn, UiThemeStyles& s, bool setSize);
void applyButtonGhost    (lv_obj_t* btn, UiThemeStyles& s, bool setSize);

} // namespace Ui
