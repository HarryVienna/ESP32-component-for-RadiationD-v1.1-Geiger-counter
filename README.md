# ESP-IDF Geiger Counter Component

A interrupt-driven ESP-IDF component for reading RadiationD-v1.1 Geiger-Müller counter with J321 (M4011) tube using the ESP32.

![RadiationD-v1.1](https://github.com/user-attachments/assets/4961b8a0-ea8c-455e-8cdc-52ee8da145a0)


This component handles pulse counting via interrupts, calculates the radiation dose rate in **µSv/h**, and implements a **Rolling Average** algorithm to smooth out the stochastic nature of radioactive decay.

## Features

* **Interrupt Driven:** Uses GPIO interrupts (Falling Edge) to capture pulses efficiently without blocking the CPU.
* **Rolling Average:** Calculates CPM (Counts Per Minute) over a sliding time window (e.g., 60s or 300s) for stable readings.
* **Thread Safe:** Uses `stdatomic` and a separate FreeRTOS task to process data safely.
* **Configurable:** Easy setup for different tubes (Conversion Factor) and GPIO pins.

## Hardware Setup

Most cheap Geiger counter kits (like the "RadiationD-v1.1" or generic CA-42 kits) have a 3-pin interface: `5V`, `GND`, and `VIN` (Signal).

* **5V:** Connect to 5V (or 3.3V if your module supports it).
* **GND:** Connect to ESP32 GND.
* **OUT/VIN (Signal):** Connect to any GPIO (e.g., GPIO 4).

**Note:** The signal is usually **Active Low** (High normally, drops to Low on detection).

## Installation

1.  Create a `components` directory in your ESP-IDF project root.
2.  Clone or copy this library into `components/geiger`.

Your project structure should look like this:

```text
my_project/
├── main/
│   ├── main.c
│   └── CMakeLists.txt
├── components/
│   └── geiger/
│       ├── include/
│       │   └── geiger.h
│       ├── geiger.c
│       └── CMakeLists.txt
└── ...
```

3. Ensure your main/CMakeLists.txt requires the component (if not automatically detected):

```
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES geiger)
```

## Usage

In your main.c:                       

```
    #include <stdio.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"
    #include "driver/gpio.h" // Required for GPIO_NUM_X
    #include "esp_log.h"

    // Include the component
    #include "geiger.h"

    void app_main(void) {
        // 1. Configure the library
        geiger_config_t config = {
            .gpio_pin = GPIO_NUM_4,       // The GPIO pin connected to the Geiger tube
            .conversion_factor = 151.0f,  // Calibration: CPM required for 1 µSv/h (151 is typical for J305/SBM-20)
            .rolling_avg_seconds = 60     // Averaging window (60s for fast response, 300s for high stability)
        };

        // 2. Initialize
        geiger_init(&config);

        // 3. Main Loop
        while (1) {
            float usvh = geiger_get_usvh();
            float cpm = geiger_get_cpm();
            
            ESP_LOGI("APP", "Radiation: %.2f CPM | %.4f µSv/h", cpm, usvh);
            
            // Update every 5 seconds
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }
```

## Configuration Guide

Conversion Factor

Different tubes have different sensitivities. The factor represents how many counts per minute (CPM) equal 1 µSv/h.

| Tube Model | Typical Factor | Notes |
| -------- | ------- | ------- |
| J321 / M4011 | ~151 - 153	|Glass tubes, common in cheap kits|
| SBM-20 |  ~175| Soviet metal tube, very common|
| LND-712 | ~123| Pancake tube, high sensitivity|

## Rolling Average Period

   - 60 Seconds: Standard setting. Updates relatively quickly but values may fluctuate at low background radiation levels (Poisson noise).

   - 300 Seconds (5 mins): Recommended for measuring background radiation. Provides very stable readings but reacts slowly to sudden spikes.
