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

/** Wrappers consumidos por ui_component_button.cpp */
inline lv_coord_t button_width()    { return static_cast<lv_coord_t>(Button::Width); }
inline lv_coord_t button_height()   { return static_cast<lv_coord_t>(Button::Height); }
inline lv_coord_t button_radius()   { return static_cast<lv_coord_t>(Button::Radius); }
inline lv_coord_t button_pad_lr()   { return static_cast<lv_coord_t>(Button::PadLR); }
inline lv_coord_t button_pad_tb()   { return static_cast<lv_coord_t>(Button::PadTB); }
inline lv_coord_t button_icon_gap() { return static_cast<lv_coord_t>(Button::IconGap); }

}} // namespace Ui::Tokens
