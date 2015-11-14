#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <sys/stat.h>
#include <malloc.h>

#include "gfx.h"
#include "menu.h"
#include "background.h"
#include "statusbar.h"
#include "filesystem.h"
#include "netloader.h"
#include "regionfree.h"
#include "boot.h"
#include "titles.h"
#include "folders.h"
#include "logo_bin.h"
#include "MAFontRobotoRegular.h"
#include "alert.h"
#include "logText.h"
#include "colours.h"

#include "screenshot.h"
#include "config.h"

#include "MAGFX.h"

#include "help.h"
#include "touchblock.h"
#include "folders.h"
#include "themegfx.h"
#include "version.h"
#include "sound.h"

#include "logText.h"

u8 sdmcCurrent = 0;
u64 nextSdCheck = 0;

u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;

bool die = false;
bool dieImmediately = false;
bool showRebootMenu = false;
bool startRebootProcess = false;

//Handle threadHandle, threadRequest;
//#define STACKSIZE (4 * 1024)

static enum
{
	HBMENU_DEFAULT,
	HBMENU_REGIONFREE,
	HBMENU_TITLESELECT,
	HBMENU_TITLETARGET_ERROR,
	HBMENU_NETLOADER_ACTIVE,
	HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2,
	HBMENU_NETLOADER_ERROR,
} hbmenu_state = HBMENU_DEFAULT;

int debugValues[100];

//void drawDebug()
//{
//	char str[256];
//	sprintf(str, "hello3 %08X %d %d %d %d %d %d %d\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n", debugValues[50], debugValues[51], debugValues[52], debugValues[53], debugValues[54], debugValues[55], debugValues[56], debugValues[57], debugValues[58], debugValues[59], debugValues[60], debugValues[61], debugValues[62], debugValues[63], debugValues[64], debugValues[65], debugValues[66], debugValues[67], debugValues[68], debugValues[69]);
//
//    rgbColour * dark = darkTextColour();
//
//    MADrawText(GFX_TOP, GFX_LEFT, 48, 100, str, &MAFontRobotoRegular8, dark->r, dark->g, dark->b);
//}

extern void closeReboot() {
    showRebootMenu = false;
}

extern void doReboot() {
    aptOpenSession();
    APT_HardwareResetAsync();
    aptCloseSession();
}

/*
 Shutdown code contributed by daxtsu from gbatemp
 */
void shutdown3DS()
{
    Handle ptmSysmHandle = 0;
    Result result = srvGetServiceHandle(&ptmSysmHandle, "ns:s");
    if (result != 0)
        return;

    // http://3dbrew.org/wiki/NSS:ShutdownAsync

    u32 *commandBuffer = getThreadCommandBuffer();
    commandBuffer[0] = 0x000E0000;

    svcSendSyncRequest(ptmSysmHandle);
    svcCloseHandle(ptmSysmHandle);
}

void launchSVDTFromTitleMenu() {
    menuEntry_s* me = getMenuEntry(&titleMenu, titleMenu.selectedEntry);

    if (me) {
        if (me->title_id) {
            if (me->title_id > 0) {
                titleInfo_s* ret = NULL;
                ret = getTitleWithID(&titleBrowser, me->title_id);
                targetProcessId = -2;
                target_title = *ret;
                die = true;
            }
        }
    }
}

void exitServices() {
    // cleanup whatever we have to cleanup
	audio_stop();
	csndExit();

    freeThemeImages();
    netloader_exit();
    titlesExit();
    ptmExit();
    acExit();
    hidExit();
    gfxExit();
    closeSDArchive();
    exitFilesystem();
    aptExit();
    srvExit();
}

void launchTitleFromMenu(menu_s* m) {
    menuEntry_s* me = getMenuEntry(m, m->selectedEntry);

    if (me) {
        if (me->title_id) {
            if (me->title_id > 0) {
                titleInfo_s* ret = NULL;
                ret = getTitleWithID(&titleBrowser, me->title_id);

                if (ret) {
                    exitServices();
                    regionFreeRun2(ret->title_id & 0xffffffff, (ret->title_id >> 32) & 0xffffffff, ret->mediatype, 0x1);
                    dieImmediately = true;
                }
            }
        }
        else {
            die = true;
        }
    }
}

void putTitleMenu(char * barTitle) {
    drawGrid(&titleMenu);
    drawBottomStatusBar(barTitle);
}

#include "progresswheel.h"

