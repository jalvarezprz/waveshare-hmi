#pragma once
struct cJSON;
struct _lv_obj_t;
typedef _lv_obj_t lv_obj_t;

cJSON* loadMenuMcr50();
void   printMenuMcr50();

// Construye el menú de primer nivel en la pantalla activa
void ui_mcr50_build_main_menu();
void ui_mcr50_build_info_menu();