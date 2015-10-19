/*
 * Copyright 2002-2010 Guillaume Cottenceau.
 *
 * This software may be freely redistributed under the terms
 * of the X11 license.
 *
 */

#include "wallpaper.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

#include "gfx.h"
#include "filesystem.h"
#include "config.h"

u8 * wallpaperTop;
u8 * wallpaperBottom;

bool wallpaperTopHasAlpha = false;
bool wallpaperBottomHasAlpha = false;

#include "logText.h"

int x, y;

int width, height;
png_byte color_type;
png_byte bit_depth;

png_structp png_ptr;
png_infop info_ptr;
int number_of_passes;
png_bytep * row_pointers;

bool read_png_file(char* file_name) {
    char header[8];    // 8 is the maximum size that can be checked
    
    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
//        logText("[read_png_file] File could not be opened for reading"); // We don't need to log an error about this
        return false;
    }
    
    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8)) {
        logText("[read_png_file] File is not recognized as a PNG file");
        fclose(fp);
        return false;
    }
    
    
    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    
    if (!png_ptr) {
        logText("[read_png_file] png_create_read_struct failed");
        fclose(fp);
        return false;
    }
    
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        logText("[read_png_file] png_create_info_struct failed");
        fclose(fp);
        return false;
    }
    
    if (setjmp(png_jmpbuf(png_ptr))) {
        logText("[read_png_file] Error during init_io");
        fclose(fp);
        return false;
    }
    
    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);
    
    png_read_info(png_ptr, info_ptr);
    
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    
    color_type = png_get_color_type(png_ptr, info_ptr);
    bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    
    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    
    
    /* read file */
    if (setjmp(png_jmpbuf(png_ptr))) {
        logText("[read_png_file] Error during read_image");
        fclose(fp);
        return false;
    }
    
    row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
    for (y=0; y<height; y++)
        row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
    
    png_read_image(png_ptr, row_pointers);
    
    fclose(fp);
    
    return true;
}

int bytesPerPixel;

u8 * process_file(void) {
    bytesPerPixel = 3;
    
    if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA) {
        bytesPerPixel = 4;
//        
//        logText("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGB");
//        return NULL;
    }

    
    u8 * out = malloc(width*height*bytesPerPixel);
    
    for (y=0; y<height; y++) {
        int moveRight = height - y;
        
        png_byte* row = row_pointers[y];
        for (x=0; x<width; x++) {
            int moveDown = x * height;
            
            png_byte* ptr = &(row[x*bytesPerPixel]);
            
//            int offset = (((y*height) + x) * 3);
            int offset = (moveRight + moveDown) * bytesPerPixel;
            offset -= bytesPerPixel;
            
            out[offset] = ptr[2];
            out[offset+1] = ptr[1];
            out[offset+2] = ptr[0];
            
            if (bytesPerPixel == 4) {
                out[offset+3] = ptr[3];
            }
        }
    }
    
    return out;
}

u8 * loadWallpaper(gfxScreen_t screen) {
    char * themePath = currentThemePath();
    char * sScreen = (screen == GFX_TOP) ? "top" : "bottom";
    char filename[128];
    sprintf(filename, "%swallpaper%s.png", themePath, sScreen);
    free(themePath);
    
    bool success = read_png_file(filename);
    
    if (success) {
        int requiredWidth = (screen == GFX_TOP) ? 400 : 320;
        int requiredHeight = 240;
        char * sScreen = (screen == GFX_TOP) ? "top" : "bottom";
        
        if (width != requiredWidth || height != requiredHeight) {
            char error[100];
            sprintf(error, "Wallpaper for the %s screen must be %d pixels wide and %d pixels high", sScreen, requiredWidth, requiredHeight);
            logText(error);
            return NULL;
        }
        
        u8 * out = process_file();
        return out;
    }
    
    return NULL;
}

void initWallpaper() {
    wallpaperTop = loadWallpaper(GFX_TOP);
    wallpaperTopHasAlpha = (bytesPerPixel==4);
    
    wallpaperBottom = loadWallpaper(GFX_BOTTOM);
    wallpaperBottomHasAlpha = (bytesPerPixel==4);
}

void drawWallpaper() {
    if (wallpaperTop) {
        if (wallpaperTopHasAlpha) {
            gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, wallpaperTop, 240, 400, 0, 0);
        }
        else {
            gfxDrawSprite(GFX_TOP, GFX_LEFT, wallpaperTop, 240, 400, 0, 0);
        }
    }
    
    if (wallpaperBottom) {
        if (wallpaperBottomHasAlpha) {
            gfxDrawSpriteAlphaBlend(GFX_BOTTOM, GFX_LEFT, wallpaperBottom, 240, 320, 0, 0);
        }
        else {
            gfxDrawSprite(GFX_BOTTOM, GFX_LEFT, wallpaperBottom, 240, 320, 0, 0);
        }
    }
}