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

//typedef struct themeImage {
//    themeImageID imageID;
//    u8 * spriteData;
//    
//    char executablePath[ENTRY_PATHLENGTH+1];
//    char name[ENTRY_NAMELENGTH+1];
//    char description[ENTRY_DESCLENGTH+1];
//    char author[ENTRY_AUTHORLENGTH+1];
//    u8 iconData[ENTRY_ICONSIZE];
//    descriptor_s descriptor;
//    struct menuEntry_s* next;
//    int page;
//    int row;
//    int col;
//    int iconX;
//    int iconY;
//    int iconW;
//    int iconH;
//    bool hidden;
//    bool isRegionFreeEntry;
//    bool isTitleEntry;
//    u64 title_id;
//    bool * showTick;
//    void (*callback)();
//    void *callbackObject1;
//    void *callbackObject2;
//    bool drawFirstLetterOfName;
//    bool drawFullTitle;
//} themeImage;

u8 appBackgroundImage[56*56*4];
bool themeHasAppBackgroundImage = false;

u8 appBackgroundImageSelected[56*56*4];
bool themeHasAppBackgroundImageSelected = false;

u8 cartBackgroundImage[59*59*4];
bool themeHasCartBackgroundImage = false;

u8 cartBackgroundImageSelected[59*59*4];
bool themeHasCartBackgroundImageSelected = false;

u8 topWallpaper[400*240*4];
bool themeHasTopWallpaper = false;
bool topWallpaperHasAlpha;

u8 bottomWallpaper[320*240*4];
bool themeHasBottomWallpaper = false;
bool bottomWallpaperHasAlpha;

//u8 topWallpaperInfo[400*240*4];
//bool themeHasTopWallpaperInfo = false;
//bool topWallpaperInfoHasAlpha;

u8 buttonTopLeft[36*36*4];
bool themeHasTopLeftButton = false;
bool topLeftButtonHasAlpha;

u8 buttonTopRight[36*36*4];
bool themeHasTopRightButton = false;
bool topRightButtonHasAlpha;

u8 buttonBottomLeft[36*36*4];
bool themeHasBottomLeftButton = false;
bool bottomLeftButtonHasAlpha;

u8 buttonBottomRight[36*36*4];
bool themeHasBottomRightButton = false;
bool bottomRightButtonHasAlpha;

u8 buttonTopLeftSelected[36*36*4];
bool themeHasTopLeftButtonSelected = false;
bool topLeftSelectedButtonHasAlpha;

u8 buttonTopRightSelected[36*36*4];
bool themeHasTopRightButtonSelected = false;
bool topRightSelectedButtonHasAlpha;

u8 buttonBottomLeftSelected[36*36*4];
bool themeHasBottomLeftButtonSelected = false;
bool bottomLeftSelectedButtonHasAlpha;

u8 buttonBottomRightSelected[36*36*4];
bool themeHasBottomRightButtonSelected = false;
bool bottomRightSelectedButtonHasAlpha;

u8 helpSymbol[36*36*4];
bool themeHasHelpSymbol = false;
bool helpSymbolHasAlpha;

u8 backSymbol[36*36*4];
bool themeHasBackSymbol = false;
bool backSymbolHasAlpha;

u8 homeSymbol[36*36*4];
bool themeHasHomeSymbol = false;
bool homeSymbolHasAlpha;

u8 settingsSymbol[36*36*4];
bool themeHasSettingsSymbol = false;
bool settingsSymbolHasAlpha;

u8 foldersSymbol[36*36*4];
bool themeHasFoldersSymbol = false;
bool foldersSymbolHasAlpha;

//u8 progressWheel1[36*36*4];
//u8 progressWheel2[36*36*4];
//u8 progressWheel3[36*36*4];
//u8 progressWheel4[36*36*4];
//u8 progressWheel5[36*36*4];
//u8 progressWheel6[36*36*4];
//bool themeHasProgressWheel = false;
//bool progressWheelHasAlpha;

void loadThemeImage(char * path, char * description, int expectedWidth, int expectedHeight, u8 * alphaMask, u8 * dest, bool * hasImageBool) {
    *hasImageBool = false;
    
    bool success = read_png_file(path);
    
    if (success) {
        if (pngWidth != expectedWidth || pngHeight != expectedHeight) {
            char error[256];
            sprintf(error, "%s must be %dx%d pixels", description, expectedWidth, expectedHeight);
            logText(error);
            return;
        }
        
        u8 * out = process_png_file();
        
        if (out) {
            if (alphaMask) {
                MAGFXApplyAlphaMask(out, alphaMask, dest, expectedWidth, expectedHeight, (bytesPerPixel==4));
            }
            else {
                memcpy(dest, out, (expectedWidth*expectedHeight*bytesPerPixel));
            }
            
            *hasImageBool = true;
        }
        
        free(out);
    }
}

