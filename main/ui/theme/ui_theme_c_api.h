#pragma once
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void ui_theme_init_once_c(void);
void ui_apply_header_style(lv_obj_t* obj);
void ui_apply_content_style(lv_obj_t* obj);
void ui_apply_footer_style(lv_obj_t* obj);

#ifdef __cplusplus
}
#endif
