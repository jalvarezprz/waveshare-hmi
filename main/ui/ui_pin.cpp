#include "ui_pin.h"
#include "lvgl.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <functional>

static int g_expected = 0;
static std::function<void(bool)> g_callback;
static std::string g_entered;
static lv_obj_t* g_label = nullptr;

void ui_show_pin_dialog(int correct_pin, std::function<void(bool)> cb) {
    g_expected = correct_pin;
    g_callback = cb;
    g_entered.clear();

    // Contenedor principal
    lv_obj_t* dlg = lv_obj_create(lv_scr_act());
    lv_obj_set_size(dlg, 400, 250);
    lv_obj_center(dlg);
    lv_obj_set_flex_flow(dlg, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(dlg, 12, 0);
    lv_obj_set_style_pad_row(dlg, 12, 0);

    lv_obj_t* title = lv_label_create(dlg);
    lv_label_set_text(title, "Introducir clave");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(title, LV_PCT(100));

    // Label de PIN oculto
    g_label = lv_label_create(dlg);
    lv_label_set_text(g_label, "----");
    lv_obj_set_style_text_font(g_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(g_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(g_label, LV_PCT(100));

    // Grid de botones numéricos
    lv_obj_t* grid = lv_obj_create(dlg);
    lv_obj_set_size(grid, LV_PCT(100), 140);
    lv_obj_set_flex_flow(grid, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(grid,
        LV_FLEX_ALIGN_SPACE_EVENLY,
        LV_FLEX_ALIGN_CENTER,
        LV_FLEX_ALIGN_CENTER);

    auto digit_cb = [](lv_event_t* e){
        int digit = (int)(intptr_t) lv_event_get_user_data(e);
        g_entered.push_back('0' + digit);
        if (g_entered.size() > 4) g_entered.erase(0, g_entered.size() - 4);

        std::string mask(g_entered.size(), '*');
        lv_label_set_text(g_label, mask.c_str());

        if (g_entered.size() == 4) {
            bool ok = (std::stoi(g_entered) == g_expected);
            if (g_callback) g_callback(ok);
            lv_obj_del_async(lv_obj_get_parent(g_label)); // eliminar diálogo
        }
    };

    for (int i = 0; i <= 9; i++) {
        lv_obj_t* btn = lv_btn_create(grid);
        lv_obj_set_size(btn, 70, 50);
        lv_obj_t* l = lv_label_create(btn);
        char buf[2]; sprintf(buf, "%d", i);
        lv_label_set_text(l, buf);
        lv_obj_center(l);

        lv_obj_add_event_cb(btn, digit_cb, LV_EVENT_CLICKED, (void*)(intptr_t)i);
    }

    // Botón cancelar
    lv_obj_t* cancel = lv_btn_create(dlg);
    lv_obj_set_size(cancel, 100, 40);
    lv_obj_t* l = lv_label_create(cancel);
    lv_label_set_text(l, "Cancelar");
    lv_obj_center(l);
    lv_obj_add_event_cb(cancel, [](lv_event_t* e){
        lv_obj_del_async(lv_obj_get_parent(lv_event_get_target(e)));
        if (g_callback) g_callback(false);
    }, LV_EVENT_CLICKED, nullptr);
}
