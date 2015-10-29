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

int themeImageBottomWallpaperNonGrid = 26;

bool themeHasProgressWheel = false;

typedef struct themeImage {
    u8 * spriteData;
    bool exists;
    bool hasAlpha;
    int w, h;
} themeImage;

themeImage themeImages[27];
#define maxProgressWheelImages 16
themeImage progressWheelImages[maxProgressWheelImages];
int numProgressWheelImages = 0;

bool loadThemeImage(char * path, char * description, int expectedWidth, int expectedHeight, u8 * alphaMask, int imageID, themeImage images[]) {
    themeImage * aThemeImage = &(images[imageID]);
    aThemeImage->exists = false;
    
//    free(aThemeImage->spriteData);
    
    if (!fileExists(path, &sdmcArchive)) {
        return false;
    }
    
    bool success = read_png_file(path);
    
    if (success) {
        if (pngWidth != expectedWidth || pngHeight != expectedHeight) {
            char error[256];
            sprintf(error, "%s must be %dx%d pixels", description, expectedWidth, expectedHeight);
            logText(error);
            return false;
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
                
                return true;
            }
            else {
                return false;
            }
        }
    }
    else {
        return false;
    }
}

#include "logText.h"

void initThemeImages() {
    char * themePath = currentThemePath();
    char path[128];
    
    
    sprintf(path, "%sappbackground.png", themePath);
    loadThemeImage(path, "App background", 56, 56, (u8*)appbackgroundalphamask_bin, themeImageAppBackground, themeImages);

    sprintf(path, "%sappbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected app background", 56, 56, (u8*)appbackgroundalphamask_bin, themeImageAppBackgroundSelected, themeImages);


    
    
    sprintf(path, "%scartbackground.png", themePath);
    loadThemeImage(path, "Cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, themeImageCartBackground, themeImages);

    sprintf(path, "%scartbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, themeImageCartBackgroundSelected, themeImages);

    
    
    sprintf(path, "%swallpapertop.png", themePath);
    loadThemeImage(path, "Top wallpaper", 400, 240, NULL, themeImageTopWallpaper, themeImages);
    
    sprintf(path, "%swallpapertopinfo.png", themePath);
    loadThemeImage(path, "Top info wallpaper", 400, 240, NULL, themeImageTopWallpaperInfo, themeImages);

    sprintf(path, "%swallpaperbottom.png", themePath);
    loadThemeImage(path, "Bottom wallpaper", 320, 240, NULL, themeImageBottomWallpaper, themeImages);
    
    sprintf(path, "%swallpaperbottomnongrid.png", themePath);
    loadThemeImage(path, "Bottom wallpaper (non grid)", 320, 240, NULL, themeImageBottomWallpaperNonGrid, themeImages);

    
    


    sprintf(path, "%sbuttontopleft.png", themePath);
    loadThemeImage(path, "Top left button", 36, 36, NULL, themeImageTopLeftButton, themeImages);

    sprintf(path, "%sbuttontopright.png", themePath);
    loadThemeImage(path, "Top right button", 36, 36, NULL, themeImageTopRightButton, themeImages);

    sprintf(path, "%sbuttonbottomleft.png", themePath);
    loadThemeImage(path, "Bottom left button", 36, 36, NULL, themeImageBottomLeftButton, themeImages);

    sprintf(path, "%sbuttonbottomright.png", themePath);
    loadThemeImage(path, "Bottom right button", 36, 36, NULL, themeImageBottomRightButton, themeImages);
    

    
    
    
    sprintf(path, "%sbuttontopleftselected.png", themePath);
    loadThemeImage(path, "Top left selected button", 36, 36, NULL, themeImageTopLeftButtonSelected, themeImages);
    
    sprintf(path, "%sbuttontoprightselected.png", themePath);
    loadThemeImage(path, "Top right selected button", 36, 36, NULL, themeImageTopRightButtonSelected, themeImages);
    
    sprintf(path, "%sbuttonbottomleftselected.png", themePath);
    loadThemeImage(path, "Bottom left selected button", 36, 36, NULL, themeImageBottomLeftButtonSelected, themeImages);
    
    sprintf(path, "%sbuttonbottomrightselected.png", themePath);
    loadThemeImage(path, "Bottom right selected button", 36, 36, NULL, themeImageBottomRightButtonSelected, themeImages);
    
    
    
    
    
    
    sprintf(path, "%shelpicon.png", themePath);
    loadThemeImage(path, "Help icon", 36, 36, NULL, themeImageHelpSymbol, themeImages);
    
    sprintf(path, "%sbackicon.png", themePath);
    loadThemeImage(path, "Back icon", 36, 36, NULL, themeImageBackSymbol, themeImages);

    sprintf(path, "%shomeicon.png", themePath);
    loadThemeImage(path, "Home icon", 36, 36, NULL, themeImageHomeSymbol, themeImages);
    
    sprintf(path, "%ssettingsicon.png", themePath);
    loadThemeImage(path, "Settings icon", 36, 36, NULL, themeImageSettingsSymbol, themeImages);
    
    sprintf(path, "%sfoldersicon.png", themePath);
    loadThemeImage(path, "Folders icon", 36, 36, NULL, themeImageFoldersSymbol, themeImages);
    
    
    numProgressWheelImages = 0;
    
    int frame;
    for (frame = 0; frame < maxProgressWheelImages; frame++) {
        char progressWheelFrameFilename[128];
        sprintf(progressWheelFrameFilename, "%sprogressWheelFrame%d.png", themePath, frame+1);
        
        bool loadSuccess = loadThemeImage(progressWheelFrameFilename, "Progress wheel frame", 36, 36, NULL, frame, progressWheelImages);
        if (loadSuccess) {
            numProgressWheelImages++;
        }
        else {
            logText(progressWheelFrameFilename);
            break;
        }
    }
    
    themeHasProgressWheel = (numProgressWheelImages > 0);
    
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

void drawProgressWheelImage(int frame, gfxScreen_t screen, int x, int y) {
    themeImage wheelFrameImage = progressWheelImages[frame];
    if (wheelFrameImage.exists) {
        drawThemeImageCheckAlpha(wheelFrameImage.spriteData, screen, x, y, wheelFrameImage.h, wheelFrameImage.w, wheelFrameImage.hasAlpha);
    }
}

bool themeImageExists(int imageID) {    
    themeImage aThemeImage = themeImages[imageID];
    return aThemeImage.exists;
}