void initThemeImages() {
    char * themePath = currentThemePath();
    char path[128];
    
    
    sprintf(path, "%sappbackground.png", themePath);
    loadThemeImage(path, "App background", 56, 56, (u8*)appbackgroundalphamask_bin, (u8*)appBackgroundImage, &themeHasAppBackgroundImage);
    
    
    sprintf(path, "%sappbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected app background", 56, 56, (u8*)appbackgroundalphamask_bin, (u8*)appBackgroundImageSelected, &themeHasAppBackgroundImageSelected);
    
    
    
    
    
    
    
    

    sprintf(path, "%scartbackground.png", themePath);
    loadThemeImage(path, "Cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, (u8*)cartBackgroundImage, &themeHasCartBackgroundImage);

    sprintf(path, "%scartbackgroundselected.png", themePath);
    loadThemeImage(path, "Selected cart background", 59, 59, (u8*)cartbackgroundalphamask_bin, (u8*)cartBackgroundImageSelected, &themeHasCartBackgroundImageSelected);
    
    
    
    
    
    
    
    
    
    sprintf(path, "%swallpapertop.png", themePath);
    loadThemeImage(path, "Top wallpaper", 400, 240, NULL, (u8*)topWallpaper, &themeHasTopWallpaper);
    topWallpaperHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%swallpaperbottom.png", themePath);
    loadThemeImage(path, "Bottom wallpaper", 320, 240, NULL, (u8*)bottomWallpaper, &themeHasBottomWallpaper);
    bottomWallpaperHasAlpha = (bytesPerPixel == 4);
    
//    sprintf(path, "%swallpapertopinfo.png", themePath);
//    loadThemeImage(path, "Top info wallpaper", 400, 240, NULL, (u8*)topWallpaperInfo, &themeHasTopWallpaperInfo);
//    topWallpaperInfoHasAlpha = (bytesPerPixel == 4);
    
    
    
    
    
    
    
    
    
    sprintf(path, "%sbuttontopleft.png", themePath);
    loadThemeImage(path, "Top left button", 36, 36, NULL, (u8*)buttonTopLeft, &themeHasTopLeftButton);
    topLeftButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttontopright.png", themePath);
    loadThemeImage(path, "Top right button", 36, 36, NULL, (u8*)buttonTopRight, &themeHasTopRightButton);
    topRightButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttonbottomleft.png", themePath);
    loadThemeImage(path, "Bottom left button", 36, 36, NULL, (u8*)buttonBottomLeft, &themeHasBottomLeftButton);
    bottomLeftButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttonbottomright.png", themePath);
    loadThemeImage(path, "Bottom right button", 36, 36, NULL, (u8*)buttonBottomRight, &themeHasBottomRightButton);
    bottomRightButtonHasAlpha = (bytesPerPixel == 4);
    

    
    
    
    
    sprintf(path, "%sbuttontopleftselected.png", themePath);
    loadThemeImage(path, "Top left selected button", 36, 36, NULL, (u8*)buttonTopLeftSelected, &themeHasTopLeftButtonSelected);
    topLeftSelectedButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttontoprightselected.png", themePath);
    loadThemeImage(path, "Top right selected button", 36, 36, NULL, (u8*)buttonTopRightSelected, &themeHasTopRightButtonSelected);
    topRightSelectedButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttonbottomleftselected.png", themePath);
    loadThemeImage(path, "Bottom left selected button", 36, 36, NULL, (u8*)buttonBottomLeftSelected, &themeHasBottomLeftButtonSelected);
    bottomLeftSelectedButtonHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbuttonbottomrightselected.png", themePath);
    loadThemeImage(path, "Bottom right selected button", 36, 36, NULL, (u8*)buttonBottomRightSelected, &themeHasBottomRightButtonSelected);
    bottomRightSelectedButtonHasAlpha = (bytesPerPixel == 4);
    
    
    
    
    
    
    sprintf(path, "%shelpicon.png", themePath);
    loadThemeImage(path, "Help icon", 36, 36, NULL, (u8*)helpSymbol, &themeHasHelpSymbol);
    helpSymbolHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sbackicon.png", themePath);
    loadThemeImage(path, "Back icon", 36, 36, NULL, (u8*)backSymbol, &themeHasBackSymbol);
    backSymbolHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%shomeicon.png", themePath);
    loadThemeImage(path, "Home icon", 36, 36, NULL, (u8*)homeSymbol, &themeHasHomeSymbol);
    homeSymbolHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%ssettingsicon.png", themePath);
    loadThemeImage(path, "Settings icon", 36, 36, NULL, (u8*)settingsSymbol, &themeHasSettingsSymbol);
    settingsSymbolHasAlpha = (bytesPerPixel == 4);
    
    sprintf(path, "%sfoldersicon.png", themePath);
    loadThemeImage(path, "Folders icon", 36, 36, NULL, (u8*)foldersSymbol, &themeHasFoldersSymbol);
    foldersSymbolHasAlpha = (bytesPerPixel == 4);
    
    
    

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

void drawThemeImage(themeImageID imageID, gfxScreen_t screen, int x, int y) {
    switch (imageID) {
        case themeImageAppBackground:
            gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, appBackgroundImage, 56, 56, x, y);
            break;
            
        case themeImageAppBackgroundSelected:
            gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, appBackgroundImageSelected, 56, 56, x, y);
            break;
            
            
            
            
            
            
            
            
        case themeImageCartBackground:
            gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, cartBackgroundImage, 59, 59, x, y);
            break;
            
        case themeImageCartBackgroundSelected:
            gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, cartBackgroundImageSelected, 59, 59, x, y);
            break;
            
            
            
            
            
            
            
            
        case themeImageTopWallpaper:
            drawThemeImageCheckAlpha(topWallpaper, screen, x, y, 240, 400, topWallpaperHasAlpha);
            break;
            
        case themeImageBottomWallpaper:
            drawThemeImageCheckAlpha(bottomWallpaper, screen, x, y, 240, 320, bottomWallpaperHasAlpha);
            break;
            
