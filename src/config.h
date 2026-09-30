#ifndef CONFIG_H
#define CONFIG_H

//================================================
// Serial
//================================================
#define SERIAL_BAUD      115200
#define CMD_BUFFER_SIZE  32

//================================================
// Timing
//================================================
#define SERIAL_TIMEOUT   10
#define DEBOUNCE_MS      300
#define SENSOR_INTERVAL  1500
#define CHART_POINTS     28

//================================================
// Boot
//================================================
#define BOOT_DURATION_MS 3500
#define BOOT_LINES       5

//================================================
// Geiger Counter (RadiationD v1.1 / CAJOE)
//================================================
#define GEIGER_PIN       26
#define GEIGER_DEBOUNCE  2       // ms debounce
#define TUBE_FACTOR      0.00812f // J305β: CPM × factor = µSv/h
#define TUBE_NOISE_CPM   12      // background noise CPM
#define CPM_ROLL_SECS    60      // rolling average window (seconds)

//================================================
// Navigation Buttons (INPUT_PULLUP, active LOW)
//================================================
#define BTN_UP_PIN       32
#define BTN_DOWN_PIN     33
#define BTN_SELECT_PIN   27
#define LONG_PRESS_MS    2000

//================================================
// Battery & GPS (still simulated)
//================================================
#define DEFAULT_BATTERY      87

#endif
