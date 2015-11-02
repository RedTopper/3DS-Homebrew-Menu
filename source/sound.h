#pragma once
#include <3ds.h>

typedef struct themeSound {
    u8* sndbuffer;
    u32 sndsize;
    int channel;
    bool loaded;
} themeSound;

extern themeSound themeSoundBGM;
extern themeSound themeSoundMove;
extern themeSound themeSoundSelect;
extern themeSound themeSoundBack;
extern themeSound themeSoundBoot;

void audio_stop(void);
void initThemeSounds();
void audioPlay(themeSound * aThemeSound, bool loop);
void playBootSound();