//        case themeImageTopWallpaperInfo:
//            drawThemeImageCheckAlpha(topWallpaperInfo, screen, x, y, 240, 400, topWallpaperInfoHasAlpha);
//            break;
            
            
            
            
            
            
            
            
        case themeImageTopLeftButton:
            drawThemeImageCheckAlpha(buttonTopLeft, screen, x, y, 36, 36, topLeftButtonHasAlpha);
            break;
            
        case themeImageTopRightButton:
            drawThemeImageCheckAlpha(buttonTopRight, screen, x, y, 36, 36, topRightButtonHasAlpha);
            break;
            
        case themeImageBottomLeftButton:
            drawThemeImageCheckAlpha(buttonBottomLeft, screen, x, y, 36, 36, bottomLeftButtonHasAlpha);
            break;
            
        case themeImageBottomRightButton:
            drawThemeImageCheckAlpha(buttonBottomRight, screen, x, y, 36, 36, bottomRightButtonHasAlpha);
            break;
            
            
            
            
            
            
        case themeImageTopLeftButtonSelected:
            drawThemeImageCheckAlpha(buttonTopLeftSelected, screen, x, y, 36, 36, topLeftSelectedButtonHasAlpha);
            break;
            
        case themeImageTopRightButtonSelected:
            drawThemeImageCheckAlpha(buttonTopRightSelected, screen, x, y, 36, 36, topRightSelectedButtonHasAlpha);
            break;
            
        case themeImageBottomLeftButtonSelected:
            drawThemeImageCheckAlpha(buttonBottomLeftSelected, screen, x, y, 36, 36, bottomLeftSelectedButtonHasAlpha);
            break;
            
        case themeImageBottomRightButtonSelected:
            drawThemeImageCheckAlpha(buttonBottomRightSelected, screen, x, y, 36, 36, bottomRightSelectedButtonHasAlpha);
            break;
            
            
            
            
            
            
        case themeImageHelpSymbol:
            drawThemeImageCheckAlpha(helpSymbol, screen, x, y, 36, 36, helpSymbolHasAlpha);
            break;
            
        case themeImageBackSymbol:
            drawThemeImageCheckAlpha(backSymbol, screen, x, y, 36, 36, backSymbolHasAlpha);
            break;
            
        case themeImageHomeSymbol:
            drawThemeImageCheckAlpha(homeSymbol, screen, x, y, 36, 36, homeSymbolHasAlpha);
            break;
            
        case themeImageSettingsSymbol:
            drawThemeImageCheckAlpha(settingsSymbol, screen, x, y, 36, 36, settingsSymbolHasAlpha);
            break;
            
        case themeImageFoldersSymbol:
            drawThemeImageCheckAlpha(foldersSymbol, screen, x, y, 36, 36, foldersSymbolHasAlpha);
            break;
            
            
            
            
            
            
            
            
        default:
            break;
    }
}