void renderFrame()
{
	// background stuff

    rgbColour * bgc = backgroundColour();

    gfxFillColor(GFX_BOTTOM, GFX_LEFT, (u8[]){bgc->r, bgc->g, bgc->b});
    gfxFillColor(GFX_TOP, GFX_LEFT, (u8[]){bgc->r, bgc->g, bgc->b});

    //Wallpaper
    if (themeImageExists(themeImageTopWallpaperInfo) && ((menuStatus == menuStatusHelp && showingHelpDetails) || menuStatus == menuStatusColourAdjust || menuStatus == menuStatusTranslucencyTop || menuStatus == menuStatusTranslucencyBottom || menuStatus == menuStatusPanelSettings || showRebootMenu)) {
        drawThemeImage(themeImageTopWallpaperInfo, GFX_TOP, 0, 0);
    }
    else if (themeImageExists(themeImageTopWallpaper)) {
        drawThemeImage(themeImageTopWallpaper, GFX_TOP, 0, 0);
    }

    if (themeImageExists(themeImageBottomWallpaperNonGrid) && ((menuStatus == menuStatusHelp && showingHelpDetails) || menuStatus == menuStatusColourAdjust || menuStatus == menuStatusTranslucencyTop || menuStatus == menuStatusTranslucencyBottom || menuStatus == menuStatusPanelSettings || showRebootMenu)) {
        drawThemeImage(themeImageBottomWallpaperNonGrid, GFX_BOTTOM, 0, 0);
    }

    else if (themeImageExists(themeImageBottomWallpaper)) {
        drawThemeImage(themeImageBottomWallpaper, GFX_BOTTOM, 0, 0);
    }

//    drawWallpaper();

	// // debug text
	// drawDebug();

//    if (!preloadTitles && titlemenuIsUpdating) {
//        drawDisk("Loading titles");
//    }
//
//    else {


        //menu stuff
        if (showRebootMenu) {
            //about to reboot
            char buttonTitles[3][32];

            if (startRebootProcess) {
                strcpy(buttonTitles[0], "Rebooting...");
            }
            else {
                strcpy(buttonTitles[0], "Reboot");
            }

            strcpy(buttonTitles[1], "Power off");
            strcpy(buttonTitles[2], "Back");

            int alertResult = drawAlert("Reboot", "You're about to reboot your console into the Home Menu.", NULL, 3, buttonTitles);

            if (startRebootProcess) {
                doReboot();
            }
            else {
                if (alertResult == 0) {
                    startRebootProcess = true;
                }
                else if (alertResult == 1) {
                    shutdown3DS();
                }
                else if (alertResult == 2 || alertResult == alertButtonKeyB) {
                    closeReboot();
                }
            }

        }else if(!sdmcCurrent)
        {
            //no SD
            drawAlert("No SD detected", "It looks like your 3DS doesn't have an SD inserted into it. Please insert an SD card for optimal homebrew launcher performance!", NULL, 0, NULL);
        }else if(sdmcCurrent<0)
        {
            //SD error
            drawAlert("SD Error", "Something unexpected happened when trying to mount your SD card. Try taking it out and putting it back in. If that doesn't work, please try again with another SD card.", NULL, 0, NULL);

        }else if(hbmenu_state == HBMENU_NETLOADER_ACTIVE){
            char bof[256];
            u32 ip = gethostid();
            sprintf(bof,
                "NetLoader Active - waiting for 3dslink connection\n\nIP: %lu.%lu.%lu.%lu, Port: %d\n\nB : Cancel\n",
                ip & 0xFF, (ip>>8)&0xFF, (ip>>16)&0xFF, (ip>>24)&0xFF, NETLOADER_PORT);

            drawAlert("NetLoader", bof, NULL, 0, NULL);
        }else if(hbmenu_state == HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2){
            drawAlert("NetLoader", "The NetLoader is currently unavailable. :( This might be normal and fixable. Try and enable it ?\n\nA : Yes\nB : No\n", NULL, 0, NULL);
        }else if(hbmenu_state == HBMENU_REGIONFREE){
            if(regionFreeGamecardIn)
            {
                drawMenuEntry(&gamecardMenuEntry, GFX_BOTTOM, true, &menu, 0, 0, false);

                drawAlert("Region free launcher", "The region free launcher is ready to run your out-of-region gamecard !\n\nA : Play\nB : Cancel", NULL, 0, NULL);
            }else{
                drawAlert("Region free launcher", "The region free loader cannot detect a gamecard in the slot.\nPlease insert a gamecard in your console before continuing.\n\nB : Cancel\n", NULL, 0, NULL);
            }
        }else if(hbmenu_state == HBMENU_TITLESELECT){

            if (updateGrid(&titleMenu)) {
                launchSVDTFromTitleMenu();
            }
            else {
                putTitleMenu("Select Title");

                if (titlemenuIsUpdating) {
                    drawProgressWheel(GFX_BOTTOM, GFX_LEFT, 0, 0);
                }
            }
        }else if(hbmenu_state == HBMENU_TITLETARGET_ERROR){
            drawAlert("Missing target title", "The application you are trying to run requested a specific target title.\nPlease make sure you have that title !\n\nB : Back\n", NULL, 0, NULL);
        }else if(hbmenu_state == HBMENU_NETLOADER_ERROR){
            netloader_draw_error();
        }else{
            //got SD

            if (menuStatus == menuStatusHomeMenuApps) {
                putTitleMenu("Select Title to Launch");

                if (titlemenuIsUpdating) {
                    drawProgressWheel(GFX_BOTTOM, GFX_LEFT, 0, 0);
                }
            }
            else if (menuStatus == menuStatusFolders) {
                drawGrid(&foldersMenu);
                drawBottomStatusBar("Select folder");
            }
            else if (menuStatus == menuStatusTitleFiltering) {
                putTitleMenu("Tap titles to show or hide them");

                if (titlemenuIsUpdating) {
                    drawProgressWheel(GFX_BOTTOM, GFX_LEFT, 0, 0);
                }
            }
            else if (menuStatus == menuStatusSettings) {
                drawGrid(&settingsMenu);
                drawBottomStatusBar("Settings");
            }
            else if (menuStatus == menuStatusGridSettings) {
                drawGrid(&gridSettingsMenu);
                drawBottomStatusBar("Grid settings");
            }
            else if (menuStatus == menuStatusWaterSettings) {
                drawGrid(&waterMenu);
                drawBottomStatusBar("Water settings");
            }
            else if (menuStatus == menuStatusThemeSettings) {
                drawGrid(&themeSettingsMenu);
                drawBottomStatusBar("Theme settings");
            }
            else if (menuStatus == menuStatusThemeSelect) {
                drawGrid(&themesMenu);
                drawBottomStatusBar("Select theme");
            }
            else if (menuStatus == menuStatusColourSettings) {
                drawGrid(&colourSelectMenu);
                drawBottomStatusBar("Colours");
            }
            else if (menuStatus == menuStatusHelp) {
                drawHelp();
            }
            else if (menuStatus == menuStatusColourAdjust) {
                drawColourAdjuster();
                drawBottomStatusBar("Colour adjustment");
            }
            else if (menuStatus == menuStatusTranslucencyTop) {
                drawTranslucencyAdjust(GFX_TOP);
                drawBottomStatusBar("Top screen translucency");
            }
            else if (menuStatus == menuStatusTranslucencyBottom) {
                drawTranslucencyAdjust(GFX_BOTTOM);
                drawBottomStatusBar("Bottom screen translucency");
            }
            else if (menuStatus == menuStatusPanelSettings) {
                drawPanelTranslucencyAdjust();
                drawBottomStatusBar("Panel settings");
            }
            else if (menuStatus == menuStatusHansMissingError) {
                char buttonTitles[3][32];
                strcpy(buttonTitles[0], "OK");
                drawAlert("Hans isn't here...", "Please copy hans.3dsx and hans.xml in /gridlauncher/hans/ on your 3ds SD card.", NULL, 1, buttonTitles);
            }
            else {
                drawMenu(&menu);
            }
        }
//    }



    drawBackground();

    if (showLogo) {
        gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, (u8*)logo_bin, 20, 214, 0, 400-214);
    }

    drawStatusBar(wifiStatus, charging, batteryLevel);
}

