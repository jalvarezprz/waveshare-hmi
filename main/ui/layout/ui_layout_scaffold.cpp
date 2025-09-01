#include "ui_layout_scaffold.h"

namespace Ui {

void UiLayoutScaffold::build(lv_obj_t* root) {
    if (!root) return;

    // Asegura que el theme está inicializado (idempotente)
    Ui::themeInitOnce();

    if (!header_)  header_  = lv_obj_create(root);
    if (!content_) content_ = lv_obj_create(root);
    if (!footer_)  footer_  = lv_obj_create(root);

    applyLayout_(root);
    applyStyles_();
}

void UiLayoutScaffold::setBarsHeight(int16_t headerHeight, int16_t footerHeight) {
    headerHeightPx_ = headerHeight;
    footerHeightPx_ = footerHeight;

    if (header_) lv_obj_set_height(header_, headerHeightPx_);
    if (footer_) lv_obj_set_height(footer_, footerHeightPx_);
    // Content ajusta por flex-grow; no necesita altura fija
}

void UiLayoutScaffold::applyLayout_(lv_obj_t* root) {
    // Root ocupa toda la pantalla y usa flex en columna
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Header: altura fija
    lv_obj_set_width(header_, LV_PCT(100));
    lv_obj_set_height(header_, headerHeightPx_);

    // Content: crece para ocupar el espacio entre header y footer
    lv_obj_set_width(content_, LV_PCT(100));
    lv_obj_set_flex_grow(content_, 1);

    // Footer: altura fija
    lv_obj_set_width(footer_, LV_PCT(100));
    lv_obj_set_height(footer_, footerHeightPx_);
}

void UiLayoutScaffold::applyStyles_() {
    // Aplica estilos del theme a cada contenedor
    auto& styles = Ui::getThemeStyles();
    Ui::applyHeader(header_,  styles);
    Ui::applyContent(content_, styles);
    Ui::applyFooter(footer_,  styles);
}

} // namespace Ui
