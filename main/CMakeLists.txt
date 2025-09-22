# =========================
# Fuentes base / sistema
# =========================
set(SRCS_BASE
    main.cpp
    lvgl_port.c
    waveshare_rgb_lcd_port.c
    lvgl_lock_shim.c

    hmi_command_sender.cpp
)

# =========================
# UI — utilidades y router
# =========================
set(SRCS_UI_CORE
    ui/view/ui_view_factory.cpp
    ui/router/ui_router.cpp
)

# =========================
# UI — acciones y presets
# =========================
set(SRCS_UI_WIDGETS
    ui/preset/ui_preset_button.cpp
    ui/preset/ui_preset_switch.cpp
    ui/preset/ui_preset_label.cpp
    ui/component/ui_component_button.cpp
    ui/component/ui_component_label.cpp
    ui/layout/ui_layout_scaffold.cpp
    ui/menu/ui_menu_tree.cpp
    ui/menu/ui_menu_loader.cpp
)

# =========================
# UI — tema y fuentes
# =========================
set(SRCS_UI_THEME
    ui/theme/ui_theme_styles.cpp
    ui/theme/ui_theme_tokens.cpp
)

set(SRCS_UI_FONTS
    ui/theme/fonts/montserrat_8_lat1.c
    ui/theme/fonts/montserrat_10_lat1.c
    ui/theme/fonts/montserrat_12_lat1.c
    ui/theme/fonts/montserrat_14_lat1.c
    ui/theme/fonts/montserrat_16_lat1.c
    ui/theme/fonts/montserrat_18_lat1.c
    ui/theme/fonts/montserrat_20_lat1.c
)

# =========================
# COMM — TX y RX
# =========================
set(SRCS_COMM
    comm/comm_demo_loopback.cpp
    comm/comm_diag.cpp
    comm/command_contract.cpp
)

set(SRCS_COMM_TRANSPORT
    comm/transport/comm_transport_espnow.cpp
)

set(SRCS_COMM_TX
    comm/tx/comm_tx_gateway.cpp
    comm/tx/comm_tx_handler.cpp
    comm/tx/comm_tx_queue.cpp
    comm/tx/comm_tx_api.cpp
    comm/tx/comm_tx_json_bridge.cpp
)

set(SRCS_COMM_RX
    comm/rx/comm_rx_gateway.cpp
    comm/rx/comm_rx_handler.cpp
    comm/rx/comm_rx_queue.cpp
    comm/rx/comm_rx_state.cpp
)

# =========================
# Unión de todas las fuentes
# =========================
set(SRCS_ALL
    ${SRCS_BASE}
    ${SRCS_UI_CORE}
    ${SRCS_UI_WIDGETS}
    ${SRCS_UI_THEME}
    ${SRCS_UI_FONTS}
    ${SRCS_COMM}
    ${SRCS_COMM_TRANSPORT}
    ${SRCS_COMM_TX}
    ${SRCS_COMM_RX}
)

# ===============================================
# Candidatos obsoletos
# ===============================================
set(OBSOLETE_CANDIDATES
    # ui/ui_router.cpp                          *
    # ui/ui_menu_json_utilities.cpp             *
    # ui/ui_menu_json_tree.cpp                  *
    # ui/ui_menu_nav.cpp                        *
    # ui/ui_menu_render_detail.cpp              *
    # ui/ui_menu_field_factory.cpp              *
    # ui/ui_view_menu_list.cpp                  *
    # ui/view/ui_view_menu_grid.cpp             *
    # ui/view/ui_menu_render_grid.cpp           *
    # ui/ui_router.cpp                          *
    # ui/ui_router_adapter.cpp                  *
    # ui/ui_router_mount.cpp                    *
    # ui/ui_pin.cpp                             *
    # ui/actions/ui_actions.cpp                 *
    # ui/mockup/ui_mockup_menu.cpp              *
    # ui/menu/ui_menu_loader_smoke_test.cpp     *
    # ui/theme/ui_theme_c_api.cpp               *
    # ui/component/ui_component_slider.cpp      *
    # ui/preset/ui_preset_slider.cpp            *
)

if(OBSOLETE_CANDIDATES)
    list(REMOVE_ITEM SRCS_ALL ${OBSOLETE_CANDIDATES})
endif()

# =========================
# Registro del componente
# =========================
idf_component_register(
    SRCS        ${SRCS_ALL}
    INCLUDE_DIRS
        .
        ui
        comm
    REQUIRES
        esp_lcd
        lvgl
        json
        esp_wifi
        esp_netif
        nvs_flash
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE
    LV_LVGL_H_INCLUDE_SIMPLE
)
