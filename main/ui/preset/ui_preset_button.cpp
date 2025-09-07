/**
 * @file ui_preset_button.cpp
 * @brief Implementación del preset "MenuButton" basado en Ui::Component::Button.
 */

#include "ui/preset/ui_preset_button.h"
#include "ui/component/ui_component_button.h"
#include "ui/theme/ui_theme_styles.h"

#include <cstring>
#include <cstdio>

namespace Ui::Preset::ButtonMenu {

/*===================== Helpers =====================*/

/** Normaliza un identificador "lv:xxx" a LV_SYMBOL_XXX; si no, usa el literal. */
static const char* normalize_icon_text(const char* iconId)
{
    if (!iconId || !*iconId) return LV_SYMBOL_SETTINGS;

    if (std::strncmp(iconId, "lv:", 3) == 0) {
        const char* name = iconId + 3;
        if      (std::strcmp(name, "home")     == 0) return LV_SYMBOL_HOME;
        else if (std::strcmp(name, "settings") == 0) return LV_SYMBOL_SETTINGS;
        else if (std::strcmp(name, "power")    == 0) return LV_SYMBOL_POWER;
        else if (std::strcmp(name, "warning")  == 0) return LV_SYMBOL_WARNING;
        else if (std::strcmp(name, "ok")       == 0) return LV_SYMBOL_OK;
        else if (std::strcmp(name, "play")     == 0) return LV_SYMBOL_PLAY;
        else if (std::strcmp(name, "stop")     == 0) return LV_SYMBOL_STOP;
        else if (std::strcmp(name, "pause")    == 0) return LV_SYMBOL_PAUSE;
        else if (std::strcmp(name, "up")       == 0) return LV_SYMBOL_UP;
        else if (std::strcmp(name, "down")     == 0) return LV_SYMBOL_DOWN;
        else if (std::strcmp(name, "left")     == 0) return LV_SYMBOL_LEFT;
        else if (std::strcmp(name, "right")    == 0) return LV_SYMBOL_RIGHT;
        else if (std::strcmp(name, "refresh")  == 0) return LV_SYMBOL_REFRESH;
        else if (std::strcmp(name, "edit")     == 0) return LV_SYMBOL_EDIT;
        else if (std::strcmp(name, "trash")    == 0) return LV_SYMBOL_TRASH;
        return LV_SYMBOL_SETTINGS;
    }
    return iconId;
}

/** Ajusta el layout vertical del botón base y centra contenido. */
static void apply_menu_layout_column(Ui::Component::Button::Handle& base, UiThemeStyles& s)
{
    lv_obj_t* root = base.root;
    if (!root) return;

    lv_obj_set_flex_flow (root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(root, s.tokens.btnIconGap, LV_PART_MAIN);

    if (s.tokens.minTouch > 0) {
        lv_obj_set_style_min_height(root, s.tokens.minTouch, LV_PART_MAIN);
    }

    if (base.label) {
        lv_obj_set_style_text_align(base.label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_long_mode(base.label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(base.label, LV_PCT(100));
    }
    if (base.icon) {
        lv_obj_set_style_text_align(base.icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    }
}

/** Crea/actualiza el badge. */
static void build_or_update_badge(Handle& h, UiThemeStyles& s, int count, bool showDot)
{
    if (count < 0 && !showDot) {
        if (h.badge) { lv_obj_del(h.badge); h.badge = nullptr; }
        return;
    }

    if (!h.badge) {
        h.badge = lv_label_create(h.base.root);
        lv_obj_set_style_bg_color  (h.badge, s.tokens.colorPrimary,   LV_PART_MAIN);
        lv_obj_set_style_bg_opa    (h.badge, s.tokens.opaEnabled,     LV_PART_MAIN);
        lv_obj_set_style_text_color(h.badge, s.tokens.colorOnPrimary, LV_PART_MAIN);
        lv_obj_set_style_pad_hor   (h.badge, s.tokens.badgePadH,      LV_PART_MAIN);
        lv_obj_set_style_pad_ver   (h.badge, s.tokens.badgePadV,      LV_PART_MAIN);
        lv_obj_set_style_radius    (h.badge, s.tokens.badgeRadius,    LV_PART_MAIN);
    }

    if (count >= 0) {
        if (count > 99) {
            lv_label_set_text(h.badge, "99+");
        } else {
            char buf[8];
            std::snprintf(buf, sizeof(buf), "%d", count);
            lv_label_set_text(h.badge, buf);
        }
    } else {
        lv_label_set_text(h.badge, "•");
    }

    lv_obj_align(h.badge, LV_ALIGN_TOP_RIGHT,
                 s.tokens.badgeOffsetX, s.tokens.badgeOffsetY);
}

/** Crea/actualiza el hint. */
static void build_or_update_hint(Handle& h, UiThemeStyles& s, const char* hint)
{
    if (!hint || !*hint) {
        if (h.hintLbl) { lv_obj_del(h.hintLbl); h.hintLbl = nullptr; }
        return;
    }
    if (!h.hintLbl) {
        h.hintLbl = lv_label_create(h.base.root);
        lv_obj_set_style_text_align(h.hintLbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(h.hintLbl, s.tokens.colorMuted,  LV_PART_MAIN);
        lv_obj_set_style_text_font (h.hintLbl, s.tokens.fontCaption, LV_PART_MAIN);
    }
    lv_label_set_text(h.hintLbl, hint);
    if (h.base.label) lv_obj_move_to_index(h.hintLbl, lv_obj_get_index(h.base.label) + 1);
}

/*===================== API pública =====================*/

Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p)
{
    Ui::themeInitOnce();

    // 0) Declarar el Handle del preset ANTES de usarlo
    Handle h{};  // <<< ESTA LÍNEA ES IMPRESCINDIBLE

    // 1) Props del componente base
    Ui::Component::Button::Props bp{};
    bp.text     = p.text;
    bp.icon     = normalize_icon_text(p.iconId);
    bp.variant  = p.variant;
    bp.size     = p.size;
    bp.iconPos  = Ui::Component::Button::IconPos::Left;
    bp.enabled  = !p.disabled;
    bp.toggle   = p.selected;
    bp.checked  = p.selected;
    bp.loading  = p.loading;
    // (si tu componente base NO tiene .action, deja esto comentado)
    // bp.action   = p.action;

    // 2) Callbacks a pasar al componente base
    Ui::Component::Button::Callbacks cb = p.callbacks;

    // (opcional) Si quieres que p.action dispare el router:
    // #include "ui/router/ui_router.h" arriba
    // if (p.action && *p.action) {
    //     cb.onClick = [act = p.action](Ui::Component::Button::Handle&, void*) {
    //         Ui::Router::dispatch(act);
    //     };
    // }

    // 3) Crear el componente base
    h.base = Ui::Component::Button::create(parent, s, bp, cb);

    // 4) Ajustes del preset
    apply_menu_layout_column(h.base, s);
    build_or_update_hint (h, s, p.hint);
    build_or_update_badge(h, s, p.badgeCount, p.showDot);

    Ui::Component::Button::setEnabled(h.base, s, !p.disabled);
    Ui::Component::Button::setChecked (h.base, s,  p.selected);

    return h;
}

bool setText(Handle& h, UiThemeStyles& s, const char* text)
{
    Ui::Component::Button::setText(h.base, s, text);
    if (h.hintLbl && h.base.label) {
        lv_obj_move_to_index(h.hintLbl, lv_obj_get_index(h.base.label) + 1);
    }
    return true;
}

bool setIcon(Handle& h, UiThemeStyles& s, const char* iconId)
{
    Ui::Component::Button::setIcon(h.base, s, normalize_icon_text(iconId),
                                   Ui::Component::Button::IconPos::Left);
    apply_menu_layout_column(h.base, s);
    return true;
}

bool setHint(Handle& h, UiThemeStyles& s, const char* hint)
{
    build_or_update_hint(h, s, hint);
    return true;
}

bool setBadge(Handle& h, UiThemeStyles& s, int count, bool showDot)
{
    build_or_update_badge(h, s, count, showDot);
    return true;
}

bool setSelected(Handle& h, UiThemeStyles& s, bool v)
{
    Ui::Component::Button::setChecked(h.base, s, v);
    return true;
}

bool setDisabled(Handle& h, UiThemeStyles& s, bool v)
{
    Ui::Component::Button::setEnabled(h.base, s, !v);
    return true;
}

bool setVariant(Handle& h, UiThemeStyles& s, Variant v)
{
    lv_obj_t* root = h.base.root;
    if (!root) return false;

    switch (v) {
        case Variant::Primary:     Ui::applyButtonPrimary    (root, s, false); break;
        case Variant::Secondary:   Ui::applyButtonSecondary  (root, s, false); break;
        case Variant::Ghost:       Ui::applyButtonGhost      (root, s, false); break;
        case Variant::Destructive: Ui::applyButtonDestructive(root, s, false); break;
        case Variant::Success:     Ui::applyButtonSuccess    (root, s, false); break;
        case Variant::Warning:     Ui::applyButtonWarning    (root, s, false); break;
    }
    apply_menu_layout_column(h.base, s);
    return true;
}

bool setSize(Handle& h, UiThemeStyles& s, Size /*sz*/)
{
    apply_menu_layout_column(h.base, s);
    return true;
}

void focus(Handle& h) { Ui::Component::Button::focus(h.base); }

} // namespace Ui::Preset::ButtonMenu
