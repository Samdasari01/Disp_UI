#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

void buttonsInit();
void buttonsUpdate();
void processCommand(String cmd);
void enterDeepSleep();

extern int  currentPage;
extern int  currentTab;
extern bool pageChanged;

#endif
