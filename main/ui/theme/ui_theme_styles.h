#pragma once
#include "lvgl.h"
#include "ui_theme_tokens.h"

namespace Ui {

// Conjunto de estilos cacheados (se inicializan una vez)
struct UiThemeStyles {
    bool initialized{false};

    lv_style_t base;         // Base para pantallas
    lv_style_t header;       // Barra superior
    lv_style_t content;      // Área de contenido
    lv_style_t footer;       // Barra inferior

    lv_style_t card;         // Contenedor con borde/sombra suave

    lv_style_t labelTitle;   // Títulos
    lv_style_t labelBody;    // Texto normal
    lv_style_t labelCaption; // Texto pequeño/muted

    lv_style_t btnPrimary;   // Botón principal
    lv_style_t btnSecondary; // Botón secundario
    lv_style_t btnGhost;     // Botón “fantasma” (sin fondo)
};

// Inicializa estilos una única vez con los tokens dados
void initThemeStyles(UiThemeStyles& styles, const UiThemeTokens& t);

// Aplicadores de conveniencia
void applyHeader(lv_obj_t* obj, UiThemeStyles& s);
void applyContent(lv_obj_t* obj, UiThemeStyles& s);
void applyFooter(lv_obj_t* obj, UiThemeStyles& s);

/// Acceso global sencillo (singleton mínimo para empezar)
UiThemeStyles& getThemeStyles();
const UiThemeTokens& getThemeTokens();

// Inicializa tokens+styles si aún no lo están
void themeInitOnce();

} // namespace Ui
