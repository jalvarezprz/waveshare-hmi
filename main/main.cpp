#include "lvgl_port.h"
#include "waveshare_rgb_lcd_port.h"
#include "lvgl_lock_shim.h"

#include "ui/theme/ui_theme_c_api.h"
#include "ui/layout/ui_layout_scaffold.h"
#include "ui/ui_router.h"
#include "ui/view/ui_view_welcome.h"

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(waveshare_esp32_s3_rgb_lcd_init());

    lvgl_port_lock(UINT32_MAX);

    ui_theme_init_once_c();

    Ui::UiLayoutScaffold scaffold;
    scaffold.setBarsHeight(56, 48);
    scaffold.build(lv_scr_act());

    // Inyecta contenido real en el Content del scaffold
    Ui::ui_view_welcome_mount(scaffold);

    lvgl_port_unlock();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
