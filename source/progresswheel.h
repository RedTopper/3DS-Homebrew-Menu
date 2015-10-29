#pragma once
#include <3ds.h>
#include "colours.h"

typedef struct progresswheel {
    int frame;
} progresswheel;

void drawProgressWheel(progresswheel * aProgressWheel, gfxScreen_t screen, gfx3dSide_t side, int x, int y, rgbColour * colour);