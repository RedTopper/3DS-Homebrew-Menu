#include "themegfx.h"
#include "pngloader.h"
#include "filesystem.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logText.h"
#include "MAGFX.h"

#include "appbackgroundalphamask_bin.h"
#include "cartbackgroundalphamask_bin.h"

int themeImageAppBackground = 0;
int themeImageAppBackgroundSelected = 1;

int themeImageCartBackground = 2;
int themeImageCartBackgroundSelected = 3;

int themeImageTopWallpaper = 4;
int themeImageBottomWallpaper = 5;

int themeImageTopLeftButton = 6;
int themeImageTopRightButton = 7;
int themeImageBottomLeftButton = 8;
int themeImageBottomRightButton = 9;

int themeImageTopLeftButtonSelected = 10;
int themeImageTopRightButtonSelected = 11;
int themeImageBottomLeftButtonSelected = 12;
int themeImageBottomRightButtonSelected = 13;

int themeImageHelpSymbol = 14;
int themeImageBackSymbol = 15;
int themeImageHomeSymbol = 16;
int themeImageSettingsSymbol = 17;
int themeImageFoldersSymbol = 18;

int themeImageTopWallpaperInfo = 19;

int themeImageProgressWheelFrame1 = 20;
int themeImageProgressWheelFrame2 = 21;
int themeImageProgressWheelFrame3 = 22;
int themeImageProgressWheelFrame4 = 23;
int themeImageProgressWheelFrame5 = 24;
int themeImageProgressWheelFrame6 = 25;

bool themeHasProgressWheel = false;

typedef struct themeImage {
    u8 * spriteData;
    bool exists;
    bool hasAlpha;
    int w, h;
} themeImage;

themeImage themeImages[26];

bool loadThemeImage(char * path, char * description, int expectedWidth, int expectedHeight, u8 * alphaMask, int imageID) {
    themeImage * aThemeImage = &(themeImages[imageID]);
    aThemeImage->exists = false;
    
    bool success = read_png_file(path);
    
    if (success) {
        if (pngWidth != expectedWidth || pngHeight != expectedHeight) {
            char error[256];
            sprintf(error, "%s must be %dx%d pixels", description, expectedWidth, expectedHeight);
            logText(error);
        }
        else {
            u8 * out = process_png_file();
            
            if (out) {
                if (alphaMask) {
                    u8 * masked = malloc(expectedWidth*expectedHeight*4);
                    MAGFXApplyAlphaMask(out, alphaMask, masked, expectedWidth, expectedHeight, (bytesPerPixel==4));
                    aThemeImage->spriteData = masked;
                    free(out);
                    aThemeImage->hasAlpha = true;
                }
                else {
                    aThemeImage->spriteData = out;
                    aThemeImage->hasAlpha = (bytesPerPixel==4);
                }
                
                
                aThemeImage->exists = true;
                aThemeImage->w = expectedWidth;
                aThemeImage->h = expectedHeight;
            }
        }
    }
    
    return aThemeImage->exists;
}

