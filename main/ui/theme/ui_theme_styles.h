#pragma once
#include "lvgl.h"
#include "ui_theme_tokens.h"

namespace Ui {

/**
 * @brief Conjunto de estilos cacheados (se inicializan una única vez).
 */
struct UiThemeStyles {
    bool initialized{false};

    // Estructura general
    lv_style_t base;
    lv_style_t header;
    lv_style_t content;
    lv_style_t footer;

    // Contenedores
    lv_style_t card;

    // Tipos de texto
    lv_style_t labelTitle;
    lv_style_t labelBody;
    lv_style_t labelCaption;

    // Botones
    lv_style_t btnPrimary;
    lv_style_t btnSecondary;
    lv_style_t btnGhost;

    // Lista e ítems de lista
    lv_style_t listContainer;    // contenedor de lista
    lv_style_t listItem;         // ítem normal
    lv_style_t listItemPressed;  // estado pressed
    lv_style_t listItemFocused;  // foco/selección
    lv_style_t listItemDisabled; // deshabilitado
    lv_style_t listDivider;      // divisor inferior
};

// Inicialización y acceso global
void initThemeStyles(UiThemeStyles& styles, const UiThemeTokens& t);
UiThemeStyles&     getThemeStyles();
const UiThemeTokens& getThemeTokens();
void themeInitOnce();

// Aplicadores layout base
void applyHeader(lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter(lv_obj_t* obj, UiThemeStyles& s);

// Helpers de listas
void applyListContainer(lv_obj_t* obj, UiThemeStyles& s);
void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large = false, bool withDivider = true);
void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large = false, bool withDivider = true);

} // namespace Ui
