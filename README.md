# ESP-IDF Geiger Counter Component

A interrupt-driven ESP-IDF component for reading RadiationD-v1.1 Geiger-Müller counter with J321 (M4011) tube using the ESP32.

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

3. Ensure your main/CMakeLists.txt requires the component (if not automatically detected):

idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES geiger)