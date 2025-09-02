#pragma once
#include "lvgl.h"

namespace Ui { namespace Tokens {

/** Colores para botón primario — derivados de Ui::UiThemeTokens */
lv_color_t button_primary_bg();          
lv_color_t button_primary_bg_pressed();  
lv_color_t button_primary_bg_focused();  
lv_color_t button_primary_text();        

/** Fuentes usadas por el botón */
const lv_font_t* font_icon();   
const lv_font_t* font_text();   

/** Medidas del botón */
lv_coord_t button_width();      
lv_coord_t button_height();     
lv_coord_t button_radius();     
lv_coord_t button_pad_lr();     
lv_coord_t button_pad_tb();     
lv_coord_t button_icon_gap();   

}} // namespace Ui::Tokens
