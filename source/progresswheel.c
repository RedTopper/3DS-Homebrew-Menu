#include "progresswheel.h"
#include "MAGFX.h"
#include "gfx.h"

#include "progressWheelFrame1_bin.h"
#include "progressWheelFrame2_bin.h"
#include "progressWheelFrame3_bin.h"
#include "progressWheelFrame4_bin.h"
#include "progressWheelFrame5_bin.h"
#include "progressWheelFrame6_bin.h"

u8* wheelFrames[] = {
    (u8*)progressWheelFrame1_bin,
    (u8*)progressWheelFrame2_bin,
    (u8*)progressWheelFrame3_bin,
    (u8*)progressWheelFrame4_bin,
    (u8*)progressWheelFrame5_bin,
    (u8*)progressWheelFrame6_bin,
};

void drawProgressWheel(progresswheel * aProgressWheel, gfxScreen_t screen, gfx3dSide_t side, int x, int y, rgbColour * colour) {
    u8 dest[40*40*4];
    MAGFXImageWithRGBAndAlphaMask(colour->r, colour->g, colour->b, wheelFrames[aProgressWheel->frame], dest, 40, 40);
    gfxDrawSpriteAlphaBlend(screen, side, dest, 40, 40, x, y);
    aProgressWheel->frame = aProgressWheel->frame + 1;
    if (aProgressWheel->frame >= 6) {
        aProgressWheel->frame = 0;
    }
}