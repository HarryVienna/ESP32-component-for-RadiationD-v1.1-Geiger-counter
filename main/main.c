#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#include "geiger.h"

static const char *TAG = "APP";

void app_main(void) {
    // 1. Create configuration
    geiger_config_t config = {
        .gpio_pin = GPIO_NUM_4,
        .conversion_factor = 153.8f,  // https://muman.ch/muman/muman-geiger-counter.htm       or 151.0 ?
        .rolling_avg_seconds = 600 
    };

    // 2. Initialize component
    geiger_init(&config);

    // 3. Main loop: Do whatever you want here
    // (Display data, send via MQTT, write to screen)
    while (1) {
        // We just read the calculated values
        float usvh = geiger_get_usvh();
        float cpm = geiger_get_cpm();

        ESP_LOGI(TAG, "Measurement: %.2f CPM | %.4f µSv/h", cpm, usvh);

        // Insert code here, e.g.: send_to_mqtt(usvh);
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Logging every 5 seconds is sufficient
    }
}