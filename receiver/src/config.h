#ifndef CONFIG_H
#define CONFIG_H

//================================================
// Serial
//================================================
#define SERIAL_BAUD      115200

//================================================
// Waveshare 1.47" LCD (ST7789V3, 172x320)
//================================================
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
#define TFT_BL   15
#define TFT_SCK  18
#define TFT_MOSI 23

#define SCREEN_W  320
#define SCREEN_H  172

//================================================
// Display Layout
//================================================
#define STATUS_BAR_H    18
#define BOTTOM_BAR_H    16
#define CONTENT_Y       STATUS_BAR_H
#define CONTENT_H       (SCREEN_H - STATUS_BAR_H - BOTTOM_BAR_H)

//================================================
// Colors — Dark Theme (RGB565)
//================================================
#define C_BG            0x0862
#define C_CARD          0x1947
#define C_CARD_BORDER   0x320A
#define C_TEXT          0xFFFF
#define C_SUBTEXT       0x63B1

#define C_PRIMARY       0x15D0  // emerald
#define C_NAV           0x3C1E  // blue
#define C_COUNT         0x8AFE  // purple
#define C_MENU          0xC618

#define C_WARNING       0xF4E1  // amber
#define C_DANGER        0xEA28  // red
#define C_OK            0x15D0

#define C_RADAR_BG      0x10A2
#define C_RADAR_RING    0x320A
#define C_RADAR_SWEEP   0x15D0

//================================================
// Card Layout
//================================================
#define CARD_PAD        4
#define CARD_R          4
#define CARD_WSmall     80
#define CARD_WBig       164

//================================================
// Status Level IDs
//================================================
#define STATUS_NORMAL   0
#define STATUS_ELEVATED 1
#define STATUS_HIGH     2
#define STATUS_CRIT     3

//================================================
// ESP-NOW
//================================================
#define DATA_TIMEOUT_MS 3000

#endif
