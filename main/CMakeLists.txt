idf_component_register(
    SRCS
        "main.cpp"
        "ui/ui_screens.cpp"
        "ui/widgets_reusable/widget_slider.cpp"
        "lvgl_port.c"
        "waveshare_rgb_lcd_port.c"
        "lvgl_lock_shim.c"                 # <— AÑADIDO
    INCLUDE_DIRS
        "."
        "ui"
        "ui/widgets_reusable"
    REQUIRES
        lvgl
        esp_lcd
        driver
        esp_timer
        esp_lcd_touch
)
target_compile_features(${COMPONENT_LIB} PUBLIC cxx_std_17)
