#pragma once
#include <3ds.h>

typedef struct themeSound {
    u8* sndbuffer;
    u32 sndsize;
    int channel;
} themeSound;

extern themeSound themeSoundBGM;
extern themeSound themeSoundMove;
extern themeSound themeSoundSelect;

void audio_stop(void);
void initThemeSounds();
void audioPlay(themeSound * aThemeSound, bool loop);
