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

void loadThemeImage(char * path, char * description, int expectedWidth, int expectedHeight, u8 * alphaMask, u8 * dest, bool * hasImageBool) {
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
        else {
            *hasImageBool = false;
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
    
    free(themePath);
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
            if (topWallpaperHasAlpha) {
                gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, topWallpaper, 240, 400, x, y);
            }
            else {
                gfxDrawSprite(screen, GFX_LEFT, topWallpaper, 240, 400, x, y);
            }
            break;
            
        case themeImageBottomWallpaper:
            if (bottomWallpaperHasAlpha) {
                gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, bottomWallpaper, 240, 320, x, y);
            }
            else {
                gfxDrawSprite(screen, GFX_LEFT, bottomWallpaper, 240, 320, x, y);
            }
            
            break;
            
        default:
            break;
    }
}