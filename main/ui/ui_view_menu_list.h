#pragma once
struct cJSON;
struct _lv_obj_t;
typedef _lv_obj_t lv_obj_t;

cJSON* loadMenu();
void   printMenu();

// Construye el menú de primer nivel en la pantalla activa
void ui_build_main_menu();
void ui_build_info_menu();