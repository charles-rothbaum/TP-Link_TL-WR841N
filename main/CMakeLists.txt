idf_component_register(
    SRCS "usb_uart_bridge.c"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_uart esp_driver_gpio espressif__esp_tinyusb
)