#pragma once
/**
 * ui_theme_tokens.h — Consolidado
 * - Primitivas core (Color/Space/Radius)
 * - Tokens de List
 * - Tokens de Button
 * - Labels & Typography helpers
 * Todo inline (header-only) para evitar linker issues.
 */
#include "lvgl.h"
#include <cstdint>

namespace Ui { namespace Tokens {

/* ───────────────────────  CORE PRIMITIVES  ─────────────────────── */

// Colores base en RGBA (0xRRGGBBAA)
namespace Color {
    inline constexpr std::uint32_t Surface        = 0x121212FF; // Fondo base oscuro
    inline constexpr std::uint32_t OnSurface      = 0xEDEDEDFF; // Texto sobre Surface
    inline constexpr std::uint32_t SurfaceVariant = 0x1E1E1EFF; // Cards / containers
    inline constexpr std::uint32_t Outline        = 0x3C3C3CFF; // Bordes / separadores
    inline constexpr std::uint32_t Muted          = 0x9E9E9EFF; // Texto secundario
}

// Conversión RGBA -> lv_color_t (0xRRGGBB)
static inline lv_color_t rgba_to_lv(std::uint32_t rgba) {
    return lv_color_hex(rgba & 0x00FFFFFFu);
}

// Helpers LVGL de color
inline lv_color_t color_surface()          { return rgba_to_lv(Color::Surface); }
inline lv_color_t color_on_surface()       { return rgba_to_lv(Color::OnSurface); }
inline lv_color_t color_surface_variant()  { return rgba_to_lv(Color::SurfaceVariant); }
inline lv_color_t color_outline()          { return rgba_to_lv(Color::Outline); }
inline lv_color_t color_muted()            { return rgba_to_lv(Color::Muted); }

// Espaciados y radios (px)
namespace Space  { inline constexpr int xs=4, sm=8, md=12, lg=16, xl=24; }
namespace Radius { inline constexpr int sm=4, md=8, lg=12; }

/* ─────────────────────────  LIST TOKENS  ───────────────────────── */

namespace List {
    inline constexpr int ItemHeightMd = 56;
    inline constexpr int ItemHeightLg = 64;
    inline constexpr int PadLR        = 16;
    inline constexpr int PadTB        = 8;
    inline constexpr int GapRow       = 8;
    inline constexpr int DividerWidth = 1;
}

/* --------------------------------------------------------------------------
 * PANELS & CARDS (tokens semánticos)
 * --------------------------------------------------------------------------*/

// Panel: barras superior/inferior y contenedores planos
namespace Panel {
    inline constexpr int PadAll     = Space::sm;   // padding uniforme
    inline constexpr int BorderW    = 1;           // grosor de borde
    // Colores derivados de primitivas:
    //   bg   -> color_surface_variant()
    //   text -> color_on_surface()
    //   bord -> color_outline()
}

// Card: contenedores elevados (tarjetas)
namespace Card {
    inline constexpr int  Radius      = Radius::md;
    inline constexpr int  PadAll      = Space::md;
    inline constexpr int  BorderW     = 1;
    inline constexpr int  ShadowW     = 8;
    inline constexpr int  ShadowOfsY  = 2;
    // Colores derivados:
    //   bg        -> color_surface()
    //   text      -> color_on_surface()
    //   border    -> color_outline()
    //   shadow    -> negro (o el que prefieras)
}

// Helpers LVGL (medidas)
inline lv_coord_t panel_pad_all()      { return static_cast<lv_coord_t>(Panel::PadAll); }
inline lv_coord_t panel_border_w()     { return static_cast<lv_coord_t>(Panel::BorderW); }

inline lv_coord_t card_radius()        { return static_cast<lv_coord_t>(Card::Radius); }
inline lv_coord_t card_pad_all()       { return static_cast<lv_coord_t>(Card::PadAll); }
inline lv_coord_t card_border_w()      { return static_cast<lv_coord_t>(Card::BorderW); }
inline lv_coord_t card_shadow_w()      { return static_cast<lv_coord_t>(Card::ShadowW); }
inline lv_coord_t card_shadow_ofs_y()  { return static_cast<lv_coord_t>(Card::ShadowOfsY); }

// Helpers LVGL (colores)
inline lv_color_t panel_bg_color()     { return color_surface_variant(); }
inline lv_color_t panel_text_color()   { return color_on_surface(); }
inline lv_color_t panel_border_color() { return color_outline(); }

inline lv_color_t card_bg_color()      { return color_surface(); }
inline lv_color_t card_text_color()    { return color_on_surface(); }
inline lv_color_t card_border_color()  { return color_outline(); }
inline lv_color_t card_shadow_color()  { return lv_color_black(); } // ajustable si más tarde defines Elevation


// Helpers LVGL: métricas y colores de lista
inline lv_coord_t list_item_height_md() { return static_cast<lv_coord_t>(List::ItemHeightMd); }
inline lv_coord_t list_item_height_lg() { return static_cast<lv_coord_t>(List::ItemHeightLg); }
inline lv_coord_t list_pad_lr()         { return static_cast<lv_coord_t>(List::PadLR); }
inline lv_coord_t list_pad_tb()         { return static_cast<lv_coord_t>(List::PadTB); }
inline lv_coord_t list_gap_row()        { return static_cast<lv_coord_t>(List::GapRow); }
inline lv_coord_t list_divider_width()  { return static_cast<lv_coord_t>(List::DividerWidth); }

inline lv_color_t list_divider_color()  { return color_outline(); }
inline lv_color_t list_bg_color()       { return color_surface(); }
inline lv_color_t list_item_bg_color()  { return color_surface(); }
inline lv_color_t list_text_color()     { return color_on_surface(); }

/* ────────────────────────  BUTTON TOKENS  ──────────────────────── */

// Colores específicos de botón primario
namespace ButtonColor {
    inline constexpr std::uint32_t Primary   = 0x1976D2FF; // Azul primario
    inline constexpr std::uint32_t OnPrimary = 0xFFFFFFFF; // Texto/ícono sobre primario
}

// Medidas del botón (px)
namespace Button {
    inline constexpr int Width   = 160;
    inline constexpr int Height  = 44;
    inline constexpr int Radius  = 12;
    inline constexpr int PadLR   = 16;
    inline constexpr int PadTB   = 8;
    inline constexpr int IconGap = 8;
}

// Wrappers consumidos por ui_component_button.cpp
inline lv_color_t button_primary_bg()         { return rgba_to_lv(ButtonColor::Primary); }
inline lv_color_t button_primary_bg_pressed() { return lv_color_darken(button_primary_bg(), 20); }
inline lv_color_t button_primary_bg_focused() { return lv_color_lighten(button_primary_bg(), 20); }
inline lv_color_t button_primary_text()       { return rgba_to_lv(ButtonColor::OnPrimary); }

inline lv_coord_t button_width()    { return static_cast<lv_coord_t>(Button::Width); }
inline lv_coord_t button_height()   { return static_cast<lv_coord_t>(Button::Height); }
inline lv_coord_t button_radius()   { return static_cast<lv_coord_t>(Button::Radius); }
inline lv_coord_t button_pad_lr()   { return static_cast<lv_coord_t>(Button::PadLR); }
inline lv_coord_t button_pad_tb()   { return static_cast<lv_coord_t>(Button::PadTB); }
inline lv_coord_t button_icon_gap() { return static_cast<lv_coord_t>(Button::IconGap); }

// Fuentes (fallback seguro). Cambia por tus Montserrat cuando corresponda.
inline const lv_font_t* font_icon() { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_text() { return LV_FONT_DEFAULT; }

/* ─────────────────────  LABELS & TYPOGRAPHY  ───────────────────── */

inline lv_color_t label_primary_color()   { return color_on_surface(); }
inline lv_color_t label_secondary_color() { return color_muted(); }

namespace TextEmphasis {
    inline constexpr lv_opa_t High     = LV_OPA_COVER; // 100%
    inline constexpr lv_opa_t Medium   = LV_OPA_70;    // ~70%
    inline constexpr lv_opa_t Disabled = LV_OPA_40;    // ~40%
}

// Guía semántica de tamaños (si necesitas referencia); LVGL usa fuentes compiladas
namespace TypographyPx { inline constexpr int Title=14, Body=12, Caption=8; }

}} // namespace Ui::Tokens