// handled in main
// doing it in main is preferred because execution ends in launching another 3dsx
void __appInit()
{
}

// same
void __appExit()
{
}

void showTitleMenu(titleBrowser_s * aTitleBrowser, menu_s * aTitleMenu, int newMenuStatus, bool filter, bool forceHideRegionFree, bool setFilterTicks) {
    if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {

        updateTitleMenu(&titleBrowser, &titleMenu, "Loading titles", filter, forceHideRegionFree, setFilterTicks);
//        titleMenuInitialLoadDone = true;
    }

//    if (forceHideRegionFree) {
//        menuEntry_s *first = getMenuEntry(aTitleMenu, 0);
//        strcpy(first->name, "foo");
//        first->hidden = true;
//    }
    //    updateMenuIconPositions(aTitleMenu);
    //    gotoFirstIcon(aTitleMenu);


    setMenuStatus(newMenuStatus);
}

void showSVDTTitleSelect() {
//    if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {
//        updateTitleMenu(&titleBrowser, &titleMenu, "Loading titles", true, false, false);
//        titleMenuInitialLoadDone = true;
//    }
//
    showTitleMenu(&titleBrowser, &titleMenu, menuStatusTitleBrowser, true, false, false);
    hbmenu_state = HBMENU_TITLESELECT;

    if (animatedGrids) {
        startTransition(transitionDirectionUp, menu.pagePosition, &menu);
    }
}

