#include "sound.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "filesystem.h"

themeSound themeSoundBGM;
themeSound themeSoundMove;
themeSound themeSoundSelect;
themeSound themeSoundBack;

#include "logText.h"

void audio_load(const char *audio, themeSound * aThemeSound){

	FILE *file = fopen(audio, "rb");
	if(file != NULL){
		fseek(file, 0, SEEK_END);
		aThemeSound->sndsize = ftell(file);
		fseek(file, 0, SEEK_SET);
		aThemeSound->sndbuffer = linearAlloc(aThemeSound->sndsize);
		fread(aThemeSound->sndbuffer, 1, aThemeSound->sndsize, file);
		fclose(file);
	}
}

void audioPlay(themeSound * aThemeSound, bool loop) {
    u32 flags;

    if (loop) {
        flags = SOUND_FORMAT_16BIT | SOUND_REPEAT;
    }
    else {
        flags = SOUND_FORMAT_16BIT;
    }

    csndPlaySound(aThemeSound->channel, flags, 44100, 1, 0, aThemeSound->sndbuffer, aThemeSound->sndbuffer, aThemeSound->sndsize);
}

void audioFree(themeSound * aThemeSound) {
    memset(aThemeSound->sndbuffer, 0, aThemeSound->sndsize);
	GSPGPU_FlushDataCache(aThemeSound->sndbuffer, aThemeSound->sndsize);
	linearFree(aThemeSound->sndbuffer);
}

void audio_stop(void){
	csndExecCmds(true);
	CSND_SetPlayState(0x8, 0);

    audioFree(&themeSoundBGM);
    audioFree(&themeSoundMove);
    audioFree(&themeSoundSelect);
}

char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    //in real code you would check for errors in malloc here.
	//perhaps this should be done?
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void loadThemeSoundOrDefault(char * filename, themeSound * aThemeSound, int channel) {
    char * themePath = currentThemePath();

    char * filePath = concat(themePath, filename);
    if (fileExists(filePath, &sdmcArchive)) {
        audio_load(filePath, aThemeSound);
        aThemeSound->channel = channel;
    }
    else {

    }
    free(filePath);
}

void initThemeSounds() {
	audio_stop();
	audio_stop();

    loadThemeSoundOrDefault("BGM.bin", &themeSoundBGM, 8);

	//char *bgmpath = concat(themePath, "BGM.bin");
	//audio_load(bgmpath, &themeSoundBGM);
	//themeSoundBGM.channel = 8;
	//free(bgmpath);

	char *movePath = concat(themePath, "movesound.bin");
	audio_load(movePath, &themeSoundMove);
	themeSoundMove.channel = 9;
	free(movePath);

	char *selectPath = concat(themePath, "selectsound.bin");
	audio_load(movePath, &themeSoundSelect);
	themeSoundSelect.channel = 10;
	free(selectPath);

	char *backPath = concat(themePath, "backsound.bin");
	audio_load(backPath, &themeSoundBack);
	themeSoundBack.channel = 10;
	free(backPath);

	audioPlay(&themeSoundBGM, true);
}
