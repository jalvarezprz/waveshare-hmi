#pragma once
/**
 * @file ui_theme_tokens.h
 * @brief Tokens autosuficientes para Button (RGBA, paddings, medidas y fuentes fallback).
 *        Implementación inline (header-only) para evitar dependencias de ui_theme_styles
 *        y errores de enlace.
 */

#include "lvgl.h"
#include <cstdint>

namespace Ui { namespace Tokens {

/* --------------------------------------------------------------------------
 * Colores (RGBA) para botón primario
 * Nota: lv_color_t no usa alfa; la opacidad se controla con *_opa.
 * --------------------------------------------------------------------------*/
namespace ButtonColor {
    inline constexpr std::uint32_t Primary   = 0x1976D2FF; // Azul primario
    inline constexpr std::uint32_t OnPrimary = 0xFFFFFFFF; // Texto/ícono sobre primario
}

// Conversión RGBA -> lv_color_t (0xRRGGBB)
static inline lv_color_t rgba_to_lv(std::uint32_t rgba) {
    return lv_color_hex(rgba & 0x00FFFFFFu);
}

/** Colores para botón primario (wrappers consumidos por Button) */
inline lv_color_t button_primary_bg()         { return rgba_to_lv(ButtonColor::Primary); }
inline lv_color_t button_primary_bg_pressed() { return lv_color_darken(rgba_to_lv(ButtonColor::Primary), 20); }
inline lv_color_t button_primary_bg_focused() { return lv_color_lighten(rgba_to_lv(ButtonColor::Primary), 20); }
inline lv_color_t button_primary_text()       { return rgba_to_lv(ButtonColor::OnPrimary); }

/* --------------------------------------------------------------------------
 * Fuentes usadas por el botón (fallback seguro)
 * Sustituir retornos por &montserrat_* cuando tengas las fuentes cargadas.
 * --------------------------------------------------------------------------*/
inline const lv_font_t* font_icon() { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_text() { return LV_FONT_DEFAULT; }

/* --------------------------------------------------------------------------
 * Medidas del botón (px)
 * --------------------------------------------------------------------------*/
namespace Button {
    inline constexpr int Width   = 160;
    inline constexpr int Height  = 44;
    inline constexpr int Radius  = 12;
    inline constexpr int PadLR   = 16;
    inline constexpr int PadTB   = 8;
    inline constexpr int IconGap = 8;
}

/* --------------------------------------------------------------------------
 * PRIMITIVAS CORE (no solo botón)
 * - Colores en RGBA 0xRRGGBBAA (lv_color_t ignora alfa; opacidad con *_opa)
 * - Espaciado y radios en px
 * --------------------------------------------------------------------------*/
namespace Color {
    inline constexpr std::uint32_t Surface        = 0x121212FF; // Fondo base oscuro
    inline constexpr std::uint32_t OnSurface      = 0xEDEDEDFF; // Texto sobre Surface
    inline constexpr std::uint32_t SurfaceVariant = 0x1E1E1EFF; // Variante de superficie (cards, contenedores)
    inline constexpr std::uint32_t Outline        = 0x3C3C3CFF; // Bordes / separadores discretos
}

namespace Space {
    inline constexpr int xs = 4;
    inline constexpr int  sm = 8;
    inline constexpr int  md = 12;
    inline constexpr int  lg = 16;
    inline constexpr int xl = 24;
}

namespace Radius {
    inline constexpr int sm = 4;
    inline constexpr int md = 8;
    inline constexpr int lg = 12;
}

/* Helpers LVGL convenientes (usar según necesidad) */
inline lv_color_t color_surface()        { return rgba_to_lv(Color::Surface); }
inline lv_color_t color_on_surface()     { return rgba_to_lv(Color::OnSurface); }
inline lv_color_t color_surface_variant(){ return rgba_to_lv(Color::SurfaceVariant); }
inline lv_color_t color_outline()        { return rgba_to_lv(Color::Outline); }

/* --------------------------------------------------------------------------
 * LIST TOKENS (métricas y helpers LVGL)
 * --------------------------------------------------------------------------*/
namespace List {
    // Alturas táctiles recomendadas (px)
    inline constexpr int ItemHeightMd = 56;
    inline constexpr int ItemHeightLg = 64;

    // Padding interno de cada item (px)
    inline constexpr int PadLR = 16;  // left/right
    inline constexpr int PadTB = 8;   // top/bottom

    // Separación vertical entre filas (si se usa en contenedor)
    inline constexpr int GapRow = 8;

    // Grosor del divisor inferior (px)
    inline constexpr int DividerWidth = 1;
}

// Helpers LVGL: métricas como lv_coord_t
inline lv_coord_t list_item_height_md() { return static_cast<lv_coord_t>(List::ItemHeightMd); }
inline lv_coord_t list_item_height_lg() { return static_cast<lv_coord_t>(List::ItemHeightLg); }
inline lv_coord_t list_pad_lr()         { return static_cast<lv_coord_t>(List::PadLR); }
inline lv_coord_t list_pad_tb()         { return static_cast<lv_coord_t>(List::PadTB); }
inline lv_coord_t list_gap_row()        { return static_cast<lv_coord_t>(List::GapRow); }
inline lv_coord_t list_divider_width()  { return static_cast<lv_coord_t>(List::DividerWidth); }

// Helpers LVGL: colores ya definidos en primitivas core
inline lv_color_t list_divider_color()  { return color_outline(); }
inline lv_color_t list_bg_color()       { return color_surface(); }
inline lv_color_t list_item_bg_color()  { return color_surface(); }
inline lv_color_t list_text_color()     { return color_on_surface(); }

/** Wrappers consumidos por ui_component_button.cpp */
inline lv_coord_t button_width()    { return static_cast<lv_coord_t>(Button::Width); }
inline lv_coord_t button_height()   { return static_cast<lv_coord_t>(Button::Height); }
inline lv_coord_t button_radius()   { return static_cast<lv_coord_t>(Button::Radius); }
inline lv_coord_t button_pad_lr()   { return static_cast<lv_coord_t>(Button::PadLR); }
inline lv_coord_t button_pad_tb()   { return static_cast<lv_coord_t>(Button::PadTB); }
inline lv_coord_t button_icon_gap() { return static_cast<lv_coord_t>(Button::IconGap); }

}} // namespace Ui::Tokens