void showHomeMenuTitleSelect() {
    checkReturnToGrid(&titleMenu);

    showTitleMenu(&titleBrowser, &titleMenu, menuStatusHomeMenuApps, true, false, false);

    if (animatedGrids) {
        startTransition(transitionDirectionUp, menu.pagePosition, &menu);
    }
}

void showFilterTitleSelect() {
    titleMenuInitialLoadDone = false;
    showTitleMenu(&titleBrowser, &titleMenu, menuStatusTitleFiltering, false, true, true);
    if (animatedGrids) {
        startTransition(transitionDirectionDown, settingsMenu.pagePosition, &settingsMenu);
    }
}

void closeTitleBrowser() {
    setMenuStatus(menuStatusIcons);
    checkReturnToGrid(&menu);
    hbmenu_state = HBMENU_DEFAULT;

    if (animatedGrids) {
        startTransition(transitionDirectionDown, titleMenu.pagePosition, &titleMenu);
    }
}

bool gamecardWasIn;
bool gamecardStatusChanged;

//void threadMain(void *arg) {
//
//    while(1) {
//        svcWaitSynchronization(threadRequest, U64_MAX);
//        svcClearEvent(threadRequest);
//
//        updateTitleMenu(&titleBrowser, &titleMenu, NULL, true, false);
//
//        titleMenuInitialLoadDone = true;
//
//        svcExitThread();
//    }
//}

void randomiseTheme() {
    buildThemesList();
    int minimum_number = 2;
    int max_number = themesMenu.numEntries - 1;
    int r = rand() % (max_number + 1 - minimum_number) + minimum_number;
    menuEntry_s * randomEntry = getMenuEntry(&themesMenu, r);
    setTheme(randomEntry->executablePath);
}

#include "MAFontRobotoRegular.h"

