#pragma once
/**
 * ui_theme_tokens.h — Consolidado (header-only)
 * - Primitivas core (Color/Space/Radius)
 * - Paleta completa + helpers color_*
 * - Panels & Cards tokens
 * - List tokens
 * - Button tokens (wrappers sobre paleta)
 * - Labels & Typography helpers
 * - Fuentes reales (title/body/caption) en tokens + icon font opcional
 */
#include "lvgl.h"
#include <cstdint>

namespace Ui { namespace Tokens {

/* ───────────────────────  CORE PRIMITIVES  ─────────────────────── */

/** Conversión RGBA -> lv_color_t (0xRRGGBB) */
static inline lv_color_t rgba_to_lv(std::uint32_t rgba) {
    return lv_color_hex(rgba & 0x00FFFFFFu);
}

/** Paleta base y completa (RGBA 0xRRGGBBAA) */
namespace Color {
    // Superficies
    inline constexpr std::uint32_t Surface        = 0x121212FF; // fondo base (oscuro)
    inline constexpr std::uint32_t OnSurface      = 0xEDEDEDFF; // texto sobre Surface
    inline constexpr std::uint32_t SurfaceVariant = 0x1E1E1EFF; // cards / containers
    inline constexpr std::uint32_t Outline        = 0x3C3C3CFF; // bordes / separadores
    inline constexpr std::uint32_t Muted          = 0x9E9E9EFF; // texto secundario

    // Paleta de marca (ajusta a tu branding si procede)
    inline constexpr std::uint32_t Primary     = 0x42A5F5FF;
    inline constexpr std::uint32_t OnPrimary   = 0xFFFFFFFF;

    inline constexpr std::uint32_t Secondary   = 0x9C27B0FF;
    inline constexpr std::uint32_t OnSecondary = 0xFFFFFFFF;

    inline constexpr std::uint32_t Success     = 0x4CAF50FF;
    inline constexpr std::uint32_t OnSuccess   = 0xFFFFFFFF;

    inline constexpr std::uint32_t Warning     = 0xFFC107FF;
    inline constexpr std::uint32_t OnWarning   = 0x212121FF;

