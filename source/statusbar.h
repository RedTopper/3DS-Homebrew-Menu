#pragma once
#include <3ds.h>

extern bool clock24;
extern bool showDate;
extern bool statusbarNeedsUpdate;

void drawStatusBar(bool wifiStatus, bool charging, int batteryLevel);
