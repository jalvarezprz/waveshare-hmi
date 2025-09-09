/**
 * @file ui_component_button.cpp
 * @brief Implementacion del componente Button (creacion y mutadores).
 * @ingroup ui_component_button
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_button.h"
#include "ui/component/ui_component_common.h"
#include "ui/ui_router.h"   // NUEVO: para Router::dispatch(action)
#include "lvgl.h"
#include <cstring>
#include <new>   // placement new

namespace Ui::Component::Button {

struct _CbHolder {
    Callbacks cb{};
    void*     userData = nullptr;
    const char* action = nullptr;  ///< NUEVO: acción declarativa opcional
    _CbHolder() = default;
    ~_CbHolder() = default;
};

/**
 * @brief Aplica la variante visual seleccionada usando ThemeStyles.
 */
static void apply_variant(lv_obj_t* btn, UiThemeStyles& s, Variant v, bool setSize) {
    switch (v) {
        case Variant::Primary:     Ui::applyButtonPrimary    (btn, s, setSize); break;
        case Variant::Secondary:   Ui::applyButtonSecondary  (btn, s, setSize); break;
        case Variant::Ghost:       Ui::applyButtonGhost      (btn, s, setSize); break;
        case Variant::Destructive: Ui::applyButtonDestructive(btn, s, setSize); break;
        case Variant::Success:     Ui::applyButtonSuccess    (btn, s, setSize); break;
        case Variant::Warning:     Ui::applyButtonWarning    (btn, s, setSize); break;
    }
}

/**
 * @copydoc Ui::Component::Button::create
 *
 * Prioridad de callbacks:
 *  - Si Props::action está definido → se usa Router::dispatch(action).
 *  - Si no hay action → se usan los callbacks clásicos (onClick, onToggle, onLong).
 */
Handle create(lv_obj_t* parent, UiThemeStyles& styles, const Props& p, const Callbacks& cb) {
    Ui::themeInitOnce();

    Handle h{};
    h.root = lv_btn_create(parent);

    // Flags y estados iniciales
    if (p.toggle)   lv_obj_add_flag (h.root, LV_OBJ_FLAG_CHECKABLE);
    if (!p.enabled) lv_obj_add_state(h.root, LV_STATE_DISABLED);
    if (p.checked)  lv_obj_add_state(h.root, LV_STATE_CHECKED);

    // Variante visual
    apply_variant(h.root, styles, p.variant, /*setSize=*/true);

    // Hijos: icono y texto (orden segun iconPos)
    if (p.icon && p.icon[0] && p.iconPos != IconPos::None) {
        h.icon = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.icon, styles.tokens.fontIcon, LV_PART_MAIN);
        lv_label_set_text(h.icon, p.icon);
    }
    if (p.text && p.text[0] && p.iconPos != IconPos::Only) {
        h.label = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.label, styles.tokens.fontBody, LV_PART_MAIN);
        lv_label_set_text(h.label, p.text);
    }

    // Layout interno (fila por defecto en el componente)
    lv_obj_set_flex_flow(h.root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_gap(h.root, styles.tokens.btnIconGap, LV_PART_MAIN);
    if (h.icon && h.label && p.iconPos == IconPos::Right) {
        lv_obj_move_to_index(h.label, 0); // texto primero
    }

    // Loading
    if (p.loading) {
        lv_obj_add_state(h.root, LV_STATE_DISABLED);
        // (opcional: spinner)
    }

    // --- Callbacks / Action ---
    _CbHolder* holder = nullptr;
    if (p.action || cb.onClick || cb.onLong || (p.toggle && cb.onToggle)) {
        void* mem = lv_mem_alloc(sizeof(_CbHolder));
        if (mem) {
            holder = new (mem) _CbHolder();
            holder->cb       = cb;
            holder->userData = p.userData;
            holder->action   = p.action; // NUEVO
        }
    }

    if (holder) {
        // Liberación: destructor + free
        lv_obj_add_event_cb(h.root, [](lv_event_t* e){
            auto* ud = static_cast<_CbHolder*>(lv_event_get_user_data(e));
            if (ud) {
                ud->~_CbHolder();
                lv_mem_free(ud);
            }
        }, LV_EVENT_DELETE, holder);
    }

    // Eventos
    if (holder) {
        // Click
        lv_obj_add_event_cb(h.root, [](lv_event_t* e){
            auto* obj = lv_event_get_target(e);
            auto* ud  = static_cast<_CbHolder*>(lv_event_get_user_data(e));
            if (!ud) return;
            Handle hh{ obj, nullptr, nullptr };
            if (ud->action && *ud->action) {
                //Ui::Router::dispatch(ud->action);
            } else if (ud->cb.onClick) {
                ud->cb.onClick(hh, ud->userData);
            }
        }, LV_EVENT_CLICKED, holder);

        // Toggle
        if (p.toggle) {
            lv_obj_add_event_cb(h.root, [](lv_event_t* e){
                auto* obj = lv_event_get_target(e);
                auto* ud  = static_cast<_CbHolder*>(lv_event_get_user_data(e));
                if (!ud) return;
                Handle hh{ obj, nullptr, nullptr };
                bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
                if (ud->cb.onToggle) ud->cb.onToggle(hh, checked, ud->userData);
            }, LV_EVENT_VALUE_CHANGED, holder);
        }

        // Long press
        if (cb.onLong) {
            lv_obj_add_event_cb(h.root, [](lv_event_t* e){
                auto* obj = lv_event_get_target(e);
                auto* ud  = static_cast<_CbHolder*>(lv_event_get_user_data(e));
                if (!ud) return;
                Handle hh{ obj, nullptr, nullptr };
                ud->cb.onLong(hh, ud->userData);
            }, LV_EVENT_LONG_PRESSED, holder);
        }
    }

    return h;
}