int main(int argc, char *argv[])
{
	srvInit();
	aptInit();
	gfxInitDefault();

    logTextP("Clear background", "/bootlog.txt");

	u8* framebuf_top;
	u8* framebuf_bot;
	framebuf_top = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	framebuf_bot = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	memset(framebuf_top, 0, 400 * 240 * 3); //clear the screen to black
	memset(framebuf_bot, 0, 320 * 240 * 3); //ensures no graphical glitching shows.

	initFilesystem();

    openSDArchive();

	Result r = csndInit();//start Audio Lib
	audioActive = (r == 0);

    int bootAttempts = getConfigIntForKey("bootAttempts", 0, configTypeMain);

    if (!audioActive) {
        if (bootAttempts < 5) {
            bootAttempts++;
            setConfigInt("bootAttempts", bootAttempts, configTypeMain);
            saveConfigWithType(configTypeMain);

            menuEntry_s* me = malloc(sizeof(menuEntry_s));
            initMenuEntry(me, "/boot.3dsx", "launcher", "", "", NULL);
            scanMenuEntry(me);
            exitServices();
            regionFreeExit();
            return bootApp(me->executablePath, &me->descriptor.executableMetadata, me->arg);
        }
    }

    if (bootAttempts > 0) {
        setConfigInt("bootAttempts", 0, configTypeMain);
        saveConfigWithType(configTypeMain);
    }

	// Moved this here as rand() is used for choosing a random theme
    srand(svcGetSystemTick());

    randomTheme = getConfigBoolForKey("randomTheme", false, configTypeMain);

    int startMs = 0;

    if (randomTheme) {
        randomiseTheme();
    }
    else {
        audio_stop();

        loadSplashImages();

        if (themeImageExists(themeImageSplashTop)) {
            drawThemeImage(themeImageSplashTop, GFX_TOP, 0, 0);
        }

        if (themeImageExists(themeImageSplashBottom)) {
            drawThemeImage(themeImageSplashBottom, GFX_BOTTOM, 0, 0);
        }

        gfxFlip();

        startMs = osGetTime();
        playBootSound();

        initThemeImages();
        initThemeSounds();
        initColours();
    }



    int endMs = 0;
    int delayMs = 0;
    unsigned long long int delayNs = 0;

	startMs = osGetTime();

    logTextP("Init services", "/bootlog.txt");

	hidInit();
	acInit();
	ptmInit();
	titlesInit();
	regionFreeInit();
	netloader_init();

	logTextP("Set CPU speed", "/bootlog.txt");

	osSetSpeedupEnable(true);

	logTextP("Create folders", "/bootlog.txt");

    mkdir(rootPath, 777);
    mkdir(themesPath, 777);
    mkdir(foldersPath, 777);
    mkdir(defaultThemePath, 777);
    mkdir("/gridlauncher/screenshots/", 777);

    logTextP("APT Set CPU time limit", "/bootlog.txt");

	// offset potential issues caused by homebrew that just ran
	aptOpenSession();
	APT_SetAppCpuTimeLimit(0);
	aptCloseSession();

    logTextP("Init background, menu and title browser", "/bootlog.txt");

    initBackground();
    //	initErrors();

	initMenu(&menu);
	initTitleBrowser(&titleBrowser, NULL);

    logTextP("Scan HB directory", "/bootlog.txt");

	u8 sdmcPrevious = 0;
	FSUSER_IsSdmcDetected(&sdmcCurrent);
	if(sdmcCurrent == 1)
	{
		scanHomebrewDirectory(&menu, currentFolder());
	}

	sdmcPrevious = sdmcCurrent;
	nextSdCheck = osGetTime()+250;
//	srand(svcGetSystemTick());

    gamecardWasIn = regionFreeGamecardIn;



    int frameRate = 60;
    int frameMs = 1000 / frameRate;

    logTextP("Log launcher info", "/bootlog.txt");

    char * glInfo = (char*)malloc(1024);

    if (argc > 0) {
        sprintf(glInfo, "%s|%d", argv[0], currentversion);
    }
    else {
        sprintf(glInfo, "sdmc:/boot.3dsx|%d", currentversion);
    }

    logTextP(glInfo, "/gridlauncher/glinfo.txt");
    free(glInfo);

    waitForDurationOfSound(&themeSoundBoot, startMs);

    startBGM();

    logTextP("Enter main loop", "/bootlog.txt");

	while(aptMainLoop()) {
        if (die || dieImmediately) {
            break;
        }

        startMs = osGetTime();


//        if (preloadTitles && !titleMenuInitialLoadDone && !titlemenuIsUpdating) {
//            svcSignalEvent(threadRequest);
//        }

        if (titleMenuInitialLoadDone && gamecardWasIn != regionFreeGamecardIn) {
            gamecardWasIn = regionFreeGamecardIn;

            if (titleMenu.numEntries > 0) {
                menuEntry_s * gcme = getMenuEntry(&titleMenu, 0);
                gcme->hidden = !regionFreeGamecardIn;
                updateMenuIconPositions(&titleMenu);
                gotoFirstIcon(&titleMenu);
            }
        }

        if (killTitleBrowser) {
            killTitleBrowser = false;
            closeTitleBrowser();
        }

        if (menuStatus == menuStatusOpenHomeMenuApps) {
            showHomeMenuTitleSelect();
        }
        else if (menuStatus == menuStatusOpenTitleFiltering) {
            showFilterTitleSelect();
        }

		if (nextSdCheck < osGetTime()) {
			regionFreeUpdate();

			FSUSER_IsSdmcDetected(&sdmcCurrent);

			if(sdmcCurrent == 1 && (sdmcPrevious == 0 || sdmcPrevious < 0))
			{
				closeSDArchive();
				openSDArchive();
				scanHomebrewDirectory(&menu, currentFolder());
			}
			else if(sdmcCurrent < 1 && sdmcPrevious == 1)
			{
				clearMenuEntries(&menu);
			}
			sdmcPrevious = sdmcCurrent;
			nextSdCheck = osGetTime()+250;
		}

		ACU_GetWifiStatus(NULL, &wifiStatus);
		PTMU_GetBatteryLevel(NULL, &batteryLevel);
		PTMU_GetBatteryChargeState(NULL, &charging);
		hidScanInput();

		updateBackground();

		menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);

        if (me) {
            debugValues[50] = me->descriptor.numTargetTitles;
            debugValues[51] = me->descriptor.selectTargetProcess;
            if(me->descriptor.numTargetTitles)
            {
                debugValues[58] = (me->descriptor.targetTitles[0].tid >> 32) & 0xFFFFFFFF;
                debugValues[59] = me->descriptor.targetTitles[0].tid & 0xFFFFFFFF;
            }
        }

		if(hbmenu_state == HBMENU_NETLOADER_ACTIVE){
			if(hidKeysDown()&KEY_B){
				netloader_deactivate();
				hbmenu_state = HBMENU_DEFAULT;
			}else{
				int rc = netloader_loop();
				if(rc > 0)
				{
					netloader_boot = true;
					break;
				}else if(rc < 0){
					hbmenu_state = HBMENU_NETLOADER_ERROR;
				}
			}
		}else if(hbmenu_state == HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}else if(hidKeysDown()&KEY_A){
				if(isNinjhax2())
				{
					// basically just relaunch boot.3dsx w/ scanning in hopes of getting netloader capabilities
					static char hbmenuPath[] = "/boot.3dsx";
					netloadedPath = hbmenuPath; // fine since it's static
					netloader_boot = true;
					break;
				}
			}
		}else if(hbmenu_state == HBMENU_REGIONFREE){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}else if(hidKeysDown()&KEY_A && regionFreeGamecardIn)
			{
				// region free menu entry is selected so we can just break out like updateMenu() normally would
				break;
			}
		}else if(hbmenu_state == HBMENU_TITLETARGET_ERROR){
			if(hidKeysDown()&KEY_B){
				hbmenu_state = HBMENU_DEFAULT;
			}
		}else if(hbmenu_state == HBMENU_TITLESELECT){
			if(hidKeysDown()&KEY_A && titleBrowser.selected)
			{
                launchSVDTFromTitleMenu();
			}
            else if(hidKeysDown()&KEY_B) {
                closeTitleBrowser();
            }

		}else if(hbmenu_state == HBMENU_NETLOADER_ERROR){
			if(hidKeysDown()&KEY_B)
				hbmenu_state = HBMENU_DEFAULT;
        }else if (!showRebootMenu) {
            if (hidKeysDown()&KEY_X) {
                takeScreenshot();
            }

            if(hidKeysDown()&KEY_START) {
                alertSelectedButton = 0;
                showRebootMenu = true;
            }
			if(hidKeysDown()&KEY_Y)
			{
				if(netloader_activate() == 0) hbmenu_state = HBMENU_NETLOADER_ACTIVE;
				else if(isNinjhax2()) hbmenu_state = HBMENU_NETLOADER_UNAVAILABLE_NINJHAX2;
			}

            if (menuStatus == menuStatusHomeMenuApps) {
                if (updateGrid(&titleMenu)) {
                    launchTitleFromMenu(&titleMenu);
                }
            }
            else if (menuStatus == menuStatusTitleFiltering) {
                if (updateGrid(&titleMenu)) {
                    menuEntry_s* me = getMenuEntry(&titleMenu, titleMenu.selectedEntry);
                    toggleTitleFilter(me, &titleMenu);
                }
            }
            else if (menuStatus == menuStatusFolders) {
                if (updateGrid(&foldersMenu)) {
                    menuEntry_s* me = getMenuEntry(&foldersMenu, foldersMenu.selectedEntry);

                    if (me->showTick == NULL) {
                        setFolder(me->name);
                    }
                }
            }

            else if (menuStatus == menuStatusSettings) {
                if (updateGrid(&settingsMenu)) {
                    handleSettingsMenuSelection(&settingsMenu);

                    if (menuStatus == menuStatusSoftwareUpdate) {
                        break;
                    }
                }
            }
            else if (menuStatus == menuStatusGridSettings) {
                if (updateGrid(&gridSettingsMenu)) {
                    handleSettingsMenuSelection(&gridSettingsMenu);
                }
            }
            else if (menuStatus == menuStatusWaterSettings) {
                if (updateGrid(&waterMenu)) {
                    handleSettingsMenuSelection(&waterMenu);
                }
            }
            else if (menuStatus == menuStatusThemeSettings) {
                if (updateGrid(&themeSettingsMenu)) {
                    handleSettingsMenuSelection(&themeSettingsMenu);
                }
            }
            else if (menuStatus == menuStatusThemeSelect) {
                if (updateGrid(&themesMenu)) {
                    if (themesMenu.selectedEntry == 0) {
                        if (!randomTheme) {
                            randomTheme = true;
                            updateMenuTicks(&themesMenu, NULL, true);
                        }
                    }
                    else {
                        menuEntry_s* me = getMenuEntry(&themesMenu, themesMenu.selectedEntry);

                        if (me->showTick == NULL) {
                            randomTheme = false;
                            setTheme(me->executablePath);
                            char * currentThemeName = getConfigStringForKey("currentTheme", "Default", configTypeMain);
                            updateMenuTicks(&themesMenu, currentThemeName, true);
                        }
                    }

                }
            }
            else if (menuStatus == menuStatusHelp) {
                updateHelp();
            }
            else if (menuStatus == menuStatusColourAdjust || menuStatus == menuStatusPanelSettings || menuStatus == menuStatusTranslucencyTop || menuStatus == menuStatusTranslucencyBottom) {
                handleNonGridToolbarNavigation();
            }

            else if (menuStatus == menuStatusColourSettings) {
                if (updateGrid(&colourSelectMenu)) {
                    handleColourSelectMenuSelection();
                }
            }

            else if(updateMenu(&menu))
            {
                menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);
                if(me && !strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot)
                {
                    regionFreeUpdate();

                    if (regionFreeGamecardIn) {
                        break;
                    }
                }else
                {


                    // if appropriate, look for specified titles in list
                    if(me->descriptor.numTargetTitles)
                    {
                        // first refresh list (for sd/gamecard)
//                        updateTitleBrowser(&titleBrowser);

                        // go through target title list in order so that first ones on list have priority
                        int i;
                        titleInfo_s* ret = NULL;
                        for(i=0; i<me->descriptor.numTargetTitles; i++)
                        {
                            ret = findTitleBrowser(&titleBrowser, me->descriptor.targetTitles[i].mediatype, me->descriptor.targetTitles[i].tid);
                            if(ret)break;
                        }

                        if(ret)
                        {
                            targetProcessId = -2;
                            target_title = *ret;
                            break;
                        }

                        // if we get here, we aint found shit
                        // if appropriate, let user select target title
                        if(me->descriptor.selectTargetProcess) hbmenu_state = HBMENU_TITLESELECT;
                        else hbmenu_state = HBMENU_TITLETARGET_ERROR;
                    }

                    else
                    {
                        if(me->descriptor.selectTargetProcess) {
                            showSVDTTitleSelect();
                        }
                        else {
                            break;
                        }
                    }


                }
            }
		}


        renderFrame();
        gfxFlip();

        endMs = osGetTime();
        delayMs = frameMs - (endMs - startMs);
        delayNs = delayMs * 1000000;
        svcSleepThread(delayNs);
	}

    if (dieImmediately) {
        return 0;
    }

    menuEntry_s* me;

	if(netloader_boot)
	{
		me = malloc(sizeof(menuEntry_s));
		initMenuEntry(me, netloadedPath, "netloaded app", "", "", NULL);
	}
    else if (menuStatus == menuStatusSoftwareUpdate) {
        me = malloc(sizeof(menuEntry_s));
        initMenuEntry(me, "/gridlauncher/update/mglupdate.3dsx", "updater", "", "", NULL);
    }
    else {
        me = getMenuEntry(&menu, menu.selectedEntry);
    }

	scanMenuEntry(me);

    if (touchThreadNeedsRelease) {
        releaseTouchThread();
    }

    if (titlemenuIsUpdating) {
        //Stop the title menu loading process, causing the thread to exit
        cancelTitleLoading();

        //Wait a little bit (two seconds) longer to allow the thread to actually terminate
        svcSleepThread(2000000000ULL);
    }

    if (titleThreadNeedsRelease) {
        releaseTitleThread();
    }

    exitServices();

	if(!strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot) {
        return regionFreeRun();
//        return regionFreeRun2(target_title.title_id & 0xffffffff, (target_title.title_id >> 32) & 0xffffffff, target_title.mediatype, 0x1);
    }

	regionFreeExit();

	return bootApp(me->executablePath, &me->descriptor.executableMetadata, me->arg);
}
