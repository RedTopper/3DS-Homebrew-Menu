#include <stdio.h>
#include <string.h>

#include "statusbar.h"
#include "gfx.h"

#include "wifi_full_bin.h"
#include "wifi_none_bin.h"
#include "battery_full_bin.h"
#include "battery_mid_high_bin.h"
#include "battery_mid_low_bin.h"
#include "battery_low_bin.h"
#include "battery_lowest_bin.h"
#include "battery_charging_bin.h"

#include "config.h"

#include "colours.h"
#include "MAFontRobotoRegular.h"

#include "MAGFX.h"
#include "menu.h"

u8* batteryLevels[] = {
	(u8*)battery_lowest_bin,
	(u8*)battery_lowest_bin,
	(u8*)battery_low_bin,
	(u8*)battery_mid_low_bin,
	(u8*)battery_mid_high_bin,
	(u8*)battery_full_bin,
};

#define SECONDS_IN_DAY 86400
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60

u8 topBar[18*400*4];
bool statusbarNeedsUpdate = true;

u64 lastTimeInSeconds = 0; //Used for calculating if the lid has been shut.

void randomiseThemeToolbar() { //No different then main.c's
    buildThemesList();
    int minimum_number = 2;
    int max_number = themesMenu.numEntries - 1;
    int r = rand() % (max_number + 1 - minimum_number) + minimum_number;
    menuEntry_s * randomEntry = getMenuEntry(&themesMenu, r);
    setTheme(randomEntry->executablePath);
}

void drawStatusBar(bool wifiStatus, bool charging, int batteryLevel)
{
    if (statusbarNeedsUpdate) {
        statusbarNeedsUpdate = false;
        rgbColour * rgb = tintColour();
        MAGFXTranslucentRect(18, 400, rgb->r, rgb->g, rgb->b, translucencyTopBar, topBar);
    }

    gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, topBar, 18, 400, 240-18, 0);
    
	if(lastTimeInSeconds == 0) {
		lastTimeInSeconds = osGetTime() / 1000; //get on boot.
	}
	u64 timeInSeconds = osGetTime() / 1000;
	
	if(timeInSeconds - lastTimeInSeconds > 5) {
		randomTheme = getConfigBoolForKey("randomTheme", false, configTypeMain);
		
		// /!\ Should be fixed to be something other than displayTitleID if ever merged onto the main branch!!
		bool trueRandomTheme = getConfigBoolForKey("displayTitleID", false, configTypeMain); //FIXME
		if (randomTheme && trueRandomTheme) {
			randomiseThemeToolbar();
		}
	}
	lastTimeInSeconds = timeInSeconds;
	u64 dayTime = timeInSeconds % SECONDS_IN_DAY;
	u8 hour = dayTime / SECONDS_IN_HOUR;
	u8 min = (dayTime % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
	u8 seconds = dayTime % SECONDS_IN_MINUTE;
        
    char *ampm = "";
    
    if (!clock24) {
        if (hour == 12) {
            ampm = " pm";
        }
        else if (hour == 0) {
            hour = 12;
            ampm = " am";
        }
        else if (hour > 12) {
            hour -= 12;
            ampm = " pm";
        }
        else {
            ampm = " am";
        }
    }
    
    rgbColour *light = lightTextColour();
    
	char timeString[13];
    memset(timeString, 0, 11);
	sprintf(timeString, "%02d:%02d:%02d%s", hour, min, seconds, ampm);
    int textWidth = MATextWidthInPixels(timeString, &MAFontRobotoRegular10);
    MADrawText(GFX_TOP, GFX_LEFT, 240-20, (400/2) - (textWidth/2), timeString, &MAFontRobotoRegular10, light->r, light->g, light->b);

	if(wifiStatus)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_full_bin, 18, 20, 240 - 18, 2);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)wifi_none_bin, 18, 20, 240 - 18, 2);
	}

	if(charging)
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)battery_charging_bin, 18, 27, 240 - 18, 400 - 29);
	}
	else
	{
		gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, batteryLevels[batteryLevel], 18, 27, 240 - 18, 400 - 29);
	}
}