void setEnabled(Handle& h, UiThemeStyles&, bool enabled) {
    if (!h.root) return;
    if (enabled) lv_obj_clear_state(h.root, LV_STATE_DISABLED);
    else         lv_obj_add_state  (h.root, LV_STATE_DISABLED);
}

void setChecked(Handle& h, UiThemeStyles&, bool checked) {
    if (!h.root) return;
    if (checked) lv_obj_add_state(h.root, LV_STATE_CHECKED);
    else         lv_obj_clear_state(h.root, LV_STATE_CHECKED);
}

void setLoading(Handle& h, UiThemeStyles&, bool loading) {
    if (!h.root) return;
    if (loading) lv_obj_add_state(h.root, LV_STATE_DISABLED);
    else         lv_obj_clear_state(h.root, LV_STATE_DISABLED);
}

void setText(Handle& h, UiThemeStyles& s, const char* txt) {
    if (!h.label) {
        h.label = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.label, s.tokens.fontBody, LV_PART_MAIN);
    }
    lv_label_set_text(h.label, txt ? txt : "");
}

void setIcon(Handle& h, UiThemeStyles& s, const char* icon, IconPos pos) {
    if (!icon || !icon[0]) { if (h.icon) { lv_obj_del(h.icon); h.icon=nullptr; } return; }
    if (!h.icon) {
        h.icon = lv_label_create(h.root);
        lv_obj_set_style_text_font(h.icon, s.tokens.fontIcon, LV_PART_MAIN);
    }
    lv_label_set_text(h.icon, icon);
    if (h.icon && h.label) {
        if (pos == IconPos::Right) lv_obj_move_to_index(h.label, 0);
        else                       lv_obj_move_to_index(h.icon, 0);
    }
}

bool isChecked(const Handle& h) { return h.root && lv_obj_has_state(h.root, LV_STATE_CHECKED); }
void focus(Handle& h)           { if (h.root) lv_group_focus_obj(h.root); }

} // namespace Ui::Component::Button
