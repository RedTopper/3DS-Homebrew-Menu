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

#include "wallpaper.h"

#include "MAGFX.h"

#include "help.h"
#include "touchblock.h"
#include "folders.h"

u8 sdmcCurrent = 0;
u64 nextSdCheck = 0;

u32 wifiStatus = 0;
u8 batteryLevel = 5;
u8 charging = 0;

bool die = false;
bool showRebootMenu = false;
bool startRebootProcess = false;

Handle threadHandle, threadRequest;
#define STACKSIZE (4 * 1024)

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

void drawDebug()
{
	char str[256];
	sprintf(str, "hello3 %08X %d %d %d %d %d %d %d\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n%08X %08X %08X %08X\n\n", debugValues[50], debugValues[51], debugValues[52], debugValues[53], debugValues[54], debugValues[55], debugValues[56], debugValues[57], debugValues[58], debugValues[59], debugValues[60], debugValues[61], debugValues[62], debugValues[63], debugValues[64], debugValues[65], debugValues[66], debugValues[67], debugValues[68], debugValues[69]);
    
    rgbColour * dark = darkTextColour();
    
    MADrawText(GFX_TOP, GFX_LEFT, 48, 100, str, &MAFontRobotoRegular8, dark->r, dark->g, dark->b);
}

extern void closeReboot() {
    showRebootMenu = false;
}

extern void doReboot() {
    aptOpenSession();
    APT_HardwareResetAsync(NULL);
    aptCloseSession();
}

void logBoot(u64 title_id) {
    char titleIDString[128];
    sprintf(titleIDString, "Booting title ID: %llu", title_id);
    
    FILE* fSave = fopen( "sdmc:/bootlog.txt", "w" );
    if (fSave != NULL) {
        fputs(titleIDString, fSave);
    }
    fclose(fSave);
    
    //for some reason I get a linking error if I call this here
//    logTextP(titleIDString, "sdmc:/bootlog.txt");
}

void launchSVDTFromTitleMenu() {
    /*
     THANK YOU to Suloku from GBATemp for the solution
     to passing the selected title to the save manager
     (writing the title_id to tid.bin on the SD card)
     */
    
    menuEntry_s* me = getMenuEntry(&titleMenu, titleMenu.selectedEntry);
    
    if (me) {
        if (me->title_id) {
            if (me->title_id > 0) {
                
                titleInfo_s* ret = NULL;
                ret = getTitleWithID(&titleBrowser, me->title_id);
                
                if (ret) {
                    FILE * pFile;
                    mkdir("sdmc:/svdt", S_IRWXO);
                    pFile = fopen ("sdmc:/svdt/tid.bin", "wb");
                    if (pFile != NULL){
                        logBoot(me->title_id);
                        
                        targetProcessId = -2;
                        fwrite (&me->title_id , sizeof(char), sizeof(me->title_id), pFile);
                        target_title = *ret;
                        die = true;
                    }
                    else {
                        //Could not write the tid.bin file. Show an error here
                        logText("Could not write tid.bin");
                    }
                    fclose (pFile);
                }
                else {
                    //Could not find title. Show an error here
                    logText("Could not match title");
                }
            }
            else {
                logText("Invalid title ID for menu entry");
            }
        }
        else {
            //No title ID found. Show an error here
            logText("No title ID for menu entry found");
        }
    }
    else {
        //Menu entry for title not found. Show an error here.
        logText("menuEntry for title not found");
    }
}

void exitServices() {
    // cleanup whatever we have to cleanup
    netloader_exit();
    titlesExit();
    ptmExit();
    acExit();
    irrstExit();
    hidExit();
    gfxExit();
    exitFilesystem();
    closeSDArchive();
    aptExit();
    srvExit();
}

void launchTitleFromTitleMenu() {
    menuEntry_s* me = getMenuEntry(&titleMenu, titleMenu.selectedEntry);
    
    if (me) {
        if (me->title_id) {
            if (me->title_id > 0) {
                
                
                
                titleInfo_s* ret = NULL;
                ret = getTitleWithID(&titleBrowser, me->title_id);
                
                if (ret) {
                    logBoot(me->title_id);
                    exitServices();
                    regionFreeRun2(ret->title_id & 0xffffffff, (ret->title_id >> 32) & 0xffffffff, ret->mediatype, 0x1);
                    
                    /*
                    
                    FILE * pFile;
                    mkdir("sdmc:/svdt", S_IRWXO);
                    pFile = fopen ("sdmc:/svdt/tid.bin", "wb");
                    if (pFile != NULL){
                        char titleIDString[128];
                        sprintf(titleIDString, "Booting title ID: %llu", me->title_id);
                        logText(titleIDString);
                        
                        targetProcessId = -2;
                        fwrite (&me->title_id , sizeof(char), sizeof(me->title_id), pFile);
                        target_title = *ret;
                        die = true;
                    }
                    else {
                        //Could not write the tid.bin file. Show an error here
                        logText("Could not write tid.bin");
                    }
                    fclose (pFile);
                     
                     */
                }
                else {
                    //Could not find title. Show an error here
                    logText("Could not match title");
                }
            }
            else {
                logText("Invalid title ID for menu entry");
            }
        }
        else {
            //No title ID found. Show an error here
            logText("No title ID for menu entry found");
        }
    }
    else {
        //Menu entry for title not found. Show an error here.
        logText("menuEntry for title not found");
    }
}