void initThemeImages() {
    char * themePath = currentThemePath();
    char path[128];
    
    
    sprintf(path, "%sappbackground.png", themePath);
    loadThemeImage(path, "App background", 56, 56, (u8*)appbackgroundalphamask_bin, themeImageAppBackground);

    sprintf(path, "%sappbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected app background", 56, 56, (u8*)appbackgroundalphamask_bin, themeImageAppBackgroundSelected);


    
    
    sprintf(path, "%scartbackground.png", themePath);
    loadThemeImage(path, "Cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, themeImageCartBackground);

    sprintf(path, "%scartbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, themeImageCartBackgroundSelected);

    
    
    sprintf(path, "%swallpapertop.png", themePath);
    loadThemeImage(path, "Top wallpaper", 400, 240, NULL, themeImageTopWallpaper);
    
    sprintf(path, "%swallpapertopinfo.png", themePath);
    loadThemeImage(path, "Top wallpaper", 400, 240, NULL, themeImageTopWallpaperInfo);

    sprintf(path, "%swallpaperbottom.png", themePath);
    loadThemeImage(path, "Bottom wallpaper", 320, 240, NULL, themeImageBottomWallpaper);

    
    


    sprintf(path, "%sbuttontopleft.png", themePath);
    loadThemeImage(path, "Top left button", 36, 36, NULL, themeImageTopLeftButton);

    sprintf(path, "%sbuttontopright.png", themePath);
    loadThemeImage(path, "Top right button", 36, 36, NULL, themeImageTopRightButton);

    sprintf(path, "%sbuttonbottomleft.png", themePath);
    loadThemeImage(path, "Bottom left button", 36, 36, NULL, themeImageBottomLeftButton);

    sprintf(path, "%sbuttonbottomright.png", themePath);
    loadThemeImage(path, "Bottom right button", 36, 36, NULL, themeImageBottomRightButton);
    

    
    
    
    sprintf(path, "%sbuttontopleftselected.png", themePath);
    loadThemeImage(path, "Top left selected button", 36, 36, NULL, themeImageTopLeftButtonSelected);
    
    sprintf(path, "%sbuttontoprightselected.png", themePath);
    loadThemeImage(path, "Top right selected button", 36, 36, NULL, themeImageTopRightButtonSelected);
    
    sprintf(path, "%sbuttonbottomleftselected.png", themePath);
    loadThemeImage(path, "Bottom left selected button", 36, 36, NULL, themeImageBottomLeftButtonSelected);
    
    sprintf(path, "%sbuttonbottomrightselected.png", themePath);
    loadThemeImage(path, "Bottom right selected button", 36, 36, NULL, themeImageBottomRightButtonSelected);
    
    
    
    
    
    
    sprintf(path, "%shelpicon.png", themePath);
    loadThemeImage(path, "Help icon", 36, 36, NULL, themeImageHelpSymbol);
    
    sprintf(path, "%sbackicon.png", themePath);
    loadThemeImage(path, "Back icon", 36, 36, NULL, themeImageBackSymbol);

    sprintf(path, "%shomeicon.png", themePath);
    loadThemeImage(path, "Home icon", 36, 36, NULL, themeImageHomeSymbol);
    
    sprintf(path, "%ssettingsicon.png", themePath);
    loadThemeImage(path, "Settings icon", 36, 36, NULL, themeImageSettingsSymbol);
    
    sprintf(path, "%sfoldersicon.png", themePath);
    loadThemeImage(path, "Folders icon", 36, 36, NULL, themeImageFoldersSymbol);
    
    
    
    sprintf(path, "%sprogressWheelFrame1.png", themePath);
    themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 1", 36, 36, NULL, themeImageProgressWheelFrame1);
    if (themeHasProgressWheel) {
        
        sprintf(path, "%sprogressWheelFrame2.png", themePath);
        themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 2", 36, 36, NULL, themeImageProgressWheelFrame2);
        if (themeHasProgressWheel) {
            
            sprintf(path, "%sprogressWheelFrame3.png", themePath);
            themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 3", 36, 36, NULL, themeImageProgressWheelFrame3);
            if (themeHasProgressWheel) {
                
                sprintf(path, "%sprogressWheelFrame4.png", themePath);
                themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 4", 36, 36, NULL, themeImageProgressWheelFrame4);
                if (themeHasProgressWheel) {
                    
                    sprintf(path, "%sprogressWheelFrame5.png", themePath);
                    themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 5", 36, 36, NULL, themeImageProgressWheelFrame5);
                    if (themeHasProgressWheel) {
                        
                        sprintf(path, "%sprogressWheelFrame6.png", themePath);
                        themeHasProgressWheel = loadThemeImage(path, "Progress wheel frame 6", 36, 36, NULL, themeImageProgressWheelFrame6);
                        
                    }
                    
                }
                
            }
            
        }
        
    }
    
    
    
    
    
    

//    sprintf(path, "%sprogressWheelFrame1.png", themePath);
//    loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel1, &themeHasProgressWheel);
//    if (themeHasProgressWheel) {
//        sprintf(path, "%sprogressWheelFrame2.png", themePath);
//        loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel2, &themeHasProgressWheel);
//        if (themeHasProgressWheel) {
//            sprintf(path, "%sprogressWheelFrame3.png", themePath);
//            loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel3, &themeHasProgressWheel);
//            if (themeHasProgressWheel) {
//                sprintf(path, "%sprogressWheelFrame4.png", themePath);
//                loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel4, &themeHasProgressWheel);
//                if (themeHasProgressWheel) {
//                    sprintf(path, "%sprogressWheelFrame5.png", themePath);
//                    loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel5, &themeHasProgressWheel);
//                    if (themeHasProgressWheel) {
//                        sprintf(path, "%sprogressWheelFrame6.png", themePath);
//                        loadThemeImage(path, "Progress wheel", 36, 36, NULL, (u8*)progressWheel6, &themeHasProgressWheel);
//                        if (themeHasProgressWheel) {
//                            progressWheelHasAlpha = true;
//                        }
//                    }
//                }
//            }
//        }
//    }
    
    
    
    free(themePath);
}

void drawThemeImageCheckAlpha(u8 * image, gfxScreen_t screen, int x, int y, int w, int h, bool hasAlpha) {
    if (hasAlpha) {
        gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, image, w, h, x, y);
    }
    else {
        gfxDrawSprite(screen, GFX_LEFT, image, w, h, x, y);
    }
}

void drawThemeImage(int imageID, gfxScreen_t screen, int x, int y) {
    themeImage aThemeImage = themeImages[imageID];
    if (aThemeImage.exists) {
        drawThemeImageCheckAlpha(aThemeImage.spriteData, screen, x, y, aThemeImage.h, aThemeImage.w, aThemeImage.hasAlpha);
    }
}

bool themeImageExists(int imageID) {    
    themeImage aThemeImage = themeImages[imageID];
    return aThemeImage.exists;
}