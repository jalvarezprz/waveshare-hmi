#include "lvgl.h"
#include "ui/widgets_reusable/widget_slider.h"
#include "ui/ui_screens.h"

static lv_obj_t* create_screen()
{
    lv_obj_t* scr = lv_obj_create(NULL);
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(scr, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_all(scr, 12, 0);
    return scr;
}

void ui_init_example()
{
    lv_obj_t* scr = create_screen();

    auto* sTemp = new WidgetSlider(scr, "Setpoint ACS", "°C", 20, 60);
    sTemp->setWidth(320);
    sTemp->setValue(45);
    sTemp->onValueChanged([](int v){
        printf("Setpoint ACS cambió a %d\n", v);
    });

    auto* sHum = new WidgetSlider(scr, "Humedad Objetivo", "%", 30, 80);
    sHum->setWidth(320);
    sHum->setValue(55);

    lv_scr_load(scr);
}
