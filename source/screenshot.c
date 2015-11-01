//Based on:
//          http://www.labbookpages.co.uk/software/imgProc/libPNG.html

#include "screenshot.h"

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <png.h>

#include "string.h"

#include "filesystem.h"

FILE *fp = NULL;
png_structp png_ptr = NULL;
png_infop info_ptr = NULL;
png_bytep row = NULL;

void createPNG(char * filePath, int width, int height) {
    fp = NULL;
    png_ptr = NULL;
    info_ptr = NULL;
    fp = fopen(filePath, "wb");

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    row = (png_bytep) malloc(3 * 400 * sizeof(png_byte));
}

void savePNG() {
    png_write_end(png_ptr, NULL);

    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);
}

void drawPNG(int width, int height, u8 * imageSource, int xAdjust, int canvasWidth, int fillR, int fillG, int fillB) {
    int x, y;

    for (y=0 ; y<height ; y++) {
        int moveRight = height - y;

        for (x=0; x<canvasWidth; x++) {
            png_byte *ptr = &(row[x*3]);
            ptr[0] = fillR;
            ptr[1] = fillG;
            ptr[2] = fillB;
        }

        for (x=0 ; x<width ; x++) {
            int moveDown = x * height;
            int offset = (moveRight + moveDown) * 3;
            offset -= 3;

            png_byte *ptr = &(row[(x+xAdjust)*3]);

            int b, g, r;

            b = imageSource[offset];
            g = imageSource[offset+1];
            r = imageSource[offset+2];

            ptr[0] = r;
            ptr[1] = g;
            ptr[2] = b;
        }
        png_write_row(png_ptr, row);
    }
}

void fillPNGGap(int gap, int width, int fillR, int fillG, int fillB) {
    int x, y;

    for (y=0 ; y<gap ; y++) {
        for (x=0; x<width; x++) {
            png_byte *ptr = &(row[x*3]);
            ptr[0] = fillR;
            ptr[1] = fillG;
            ptr[2] = fillB;
        }
        png_write_row(png_ptr, row);
    }
}

void takeScreenshot() {
    u64 timeInSeconds = osGetTime() / 1000;
    char filePath[128];
    strcpy(filePath, "");
    sprintf(filePath, "/gridlauncher/screenshots/hbl-grid-screenshot-%llu.png", timeInSeconds);

    int gap = 10;

    createPNG(filePath, 400, 240 + 240 + gap);

    int fillR = 255;
    int fillG = 255;
    int fillB = 255;

    u16 fbWidthT, fbHeightT;
    u8* fbAdrT=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &fbWidthT, &fbHeightT);
    drawPNG(fbHeightT, fbWidthT, fbAdrT, 0, 400, fillR, fillG, fillB);

    fillPNGGap(gap, 400, fillR, fillG, fillB);

    u16 fbWidthB, fbHeightB;
    u8* fbAdrB=gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &fbWidthB, &fbHeightB);
    drawPNG(fbHeightB, fbWidthB, fbAdrB, 40, 400, fillR, fillG, fillB);


    savePNG();
}

