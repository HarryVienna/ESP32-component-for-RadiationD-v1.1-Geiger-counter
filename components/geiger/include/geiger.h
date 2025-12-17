#pragma once

#include <stdint.h>

// Konfigurations-Struktur
typedef struct {
    int gpio_pin;
    float conversion_factor;  // z.B. 151.0 für J305/SBM-20
    int rolling_avg_seconds;  // z.B. 60 oder 300
} geiger_config_t;

/**
 * @brief Initialisiert den Geigerzähler und startet den Hintergrund-Task
 * @param config Zeiger auf die Konfiguration
 */
void geiger_init(const geiger_config_t *config);

/**
 * @brief Gibt den aktuellen µSv/h Wert zurück
 */
float geiger_get_usvh(void);

/**
 * @brief Gibt die aktuellen CPM (Counts Per Minute) zurück
 */
float geiger_get_cpm(void);
