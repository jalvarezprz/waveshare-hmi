#pragma once
#include "lvgl.h"

class Button {
public:
    Button() : btn_(nullptr), iconLabel_(nullptr), textLabel_(nullptr) {}
    Button(lv_obj_t* btn, lv_obj_t* icon, lv_obj_t* text)
        : btn_(btn), iconLabel_(icon), textLabel_(text) {}

    static Button create(lv_obj_t* parent,
                         const char* text,
                         const char* symbol,
                         void (*onClick)(void),
                         lv_coord_t width = 0,
                         lv_coord_t height = 0);

    lv_obj_t* root() const { return btn_; }

    void setText(const char* txt);
    void setEnabled(bool enabled);

private:
    static void ensureStyles();

    static bool stylesInited_;
    static lv_style_t styleBtnBase_;
    static lv_style_t styleBtnPressed_;
    static lv_style_t styleBtnFocused_;
    static lv_style_t styleIconLabel_;
    static lv_style_t styleTextLabel_;

    lv_obj_t* btn_;
    lv_obj_t* iconLabel_;
    lv_obj_t* textLabel_;
};
