#ifndef THEME_H
#define THEME_H

#include <TFT_eSPI.h>

//================================================
// Display Layout
//================================================
#define SCREEN_W        320
#define SCREEN_H        480
#define STATUS_BAR_H    20
#define BOTTOM_NAV_H    50
#define CONTENT_Y       STATUS_BAR_H
#define CONTENT_H       (SCREEN_H - STATUS_BAR_H - BOTTOM_NAV_H)

//================================================
// Page IDs
//================================================
#define PAGE_BOOT       0
#define PAGE_HOME       1
#define PAGE_NAV        2
#define PAGE_COUNT      3
#define PAGE_MENU       4
#define PAGE_SETTINGS   5
#define PAGE_ABOUT      6
#define PAGE_SENSORS    7
#define PAGE_LOGS       8
#define NUM_PAGES       9

//================================================
// Colors — Dark Theme (RGB565)
//================================================
#define C_BG            0x0862  // #0A0C10
#define C_CARD          0x1947  // #1E293B
#define C_CARD_BORDER   0x320A  // #334155
#define C_TEXT          0xFFFF  // white
#define C_SUBTEXT       0x63B1  // #64748B

#define C_PRIMARY       0x15D0  // #10B981 emerald
#define C_NAV           0x3C1E  // #3B82F6 blue
#define C_COUNT         0x8AFE  // #8B5CF6 purple
#define C_MENU          0xC618  // #AAAAAA light gray

#define C_WARNING       0xF4E1  // #F59E0B amber
#define C_DANGER        0xEA28  // #EF4444 red
#define C_OK            0x15D0  // emerald

#define C_CHART_BAR     0x15D0  // emerald
#define C_CHART_LINE    0x3C1E  // blue
#define C_CHART_BG      0x10A2  // darker card
#define C_CHART_GRID    0x294A  // subtle grid

#define C_RADAR_BG      0x10A2
#define C_RADAR_RING    0x320A
#define C_RADAR_SWEEP   0x15D0

//================================================
// Sizes
//================================================
#define CARD_R          6
#define NAV_ICON_SIZE   16

//================================================
// Bottom Nav Bar Zones (4 columns)
//================================================
#define NAV_TABS        4
#define NAV_TAB_W       (SCREEN_W / NAV_TABS)
#define NAV_ZONE_0      0
#define NAV_ZONE_1      NAV_TAB_W
#define NAV_ZONE_2      (NAV_TAB_W * 2)
#define NAV_ZONE_3      (NAV_TAB_W * 3)

//================================================
// Status level IDs
//================================================
#define STATUS_NORMAL   0
#define STATUS_ELEVATED 1
#define STATUS_HIGH     2
#define STATUS_CRIT     3

#endif
