#include "folders.h"
#include "filesystem.h"
#include <string.h>
#include "utils.h"
#include "config.h"
#include "stdlib.h"
#include "smdh.h"
#include "defaultFolder_bin.h"
#include "MAGFX.h"

//buttonList folderButtons;
menu_s foldersMenu;

//button buttons[1024][sizeof(button)];
//int buttonCount = 0;

char * currentFolder() {
    return getConfigStringForKey("currentfolder", "/3ds/", configTypeMain);
}

char * currentFolderName() {
    char * cf = currentFolder();
    
    if (strcmp(cf, "/3ds/") == 0) {
        char * folderName = malloc(strlen("3ds"));
        strcpy(folderName, "3ds");
        return folderName;
    }
    else {
        int foldersPathLen = strlen(foldersPath);
        int len = strlen(cf)-foldersPathLen;
        char * folderName = malloc(len);
        strncpy(folderName, cf+foldersPathLen, len-1);
        memset(&folderName[len-1], 0, 1);
        return folderName;
    }
}

#include "logText.h"

char * folderPathForFolderName(char * folderName) {
    if (strcmp(folderName, "3ds") == 0) {
        char * folderPath = malloc(strlen("/3ds/"));
        strcpy(folderPath, "/3ds/");
        return folderPath;
    }
    
    char *folderPath = malloc(strlen(foldersPath) + strlen(folderName) + 2);
    strcpy(folderPath, foldersPath);
    strcat(folderPath, folderName);
    
    strcat(folderPath, "/");
    
    return folderPath;
}

void setFolder(char * folderName) {
    char * folderPath = folderPathForFolderName(folderName);

    setConfigString("currentfolder", folderPath, configTypeMain);
    saveConfigWithType(configTypeMain);
    setMenuStatus(menuStatusFolderChanged);
    
    free(folderPath);
    
    if (animatedGrids) {
        startTransition(transitionDirectionUp, foldersMenu.pagePosition, &foldersMenu);
    }
}

void addFolderToList(char * fullPath, menuEntry_s * me, char * smdhName, int folderPathLen) {
    me->hidden = false;
    me->isTitleEntry = false;
    me->isRegionFreeEntry = false;
    me->showTick = NULL;
    
//    logText(fullPath);
    
    char smdhPath[strlen(fullPath) + strlen(smdhName) + 1];
    strcpy(smdhPath, fullPath);
    strcat(smdhPath, smdhName);
        
    bool iconNeedsToBeGenerated = true;
    
    if(fileExists(smdhPath, &sdmcArchive)) {
        static smdh_s tmpSmdh;
        int ret = loadFile(smdhPath, &tmpSmdh, &sdmcArchive, sizeof(smdh_s));
        
        if (!ret) {
            ret = extractSmdhData(&tmpSmdh, me->name, me->description, me->author, me->iconData);
            if (!ret) {
                iconNeedsToBeGenerated = false;
            }
        }
    }
    
    if (iconNeedsToBeGenerated) {
        memcpy(me->iconData, defaultFolder_bin, 48*48*3);
    }
    
    if (strcmp(fullPath, "/3ds") == 0) {
        strcpy(me->name, "3ds");
    }
    else {
        strcpy(me->name, fullPath+folderPathLen);
    }
    
    strcpy(me->description, "");
    strcpy(me->author, "");
    
    me->drawFirstLetterOfName = iconNeedsToBeGenerated;
    
    addMenuEntryCopy(&foldersMenu, me);
//    foldersMenu.numEntries = foldersMenu.numEntries + 1;
}

bool foldersLoaded = false;

void buildFoldersList() {
    if (foldersLoaded) {
        return;
    }
    
    foldersLoaded = true;
    
    directoryContents * contents = contentsOfDirectoryAtPath(foldersPath, true);
    
    foldersMenu.entries=NULL;
    foldersMenu.numEntries=0;
    foldersMenu.selectedEntry=0;
    foldersMenu.scrollLocation=0;
    foldersMenu.scrollVelocity=0;
    foldersMenu.scrollBarSize=0;
    foldersMenu.scrollBarPos=0;
    foldersMenu.scrollTarget=0;
    foldersMenu.atEquilibrium=false;
    
    char * smdhName = "/folder.smdh";
    int folderPathLen = strlen(foldersPath);
    
    menuEntry_s rootEntry;
    addFolderToList("/3ds", &rootEntry, smdhName, folderPathLen);
    
    int i;
    for (i=0; i<contents->numPaths; i++) {
        char * fullPath = contents->paths[i];
        static menuEntry_s me;
        addFolderToList(fullPath, &me, smdhName, folderPathLen);
    }
    
    updateMenuIconPositions(&foldersMenu);
    
    free(contents);
}