void putTitleMenu(char * barTitle) {
    drawGrid(&titleMenu);
    drawBottomStatusBar(barTitle);
}

void renderFrame()
{
	// background stuff
    
    rgbColour * bgc = backgroundColour();
    
    gfxFillColor(GFX_BOTTOM, GFX_LEFT, (u8[]){bgc->r, bgc->g, bgc->b});
    gfxFillColor(GFX_TOP, GFX_LEFT, (u8[]){bgc->r, bgc->g, bgc->b});
    
    //Wallpaper
    drawWallpaper();
    
	// debug text
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
            
//            strcpy(buttonTitles[0], "Reboot");
            strcpy(buttonTitles[1], "Back");
            
            int alertResult = drawAlert("Reboot", "You're about to reboot your console into the Home Menu.", NULL, 2, buttonTitles);
            
            if (startRebootProcess) {
                doReboot();
            }
            else {
                if (alertResult == 0) {
                    startRebootProcess = true;
                }
                else if (alertResult == 1 || alertResult == alertButtonKeyB) {
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
                drawMenuEntry(&gamecardMenuEntry, GFX_BOTTOM, true, &menu);
                
                drawAlert("Region free launcher", "The region free launcher is ready to run your out-of-region gamecard !\n\nA : Play\nB : Cancel", NULL, 0, NULL);
            }else{
                drawAlert("Region free launcher", "The region free loader cannot detect a gamecard in the slot.\nPlease insert a gamecard in your console before continuing.\n\nB : Cancel\n", NULL, 0, NULL);
            }
        }else if(hbmenu_state == HBMENU_TITLESELECT){
            
            if (updateGrid(&titleMenu)) {
                launchSVDTFromTitleMenu();
            }
            else {
                putTitleMenu("Select Title to Manage Saves");
            }
        }else if(hbmenu_state == HBMENU_TITLETARGET_ERROR){
            drawAlert("Missing target title", "The application you are trying to run requested a specific target title.\nPlease make sure you have that title !\n\nB : Back\n", NULL, 0, NULL);
        }else if(hbmenu_state == HBMENU_NETLOADER_ERROR){
            netloader_draw_error();
        }else{
            //got SD
            
            if (menuStatus == menuStatusHomeMenuApps) {
                putTitleMenu("Select Title to Launch");
            }
            else if (menuStatus == menuStatusFolders) {
                drawGrid(&foldersMenu);
                drawBottomStatusBar("Select folder");
            }
            else if (menuStatus == menuStatusSettings) {
                drawGrid(&settingsMenu);
                drawBottomStatusBar("Settings");
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
                drawBottomStatusBar("Panel translucency");
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

void showTitleMenu(titleBrowser_s * aTitleBrowser, menu_s * aTitleMenu, int newMenuStatus) {
    if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {
        updateTitleMenu(&titleBrowser, &titleMenu, "Loading titles");
        titleMenuInitialLoadDone = true;
    }
    
    updateMenuIconPositions(aTitleMenu);
    gotoFirstIcon(aTitleMenu);
    
    setMenuStatus(newMenuStatus);
}

void showSVDTTitleSelect() {
    if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {
        updateTitleMenu(&titleBrowser, &titleMenu, "Loading titles");
        titleMenuInitialLoadDone = true;
    }
    
    showTitleMenu(&titleBrowser, &titleMenu, menuStatusTitleBrowser);
    hbmenu_state = HBMENU_TITLESELECT;
}

void showHomeMenuTitleSelect() {
    showTitleMenu(&titleBrowser, &titleMenu, menuStatusHomeMenuApps);
}

void closeTitleBrowser() {
    setMenuStatus(menuStatusIcons);
//    updateMenuIconPositions(&menu);
    checkReturnToGrid(&menu);
    hbmenu_state = HBMENU_DEFAULT;
}

bool gamecardWasIn;
bool gamecardStatusChanged;

void threadMain(void *arg) {
    
    while(1) {
        svcWaitSynchronization(threadRequest, U64_MAX);
        svcClearEvent(threadRequest);
        
        updateTitleMenu(&titleBrowser, &titleMenu, NULL);
        
        titleMenuInitialLoadDone = true;
        
        svcExitThread();
    }
}

int main()
{
	srvInit();
	aptInit();
	gfxInitDefault();
	initFilesystem();
	openSDArchive();
	hidInit(NULL);
	irrstInit(NULL);
	acInit();
	ptmInit();
	titlesInit();
	regionFreeInit();
	netloader_init();

    mkdir(rootPath, 777);
    mkdir(themesPath, 777);
    mkdir(foldersPath, 777);
    mkdir(defaultThemePath, 777);
    mkdir("/gridlauncher/screenshots/", 777);

	// offset potential issues caused by homebrew that just ran
	aptOpenSession();
	APT_SetAppCpuTimeLimit(NULL, 0);
	aptCloseSession();

	initBackground();
//	initErrors();
	initMenu(&menu);
	initTitleBrowser(&titleBrowser, NULL);

	u8 sdmcPrevious = 0;
	FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);
	if(sdmcCurrent == 1)
	{
		scanHomebrewDirectory(&menu, currentFolder());
	}

	sdmcPrevious = sdmcCurrent;
	nextSdCheck = osGetTime()+250;
	srand(svcGetSystemTick());
    
    gamecardWasIn = regionFreeGamecardIn;
    
    initWallpaper();

    u32 *threadStack;
    
    preloadTitles = getConfigBoolForKey("preloadTitles", true, configTypeMain);
    
    if (preloadTitles) {
        svcCreateEvent(&threadRequest,0);
        threadStack = memalign(32, STACKSIZE);
        Result ret = svcCreateThread(&threadHandle, threadMain, 0, &threadStack[STACKSIZE/4], 0x3f, 0);
    }
    
	while(aptMainLoop()) {
        if (die) {
            break;
        }
        
        if (preloadTitles && !titleMenuInitialLoadDone && !titlemenuIsUpdating) {
            svcSignalEvent(threadRequest);
        }
        
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
        
		if (nextSdCheck < osGetTime()) {
			regionFreeUpdate();

			FSUSER_IsSdmcDetected(NULL, &sdmcCurrent);

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
                
////                logText("SVDT Exit B");
//				targetProcessId = -2;
//				target_title = *titleBrowser.selected;
//                /* "A very bad way to pass tid to svdt"
//                        By Suloku @ GBATemp - thank you Soluku!
//                 */
//                menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);
//                if(strcmp(me->name, "svdt") == 0){
//                    FILE * pFile;
//                    mkdir("sdmc:/svdt", S_IRWXO);
//                    pFile = fopen ("sdmc:/svdt/tid.bin", "wb");
//                    if (pFile != NULL){
//                        fwrite (&target_title.title_id , sizeof(char), sizeof(target_title.title_id), pFile);
//                    }
//                    fclose (pFile);
//                }
//				break;
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
                    launchTitleFromTitleMenu();
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
                    handleSettingsMenuSelection();
                }
            }
            else if (menuStatus == menuStatusThemeSelect) {
                if (updateGrid(&themesMenu)) {
                    menuEntry_s* me = getMenuEntry(&themesMenu, themesMenu.selectedEntry);
                    
                    if (me->showTick == NULL) {
                        setTheme(me->name);
                        char * currentThemeName = getConfigStringForKey("currentTheme", "Default", configTypeMain);
                        updateMenuTicks(&themesMenu, currentThemeName);
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
                        // go through target title list in order so that first ones on list have priority
                        int i;
                        titleInfo_s* ret = NULL;
                        for(i=0; i<me->descriptor.numTargetTitles; i++)
                        {
                            launchSVDTFromTitleMenu();
                        }
                        
                        if(ret)
                        {
                            targetProcessId = -2;
                            target_title = *ret;
                            break;
                        }
                        
                        // if we get here, we aint found shit
                        // if appropriate, let user select target title
                        if(me->descriptor.selectTargetProcess){
                            showSVDTTitleSelect();
                        }
                        else {
                            /*
                             XML titles
                             */
                            //If the title menu has not been loaded yet
                            if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {
                                //Force an updatae to the title menu and then break out of the main loop to boot the title
                                updateTitleMenu(&titleBrowser, &titleMenu, "Preparing title");
                                titleMenuInitialLoadDone = true;
                                menuForceReturnTrue = true;
//                                break;
                            }
                            //The title menu has been populated but there is still no matching title
                            //Show an error
                            else {
                                hbmenu_state = HBMENU_TITLETARGET_ERROR;
                            }
                        }
                    }else
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
	}
    
	menuEntry_s* me = getMenuEntry(&menu, menu.selectedEntry);

	if(netloader_boot)
	{
		me = malloc(sizeof(menuEntry_s));
		initMenuEntry(me, netloadedPath, "netloaded app", "", "", NULL);
	}
    
	scanMenuEntry(me);
    
    if (preloadTitles) {
        svcCloseHandle(threadRequest);
        svcCloseHandle(threadHandle);
        free(threadStack);
    }
    
    if (touchThreadNeedsRelease) {
        releaseTouchThread();
    }

    exitServices();
    
	if(!strcmp(me->executablePath, REGIONFREE_PATH) && regionFreeAvailable && !netloader_boot) {
        return regionFreeRun();
    }
    
	regionFreeExit();
    
	return bootApp(me->executablePath, &me->descriptor.executableMetadata);
}
