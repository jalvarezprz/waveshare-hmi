#include "ui/theme/ui_theme_c_api.h"
#include "ui/theme/ui_theme_styles.h"

using namespace Ui;

extern "C" {

void ui_theme_init_once_c(void) {
    Ui::themeInitOnce();
}

void ui_apply_header_style(lv_obj_t* obj)  { Ui::applyHeader(obj,  Ui::getThemeStyles()); }
void ui_apply_content_style(lv_obj_t* obj) { Ui::applyContent(obj, Ui::getThemeStyles()); }
void ui_apply_footer_style(lv_obj_t* obj)  { Ui::applyFooter(obj,  Ui::getThemeStyles()); }

}
