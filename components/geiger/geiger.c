#include "geiger.h"
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "stdatomic.h"
#include "esp_timer.h"

static const char *TAG = "geiger_lib";

#define DEBOUNCE_TIME_US 200

// Interne Variablen (statisch, damit sie privat bleiben)
static atomic_uint pulse_count = ATOMIC_VAR_INIT(0);
static float current_usvh = 0.0f;
static float current_cpm = 0.0f;

// Konfiguration speichern
static geiger_config_t active_config;
static uint32_t *rolling_buffer = NULL;

// ISR Handler
static void IRAM_ATTR geiger_isr_handler(void* arg) {
    // static int64_t last_pulse_time = 0;
    // int64_t current_time = esp_timer_get_time();

    // if (current_time - last_pulse_time > DEBOUNCE_TIME_US) {
    //     atomic_fetch_add(&pulse_count, 1);
    //     last_pulse_time = current_time;
    // }

    atomic_fetch_add(&pulse_count, 1);
}

// Der Hintergrund-Task (ersetzt deine while-Schleife aus main)
static void geiger_task(void *pvParameters) {
    int buffer_index = 0;
    uint32_t total_counts_in_period = 0;

    // Puffer initialisieren (Speicher reservieren)
    rolling_buffer = calloc(active_config.rolling_avg_seconds, sizeof(uint32_t));
    if (rolling_buffer == NULL) {
        ESP_LOGE(TAG, "Kein Speicher für Rolling Buffer!");
        vTaskDelete(NULL);
    }

    while (1) {
        // Exakt 1 Sekunde warten
        vTaskDelay(pdMS_TO_TICKS(1000));

        // 1. Zähler atomar holen und resetten
        uint32_t counts_this_second = atomic_exchange(&pulse_count, 0);

        // 2. Rolling Average Logik
        total_counts_in_period -= rolling_buffer[buffer_index];
        total_counts_in_period += counts_this_second;
        rolling_buffer[buffer_index] = counts_this_second;

        buffer_index = (buffer_index + 1) % active_config.rolling_avg_seconds;

        // 3. Berechnung
        float cpm = (float)total_counts_in_period * (60.0f / (float)active_config.rolling_avg_seconds);
        
        // Werte in die globalen (statischen) Variablen schreiben
        current_cpm = cpm;
        current_usvh = cpm / active_config.conversion_factor;
        
        // Optional: Debug-Log direkt aus der Lib (kann man auch auskommentieren)
        // ESP_LOGD(TAG, "CPM: %.1f, uSv/h: %.4f", current_cpm, current_usvh);
    }
}

void geiger_init(const geiger_config_t *config) {
    // Konfiguration kopieren
    active_config = *config;

    // GPIO Setup
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = (1ULL << config->gpio_pin);
    io_conf.mode = GPIO_MODE_INPUT;
    // No pullup here!
    //io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // ISR installieren
    // Checken ob Service schon läuft, um Abstürze zu vermeiden
    gpio_install_isr_service(0); 
    ESP_ERROR_CHECK(gpio_isr_handler_add(config->gpio_pin, geiger_isr_handler, NULL));

    ESP_LOGI(TAG, "Geiger initialisiert an Pin %d. Starte Task...", config->gpio_pin);

    // Task starten (Stackgröße 4096 ist sicher, Prio 5 ist mittel)
    xTaskCreate(geiger_task, "geiger_task", 4096, NULL, 5, NULL);
}

// Getter-Funktionen (Thread-Safe genug für Lesezugriff)
float geiger_get_usvh(void) { return current_usvh; }
float geiger_get_cpm(void) { return current_cpm; }