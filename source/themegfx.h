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
    
    themeImageTopLeftButton,
    themeImageTopRightButton,
    themeImageBottomLeftButton,
    themeImageBottomRightButton,
    
    themeImageTopLeftButtonSelected,
    themeImageTopRightButtonSelected,
    themeImageBottomLeftButtonSelected,
    themeImageBottomRightButtonSelected,
    
    themeImageHelpSymbol,
    themeImageBackSymbol,
    themeImageHomeSymbol,
    themeImageSettingsSymbol,
    themeImageFoldersSymbol,
    
    themeImageProgressWheel1,
    themeImageProgressWheel2,
    themeImageProgressWheel3,
    themeImageProgressWheel4,
    themeImageProgressWheel5,
    themeImageProgressWheel6,
} themeImageID;

extern bool     themeHasAppBackgroundImage,
                themeHasAppBackgroundImageSelected,

                themeHasCartBackgroundImage,
                themeHasCartBackgroundImageSelected,

                themeHasTopWallpaper,
                themeHasBottomWallpaper,

                themeHasTopLeftButton,
                themeHasTopRightButton,
                themeHasBottomLeftButton,
                themeHasBottomRightButton,

                themeHasTopLeftButtonSelected,
                themeHasTopRightButtonSelected,
                themeHasBottomLeftButtonSelected,
                themeHasBottomRightButtonSelected,

                themeHasHelpSymbol,
                themeHasBackSymbol,
                themeHasHomeSymbol,
                themeHasSettingsSymbol,
                themeHasFoldersSymbol,
                themeHasProgressWheel;

void initThemeImages();

void drawThemeImage(themeImageID imageID, gfxScreen_t screen, int x, int y);