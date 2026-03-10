#include <string.h>
#include <assert.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/uart.h"
#include "driver/gpio.h"

#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_cdc_acm.h"

static const char *TAG = "usb_uart_bridge";

// ----------------- UART CONFIG -----------------
#define BRIDGE_UART         UART_NUM_1
#define BRIDGE_BAUD         115200
#define BRIDGE_UART_TX_GPIO GPIO_NUM_17
#define BRIDGE_UART_RX_GPIO GPIO_NUM_18

#define UART_READ_CHUNK     32

#ifndef CFG_TUD_CDC_RX_BUFSIZE
#define CFG_TUD_CDC_RX_BUFSIZE 64
#endif

typedef struct {
    uint8_t data[CFG_TUD_CDC_RX_BUFSIZE];
    size_t  len;
    int     itf;
} usb_rx_msg_t;

static QueueHandle_t usb_rx_queue;

// pull bytes from USB CDC and queue for UART
static void usb_cdc_rx_callback(int itf, cdcacm_event_t *event)
{
    (void)event;

    usb_rx_msg_t msg = { .len = 0, .itf = itf };
    size_t rx_size = 0;

    esp_err_t ret = tinyusb_cdcacm_read(itf, msg.data, sizeof(msg.data), &rx_size);
    if (ret != ESP_OK || rx_size == 0) {
        return;
    }

    msg.len = rx_size;

    (void)xQueueSend(usb_rx_queue, &msg, 0);
}

static void usb_to_uart_task(void *arg)
{
    (void)arg;
    usb_rx_msg_t msg;

    while (1) {
        if (xQueueReceive(usb_rx_queue, &msg, portMAX_DELAY)) {
            if (msg.len) {
                int written = uart_write_bytes(BRIDGE_UART, msg.data, msg.len);
                if (written < 0) {
                    ESP_LOGW(TAG, "uart_write_bytes failed");
                }
            }
        }
    }
}

static void uart_to_usb_task(void *arg)
{
    uint8_t buf[UART_READ_CHUNK];

    while (1) {
        int n = uart_read_bytes(BRIDGE_UART, buf, sizeof(buf), pdMS_TO_TICKS(1));

        if (n > 0) {
            tinyusb_cdcacm_write_queue(TINYUSB_CDC_ACM_0, buf, n);
            tinyusb_cdcacm_write_flush(TINYUSB_CDC_ACM_0, 0);
        }
    }
}

static void uart_init_bridge(void)
{
    uart_config_t cfg = {
        .baud_rate = BRIDGE_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(BRIDGE_UART, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(BRIDGE_UART,
                                 BRIDGE_UART_TX_GPIO,
                                 BRIDGE_UART_RX_GPIO,
                                 UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE));

    ESP_ERROR_CHECK(uart_driver_install(BRIDGE_UART, 2048, 2048, 0, NULL, 0));
}

static void usb_init_cdc(void)
{
    const tinyusb_config_t tusb_cfg = TINYUSB_DEFAULT_CONFIG();
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));

    tinyusb_config_cdcacm_t acm_cfg = {
        .cdc_port = TINYUSB_CDC_ACM_0,
        .callback_rx = &usb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
        .callback_line_state_changed = NULL,
        .callback_line_coding_changed = NULL,
    };
    ESP_ERROR_CHECK(tinyusb_cdcacm_init(&acm_cfg));
}

void app_main(void)
{
    ESP_LOGI(TAG, "Init UART");
    uart_init_bridge();

    ESP_LOGI(TAG, "Init USB CDC");
    usb_rx_queue = xQueueCreate(8, sizeof(usb_rx_msg_t));
    assert(usb_rx_queue);

    usb_init_cdc();

    ESP_LOGI(TAG, "Starting bridge tasks");
    xTaskCreate(usb_to_uart_task, "usb_to_uart", 4096, NULL, 10, NULL);
    xTaskCreate(uart_to_usb_task, "uart_to_usb", 4096, NULL, 10, NULL);

    ESP_LOGI(TAG, "Bridge running: USB CDC <-> UART%d (%d baud)", BRIDGE_UART, BRIDGE_BAUD);
}