#include "ui_pin.h"
#include "lvgl.h"
#include <array>

static lv_obj_t* g_container = nullptr;
static lv_obj_t* g_mask_label = nullptr;
static std::array<int,4> g_digits = {0,0,0,0};
static int g_idx = 0;
static int g_expected = 0;
static std::function<void(bool)> g_cb;

static void redraw_mask() {
    static char buf[8] = "****";
    lv_label_set_text(g_mask_label, buf);
}

static void close_dialog() {
    if (g_container) {
        lv_obj_del(g_container);
        g_container = nullptr;
    }
}

static void inc_digit(int delta) {
    g_digits[g_idx] = (g_digits[g_idx] + delta + 10) % 10;
    redraw_mask();
}

static int current_pin() {
    return g_digits[0]*1000 + g_digits[1]*100 + g_digits[2]*10 + g_digits[3];
}

void ui_show_pin_dialog(int expected_pin, std::function<void(bool)> on_done) {
    g_expected = expected_pin;
    g_cb = std::move(on_done);
    g_digits = {0,0,0,0};
    g_idx = 0;

    // Contenedor
    g_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(g_container, 420, 260);
    lv_obj_center(g_container);

    // Título
    lv_obj_t* title = lv_label_create(g_container);
    lv_label_set_text(title, "Introducir\nclave");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // ****
    g_mask_label = lv_label_create(g_container);
    lv_label_set_text(g_mask_label, "****");
    lv_obj_align(g_mask_label, LV_ALIGN_CENTER, 0, -10);

    auto mk_btn = [&](const char* txt, lv_event_cb_t cb, lv_align_t a, int dx, int dy){
        lv_obj_t* b = lv_btn_create(g_container);
        lv_obj_set_size(b, 90, 48);
        lv_obj_align(b, a, dx, dy);
        lv_obj_t* l = lv_label_create(b);
        lv_label_set_text(l, txt);
        lv_obj_center(l);
        lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, nullptr);
        return b;
    };

    mk_btn("▲", [](lv_event_t*){ inc_digit(+1); }, LV_ALIGN_BOTTOM_LEFT,  30, -80);
    mk_btn("▼", [](lv_event_t*){ inc_digit(-1); }, LV_ALIGN_BOTTOM_LEFT,  30, -20);
    mk_btn("◄", [](lv_event_t*){ g_idx = (g_idx + 3) % 4; }, LV_ALIGN_BOTTOM_MID, -60, -50);
    mk_btn("►", [](lv_event_t*){ g_idx = (g_idx + 1) % 4; }, LV_ALIGN_BOTTOM_MID,  60, -50);
    mk_btn("SEGUIR", [](lv_event_t*){
        bool ok = (current_pin() == g_expected);
        auto cb = g_cb;
        close_dialog();
        if (cb) cb(ok);
    }, LV_ALIGN_BOTTOM_RIGHT, -20, -20);

    redraw_mask();
}