    inline constexpr std::uint32_t Error       = 0xF44336FF;
    inline constexpr std::uint32_t OnError     = 0xFFFFFFFF;
}

/** Helpers LVGL de color (derivados de Color::*) */
inline lv_color_t color_surface()          { return rgba_to_lv(Color::Surface); }
inline lv_color_t color_on_surface()       { return rgba_to_lv(Color::OnSurface); }
inline lv_color_t color_surface_variant()  { return rgba_to_lv(Color::SurfaceVariant); }
inline lv_color_t color_outline()          { return rgba_to_lv(Color::Outline); }
inline lv_color_t color_muted()            { return rgba_to_lv(Color::Muted); }

inline lv_color_t color_primary()          { return rgba_to_lv(Color::Primary); }
inline lv_color_t color_on_primary()       { return rgba_to_lv(Color::OnPrimary); }
inline lv_color_t color_secondary()        { return rgba_to_lv(Color::Secondary); }
inline lv_color_t color_on_secondary()     { return rgba_to_lv(Color::OnSecondary); }
inline lv_color_t color_success()          { return rgba_to_lv(Color::Success); }
inline lv_color_t color_on_success()       { return rgba_to_lv(Color::OnSuccess); }
inline lv_color_t color_warning()          { return rgba_to_lv(Color::Warning); }
inline lv_color_t color_on_warning()       { return rgba_to_lv(Color::OnWarning); }
inline lv_color_t color_error()            { return rgba_to_lv(Color::Error); }
inline lv_color_t color_on_error()         { return rgba_to_lv(Color::OnError); }

/** Espaciados y radios (px) */
namespace Space  { inline constexpr int xs=4, sm=8, md=12, lg=16, xl=24; }
namespace Radius { inline constexpr int sm=4, md=8, lg=12; }

/* ─────────────────────────  PANELS & CARDS  ───────────────────────── */

namespace Panel {
    inline constexpr int PadAll  = Space::sm;
    inline constexpr int BorderW = 1;
}
namespace Card {
    inline constexpr int Radius     = Radius::md;
    inline constexpr int PadAll     = Space::md;
    inline constexpr int BorderW    = 1;
    inline constexpr int ShadowW    = 8;
    inline constexpr int ShadowOfsY = 2;
}

// Helpers medidas
inline lv_coord_t panel_pad_all()      { return static_cast<lv_coord_t>(Panel::PadAll); }
inline lv_coord_t panel_border_w()     { return static_cast<lv_coord_t>(Panel::BorderW); }
inline lv_coord_t card_radius()        { return static_cast<lv_coord_t>(Card::Radius); }
inline lv_coord_t card_pad_all()       { return static_cast<lv_coord_t>(Card::PadAll); }
inline lv_coord_t card_border_w()      { return static_cast<lv_coord_t>(Card::BorderW); }
inline lv_coord_t card_shadow_w()      { return static_cast<lv_coord_t>(Card::ShadowW); }
inline lv_coord_t card_shadow_ofs_y()  { return static_cast<lv_coord_t>(Card::ShadowOfsY); }

// Helpers colores
inline lv_color_t panel_bg_color()     { return color_surface_variant(); }
inline lv_color_t panel_text_color()   { return color_on_surface(); }
inline lv_color_t panel_border_color() { return color_outline(); }
inline lv_color_t card_bg_color()      { return color_surface(); }
inline lv_color_t card_text_color()    { return color_on_surface(); }
inline lv_color_t card_border_color()  { return color_outline(); }
inline lv_color_t card_shadow_color()  { return lv_color_black(); }

/* ─────────────────────────────  LIST  ───────────────────────────── */

namespace List {
    inline constexpr int ItemHeightMd = 56;
    inline constexpr int ItemHeightLg = 64;
    inline constexpr int PadLR        = 16;
    inline constexpr int PadTB        = 8;
    inline constexpr int GapRow       = 8;
    inline constexpr int DividerWidth = 1;
}
// Helpers métricas/colores de lista
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

/* ───────── Interaction (opacidades/outline para estados de lista) ───────── */
namespace Interaction {
    inline constexpr lv_opa_t PressedTintOpa = LV_OPA_20;  // mezcla para item pressed
    inline constexpr lv_opa_t DisabledOpa    = LV_OPA_50;  // opacidad disabled
    inline constexpr int      FocusOutlineW  = 2;          // grosor outline
    inline constexpr int      FocusOutlinePad= 2;          // padding del outline
}

// Wrappers LVGL usados por ui_theme_styles.cpp
inline lv_opa_t  list_pressed_tint_opa()   { return Interaction::PressedTintOpa; }
inline lv_opa_t  list_disabled_opa()       { return Interaction::DisabledOpa; }
inline lv_coord_t list_focus_outline_w()   { return static_cast<lv_coord_t>(Interaction::FocusOutlineW); }
inline lv_coord_t list_focus_outline_pad() { return static_cast<lv_coord_t>(Interaction::FocusOutlinePad); }

/* Opacidad del separador de listas */
namespace List { inline constexpr lv_opa_t DividerOpa = LV_OPA_40; } // si no existía
inline lv_opa_t list_divider_opa() { return List::DividerOpa; }


/* ────────────────────────────  BUTTON  ──────────────────────────── */

// Medidas del botón (px)
namespace Button {
    inline constexpr int Width   = 160;
    inline constexpr int Height  = 44;
    inline constexpr int Radius  = 12;
    inline constexpr int PadLR   = 16;
    inline constexpr int PadTB   = 8;
    inline constexpr int IconGap = 8;
}
// Wrappers (apoyados en la paleta global)
inline lv_color_t button_primary_bg()         { return color_primary(); }
inline lv_color_t button_primary_bg_pressed() { return lv_color_darken(button_primary_bg(), 20); }
inline lv_color_t button_primary_bg_focused() { return lv_color_lighten(button_primary_bg(), 20); }
inline lv_color_t button_primary_text()       { return color_on_primary(); }
// Secundario / Ghost
inline lv_color_t button_secondary_text()     { return color_on_secondary(); }
inline lv_color_t button_ghost_text()         { return color_on_surface(); }

inline lv_coord_t button_width()    { return static_cast<lv_coord_t>(Button::Width); }
inline lv_coord_t button_height()   { return static_cast<lv_coord_t>(Button::Height); }
inline lv_coord_t button_radius()   { return static_cast<lv_coord_t>(Button::Radius); }
inline lv_coord_t button_pad_lr()   { return static_cast<lv_coord_t>(Button::PadLR); }
inline lv_coord_t button_pad_tb()   { return static_cast<lv_coord_t>(Button::PadTB); }
inline lv_coord_t button_icon_gap() { return static_cast<lv_coord_t>(Button::IconGap); }

/* ─────────────────────  LABELS & TYPOGRAPHY  ───────────────────── */

inline lv_color_t label_primary_color()   { return color_on_surface(); }
inline lv_color_t label_secondary_color() { return color_muted(); }

namespace TextEmphasis {
    inline constexpr lv_opa_t High     = LV_OPA_COVER; // 100%
    inline constexpr lv_opa_t Medium   = LV_OPA_70;    // ~70%
    inline constexpr lv_opa_t Disabled = LV_OPA_40;    // ~40%
}

// Escala semántica (guía; LVGL usa fuentes compiladas)
namespace TypographyPx {
    inline constexpr int Title    = 14;
    inline constexpr int Body     = 12;
    inline constexpr int Caption  = 8;
    inline constexpr int Subtitle = 13; // opcional
    inline constexpr int Overline = 10; // opcional
}

/* ───────────────────────────  FONTS  ────────────────────────────
 * Fuente de verdad en Tokens. Si compilas Montserrat *_lat1*:
 *   target_compile_definitions(${COMPONENT_LIB} PRIVATE UI_HAVE_MONTSERRAT_LAT1)
 */
#if defined(UI_HAVE_MONTSERRAT_LAT1)
extern "C" {
    LV_FONT_DECLARE(montserrat_14_lat1)
    LV_FONT_DECLARE(montserrat_12_lat1)
    LV_FONT_DECLARE(montserrat_8_lat1)
}
inline const lv_font_t* font_title()   { return &montserrat_14_lat1; }
inline const lv_font_t* font_body()    { return &montserrat_12_lat1; }
inline const lv_font_t* font_caption() { return &montserrat_8_lat1;  }
#else
inline const lv_font_t* font_title()   { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_body()    { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_caption() { return LV_FONT_DEFAULT; }
#endif

// Compatibilidad y consistencia
inline const lv_font_t* font_text() { return font_body(); } // usado por Button

/* Icon font (opcional).
 * Si la tienes, declara el símbolo y define UI_HAVE_ICON_FONT en CMake:
 *   target_compile_definitions(${COMPONENT_LIB} PRIVATE UI_HAVE_ICON_FONT)
 */
#if defined(UI_HAVE_ICON_FONT)
extern "C" { LV_FONT_DECLARE(ui_icon_font_16) } // ajusta el nombre a tu font real
inline const lv_font_t* font_icon() { return &ui_icon_font_16; }
#else
inline const lv_font_t* font_icon() { return LV_FONT_DEFAULT; }
#endif

}} // namespace Ui::Tokens
