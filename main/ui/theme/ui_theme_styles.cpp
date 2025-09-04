#include "ui_theme_styles.h"
#include "ui_theme_tokens.h"   // <— usamos Ui::Tokens

namespace Ui {

static UiThemeTokens  g_tokens;
static UiThemeStyles  g_styles;

UiThemeStyles&       getThemeStyles()       { return g_styles; }
const UiThemeTokens& getThemeTokens()       { return g_tokens; }

static void style_reset(lv_style_t& s) {
    if (s.prop_cnt) lv_style_reset(&s);
    else            lv_style_init(&s);
}

/* --------------------------------------------------------------------------
 * Tokens (valores atómicos) iniciales del Theme “legacy”.
 * Aquí mapeamos los valores a partir de Ui::Tokens para evitar duplicación.
 * --------------------------------------------------------------------------*/
UiThemeTokens makeDefaultTokens() {
    UiThemeTokens t{};

    // === Paleta base desde Tokens (core + completa) ===
    t.colorBg        = Ui::Tokens::color_surface();          // fondo base
    t.colorSurface   = Ui::Tokens::color_surface_variant();  // cards/containers
    t.colorText      = Ui::Tokens::color_on_surface();       // texto principal

    // Paleta completa centralizada en Tokens
    t.colorPrimary   = Ui::Tokens::color_primary();
    t.colorSecondary = Ui::Tokens::color_secondary();
    t.colorMuted     = Ui::Tokens::color_muted();
    t.colorSuccess   = Ui::Tokens::color_success();
    t.colorWarning   = Ui::Tokens::color_warning();
    t.colorError     = Ui::Tokens::color_error();

    // === Opacidades ===
    t.opaEnabled  = LV_OPA_COVER;
    t.opaDisabled = Ui::Tokens::list_disabled_opa(); // coherente con “disabled” en listas

    // === Radios y spacing (desde Tokens) ===
    t.radiusSm = static_cast<uint16_t>(Ui::Tokens::Radius::sm);
    t.radiusMd = static_cast<uint16_t>(Ui::Tokens::Radius::md);
    t.radiusLg = static_cast<uint16_t>(Ui::Tokens::Radius::lg);

    t.spaceXs  = static_cast<uint8_t>(Ui::Tokens::Space::xs);
    t.spaceSm  = static_cast<uint8_t>(Ui::Tokens::Space::sm);
    t.spaceMd  = static_cast<uint8_t>(Ui::Tokens::Space::md);
    t.spaceLg  = static_cast<uint8_t>(Ui::Tokens::Space::lg);

    // === Tipografías === (usar helpers de tokens; resuelven condicionales)
    t.fontTitle   = Ui::Tokens::font_title();
    t.fontBody    = Ui::Tokens::font_body();
    t.fontCaption = Ui::Tokens::font_caption();

    // === Alturas estándar para listas ===
    t.itemHeightMd = static_cast<uint16_t>(Ui::Tokens::list_item_height_md());
    t.itemHeightLg = static_cast<uint16_t>(Ui::Tokens::list_item_height_lg());

    return t;
}

/* --------------------------------------------------------------------------
 * Estilos (recetas) construidos a partir de los tokens anteriores.
 * --------------------------------------------------------------------------*/
void initThemeStyles(UiThemeStyles& s, const UiThemeTokens& t) {
    // ===== Base =====
    style_reset(s.base);
    lv_style_set_bg_color  (&s.base, t.colorBg);
    lv_style_set_text_color(&s.base, t.colorText);

    // ===== Header (panel) =====
    style_reset(s.header);
    lv_style_set_bg_color    (&s.header, Ui::Tokens::panel_bg_color());
    lv_style_set_text_color  (&s.header, Ui::Tokens::panel_text_color());
    lv_style_set_pad_all     (&s.header, Ui::Tokens::panel_pad_all());
    lv_style_set_border_width(&s.header, Ui::Tokens::panel_border_w());
    lv_style_set_border_color(&s.header, Ui::Tokens::panel_border_color());

    // ===== Content =====
    style_reset(s.content);
    lv_style_set_bg_color(&s.content, t.colorBg);
    lv_style_set_pad_all (&s.content, t.spaceMd);

    // ===== Footer (panel) =====
    style_reset(s.footer);
    lv_style_set_bg_color    (&s.footer, Ui::Tokens::panel_bg_color());
    lv_style_set_text_color  (&s.footer, Ui::Tokens::panel_text_color());
    lv_style_set_pad_all     (&s.footer, Ui::Tokens::panel_pad_all());
    lv_style_set_border_width(&s.footer, Ui::Tokens::panel_border_w());
    lv_style_set_border_color(&s.footer, Ui::Tokens::panel_border_color());

    // ===== Card =====
    style_reset(s.card);
    lv_style_set_bg_color     (&s.card, Ui::Tokens::card_bg_color());
    lv_style_set_text_color   (&s.card, Ui::Tokens::card_text_color());
    lv_style_set_radius       (&s.card, Ui::Tokens::card_radius());
    lv_style_set_pad_all      (&s.card, Ui::Tokens::card_pad_all());
    lv_style_set_border_width (&s.card, Ui::Tokens::card_border_w());
    lv_style_set_border_color (&s.card, Ui::Tokens::card_border_color());
    lv_style_set_shadow_width (&s.card, Ui::Tokens::card_shadow_w());
    lv_style_set_shadow_ofs_y (&s.card, Ui::Tokens::card_shadow_ofs_y());
    lv_style_set_shadow_color (&s.card, Ui::Tokens::card_shadow_color());

    // ===== Labels =====
    style_reset(s.labelTitle);
    lv_style_set_text_font (&s.labelTitle, t.fontTitle);
    lv_style_set_text_color(&s.labelTitle, t.colorText);

    style_reset(s.labelBody);
    lv_style_set_text_font (&s.labelBody, t.fontBody);
    lv_style_set_text_color(&s.labelBody, t.colorText);

    style_reset(s.labelCaption);
    lv_style_set_text_font (&s.labelCaption, t.fontCaption);
    lv_style_set_text_color(&s.labelCaption, t.colorMuted);

    // ===== Botones =====
    style_reset(s.btnPrimary);
    lv_style_set_bg_color (&s.btnPrimary, t.colorPrimary);
    lv_style_set_bg_opa   (&s.btnPrimary, t.opaEnabled);
    lv_style_set_radius   (&s.btnPrimary, t.radiusMd);
    lv_style_set_pad_hor  (&s.btnPrimary, t.spaceLg);
    lv_style_set_pad_ver  (&s.btnPrimary, t.spaceSm);
    lv_style_set_text_color(&s.btnPrimary, Ui::Tokens::color_on_primary());

    style_reset(s.btnSecondary);
    lv_style_set_bg_color (&s.btnSecondary, t.colorSecondary);
    lv_style_set_bg_opa   (&s.btnSecondary, t.opaEnabled);
    lv_style_set_radius   (&s.btnSecondary, t.radiusMd);
    lv_style_set_pad_hor  (&s.btnSecondary, t.spaceLg);
    lv_style_set_pad_ver  (&s.btnSecondary, t.spaceSm);
    lv_style_set_text_color(&s.btnSecondary, Ui::Tokens::color_on_secondary());

    style_reset(s.btnGhost);
    lv_style_set_bg_opa     (&s.btnGhost, LV_OPA_TRANSP);
    lv_style_set_border_width(&s.btnGhost, Ui::Tokens::panel_border_w());
    lv_style_set_border_color(&s.btnGhost, Ui::Tokens::panel_border_color());
    lv_style_set_radius     (&s.btnGhost, t.radiusMd);
    lv_style_set_pad_hor    (&s.btnGhost, t.spaceLg);
    lv_style_set_pad_ver    (&s.btnGhost, t.spaceSm);
    lv_style_set_text_color (&s.btnGhost, Ui::Tokens::label_primary_color());

    // ===== Listas =====
    style_reset(s.listContainer);
    lv_style_set_bg_color (&s.listContainer, Ui::Tokens::list_bg_color());
    lv_style_set_pad_ver  (&s.listContainer, Ui::Tokens::list_pad_tb());
    lv_style_set_pad_hor  (&s.listContainer, Ui::Tokens::list_pad_lr());

    style_reset(s.listItem);
    lv_style_set_bg_color  (&s.listItem, Ui::Tokens::list_item_bg_color());
    lv_style_set_bg_opa    (&s.listItem, t.opaEnabled);
    lv_style_set_radius    (&s.listItem, t.radiusSm);
    lv_style_set_pad_left  (&s.listItem, Ui::Tokens::list_pad_lr());
    lv_style_set_pad_right (&s.listItem, Ui::Tokens::list_pad_lr());
    lv_style_set_pad_top   (&s.listItem, Ui::Tokens::list_pad_tb());
    lv_style_set_pad_bottom(&s.listItem, Ui::Tokens::list_pad_tb());
    lv_style_set_text_color(&s.listItem, Ui::Tokens::list_text_color());
    lv_style_set_text_font (&s.listItem, t.fontBody);

    style_reset(s.listItemPressed);
    lv_style_set_bg_color(&s.listItemPressed,
        lv_color_mix(t.colorPrimary, t.colorSurface, Ui::Tokens::list_pressed_tint_opa()));

    style_reset(s.listItemFocused);
    lv_style_set_outline_width(&s.listItemFocused, Ui::Tokens::list_focus_outline_w());
    lv_style_set_outline_color(&s.listItemFocused, t.colorPrimary);
    lv_style_set_outline_pad (&s.listItemFocused, Ui::Tokens::list_focus_outline_pad());

    style_reset(s.listItemDisabled);
    lv_style_set_bg_opa    (&s.listItemDisabled, Ui::Tokens::list_disabled_opa());
    lv_style_set_text_color(&s.listItemDisabled, t.colorMuted);

    style_reset(s.listDivider);
    lv_style_set_border_width(&s.listDivider, Ui::Tokens::list_divider_width());
    lv_style_set_border_side (&s.listDivider, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&s.listDivider, Ui::Tokens::list_divider_color());
    lv_style_set_border_opa  (&s.listDivider, Ui::Tokens::list_divider_opa());

    s.initialized = true;
}

/* -------------------------------------------------------------------------- */
void applyHeader (lv_obj_t* obj, UiThemeStyles& s) { lv_obj_add_style(obj, &s.header,  LV_PART_MAIN); }
void applyContent(lv_obj_t* obj, UiThemeStyles& s) { lv_obj_add_style(obj, &s.content, LV_PART_MAIN); }
void applyFooter (lv_obj_t* obj, UiThemeStyles& s) { lv_obj_add_style(obj, &s.footer,  LV_PART_MAIN); }

void themeInitOnce() {
    if (!g_styles.initialized) {
        g_tokens = makeDefaultTokens();
        initThemeStyles(g_styles, g_tokens);
    }
}

// ===== Helpers de listas =====
static void set_row_height(lv_obj_t* obj, uint16_t h) {
    lv_obj_set_style_min_height(obj, h, LV_PART_MAIN);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_width(obj, LV_PCT(100));
}

void applyListContainer(lv_obj_t* obj, UiThemeStyles& s) {
    if (!obj) return;
    lv_obj_add_style(obj, &s.listContainer, LV_PART_MAIN);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(obj, Ui::Tokens::list_gap_row(), LV_PART_MAIN);
    lv_obj_set_width(obj, LV_PCT(100));
}

void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider) {
    if (!obj) return;
    const uint16_t H = static_cast<uint16_t>(large ? Ui::Tokens::list_item_height_lg()
                                                   : Ui::Tokens::list_item_height_md());

    lv_obj_add_style(obj, &s.listItem, LV_PART_MAIN);
    lv_obj_add_style(obj, &s.listItemPressed, LV_STATE_PRESSED);
    lv_obj_add_style(obj, &s.listItemFocused, LV_STATE_FOCUSED | LV_STATE_EDITED);
    lv_obj_add_style(obj, &s.listItemDisabled, LV_STATE_DISABLED);

    if (withDivider) {
        lv_obj_add_style(obj, &s.listDivider, LV_PART_MAIN);
    }

    set_row_height(obj, H);

    lv_obj_set_flex_flow (obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
}

void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider) {
    if (!parent) return;
    applyListContainer(parent, s);

    // Aplica a todos los hijos directos como ítems
    uint32_t cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        applyListItem(child, s, large, withDivider);
    }
}

} // namespace Ui
