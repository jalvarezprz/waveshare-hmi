#include "ui_view_welcome.h"
#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_label.h"
#include "ui/component/ui_component_button.h"

namespace Ui {

void ui_view_welcome_mount(UiLayoutScaffold& scaffold)
{
    themeInitOnce();                 // seguro idempotente
    auto& styles = getThemeStyles();

    // Header: título sencillo
    if (auto* hdr = scaffold.header()) {
        // Limpia hijos previos si los hubiera
        while (lv_obj_get_child_cnt(hdr)) lv_obj_del(lv_obj_get_child(hdr, 0));

        UiLabel title("PLC HVAC — Panel local", LabelVariant::Title);
        lv_obj_t* lbl = title.create(hdr);
        lv_obj_center(lbl);
    }

    // Content: tarjeta con texto + botones
    lv_obj_t* parent = scaffold.content();
    if (!parent) return;

    // Limpia hijos previos
    while (lv_obj_get_child_cnt(parent)) lv_obj_del(lv_obj_get_child(parent, 0));

    // Contenedor "card"
    lv_obj_t* card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(92));
    lv_obj_set_height(card, LV_SIZE_CONTENT);
    lv_obj_center(card);
    lv_obj_add_style(card, &styles.card, LV_PART_MAIN);
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(card, 8, LV_PART_MAIN);

    // Texto de bienvenida
    {
        UiLabel h1("Bienvenido a la HMI del PLC HVAC", LabelVariant::Title);
        lv_obj_t* l1 = h1.create(card);

        UiLabel body("Esta vista valida el layout Header/Content/Footer,\nel theme y los componentes base.", LabelVariant::Body);
        lv_obj_t* l2 = body.create(card);
        (void)l1; (void)l2;
    }

    // Fila de botones
    lv_obj_t* row = lv_obj_create(card);
    lv_obj_set_width(row, LV_PCT(100));
    lv_obj_set_height(row, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(row, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Botones (callbacks opcionales)
    auto onPrimary = [](lv_event_t* e) {
        (void)e;
        // Aquí podrías navegar con tu router, p.ej.: ui_router_go(UiScreen::MAIN_MENU);
    };
    auto onGhost = [](lv_event_t* e) {
        (void)e;
        // Acción secundaria (p.ej. abrir ajustes)
    };

    {
        UiButton btnPrimary("Entrar", ButtonRole::Primary, onPrimary);
        lv_obj_t* b1 = btnPrimary.create(row);
        (void)b1;

        UiButton btnGhost("Ajustes", ButtonRole::Ghost, onGhost);
        lv_obj_t* b2 = btnGhost.create(row);
        (void)b2;
    }
}

} // namespace Ui
