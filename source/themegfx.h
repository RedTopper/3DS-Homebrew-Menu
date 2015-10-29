#pragma once
#include <3ds.h>
#include "gfx.h"

extern int themeImageAppBackground;
extern int themeImageAppBackgroundSelected;
    
extern int themeImageCartBackground;
extern int themeImageCartBackgroundSelected;
    
extern int themeImageTopWallpaper;
extern int themeImageTopWallpaperInfo;
extern int themeImageBottomWallpaper;
    
extern int themeImageTopLeftButton;
extern int themeImageTopRightButton;
extern int themeImageBottomLeftButton;
extern int themeImageBottomRightButton;
    
extern int themeImageTopLeftButtonSelected;
extern int themeImageTopRightButtonSelected;
extern int themeImageBottomLeftButtonSelected;
extern int themeImageBottomRightButtonSelected;
    
extern int themeImageHelpSymbol;
extern int themeImageBackSymbol;
extern int themeImageHomeSymbol;
extern int themeImageSettingsSymbol;
extern int themeImageFoldersSymbol;

void initThemeImages();

void drawThemeImage(int imageID, gfxScreen_t screen, int x, int y);

bool themeImageExists(int imageID);