#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "geiger.h" // Unsere neue Komponente einbinden

static const char *TAG = "APP";

void app_main(void) {
    // 1. Konfiguration erstellen
    geiger_config_t config = {
        .gpio_pin = GPIO_NUM_4,
        .conversion_factor = 151.0f,
        .rolling_avg_seconds = 600 
    };

    // 2. Komponente initialisieren
    geiger_init(&config);

    // 3. Hauptschleife: Hier kannst du machen, was du willst
    // (Daten anzeigen, MQTT senden, auf Display schreiben)
    while (1) {
        // Wir lesen nur noch die fertigen Werte ab
        float usvh = geiger_get_usvh();
        float cpm = geiger_get_cpm();

        ESP_LOGI(TAG, "Messung: %.2f CPM | %.4f µSv/h", cpm, usvh);

        // Hier z.B. Code einfügen: send_to_mqtt(usvh);
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Nur alle 5 Sekunden loggen reicht völlig
    }
}