#pragma once
#include <3ds.h>
#include "gfx.h"

typedef enum
{
    themeImageAppBackground,
    themeImageAppBackgroundSelected,
    themeImageCartBackground,
    themeImageCartBackgroundSelected,
    themeImageTopWallpaper,
    themeImageBottomWallpaper,
} themeImageID;

extern bool     themeHasAppBackgroundImage,
                themeHasAppBackgroundImageSelected,
                themeHasCartBackgroundImage,
                themeHasCartBackgroundImageSelected,
                themeHasTopWallpaper,
                themeHasBottomWallpaper;

void initThemeImages();

void drawThemeImage(themeImageID imageID, gfxScreen_t screen, int x, int y);