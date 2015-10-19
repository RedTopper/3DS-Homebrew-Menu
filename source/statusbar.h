#pragma once
#include <3ds.h>

bool clock24;
extern bool statusbarNeedsUpdate;

void drawStatusBar(bool wifiStatus, bool charging, int batteryLevel);