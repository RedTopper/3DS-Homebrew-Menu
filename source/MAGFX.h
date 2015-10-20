#pragma once
#include <3ds.h>

extern int panelAlphaTop;
extern int panelAlphaBottom;
extern int panelLeftOffsetTop;
extern bool panelsDrawn;


void MAGFXImageWithRGBAndAlphaMask(u8 r, u8 g, u8 b, u8* GRBAAlphaSource, u8* dest, int width, int height);
void MAGFXApplyAlphaMask(u8* GRBImageSource, u8* GRBAAlphaSource, u8* dest, int width, int height);
void MAGFXTranslucentRect(int width, int height, int r, int g, int b, int alpha, u8* dest);
void MAGFXDrawPanel(gfxScreen_t screen);
void drawDisk(char * text);

//u8 * MAGFXScaleSprite(u8* spriteData, u8* newSpriteData, u16 width, u16 height, int scaleFactor, bool alpha);