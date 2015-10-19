#include "folders.h"
#include "filesystem.h"
#include <string.h>
#include "utils.h"
#include "config.h"
#include "stdlib.h"

buttonList folderButtons;

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
//        char * folderName2 = malloc(strlen("foo"));
//        strcpy(folderName2, "foo");
//        return folderName2;
//        
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

void drawFoldersList() {
    char * currentFolder = getConfigStringForKey("currentfolder", "/3ds/", configTypeMain);
    
    int i;
    for (i=0; i<folderButtons.numButtons; i++) {
        button * aButton = folderButtons.buttons[i];
        char *folderPath = folderPathForFolderName(aButton->longText);
        if (strcmp(folderPath, currentFolder) == 0) {
            aButton->selected = true;
            aButton->enabled = false;
        }
        else {
            aButton->selected = false;
            aButton->enabled = true;
        }
    }
    
    btnDrawButtonList(&folderButtons);
}

void setFolder(char * folderName) {
    char * folderPath = folderPathForFolderName(folderName);

//    char * currentFolder = getConfigStringForKey("currentfolder", "/3ds/", configTypeMain);

//    if (strcmp(folderPath, currentFolder) != 0) {
        setConfigString("currentfolder", folderPath, configTypeMain);
        saveConfigWithType(configTypeMain);
        setMenuStatus(menuStatusFolderChanged);
//    }
    
    free(folderPath);
}

bool foldersLoaded = false;

void buildFoldersList() {
    if (foldersLoaded) {
        return;
    }
    
    foldersLoaded = true;
    
    int x = -1;
    int y = -1;
    
    button rootButton;
    btnConfigureButtonForGrid(&rootButton, &x, &y, "", "", "3ds");
    rootButton.callback = &setFolder;
    rootButton.callbackObject1 = "3ds";
    btnAddButtonToButtonList(&rootButton, &folderButtons);
    
    directoryContents * contents = contentsOfDirectoryAtPath(foldersPath, true);
    
    int i;
    for (i=0; i<contents->numPaths; i++) {
        char * fullPath = contents->paths[i];
        button * pathButton = malloc(sizeof(button));
        int folderPathLen = strlen(foldersPath);
        btnConfigureButtonForGrid(pathButton, &x, &y, "", "", fullPath+folderPathLen);
        pathButton->callback = &setFolder;
        char * filenameCopy = malloc(1024);
        strcpy(filenameCopy, fullPath+folderPathLen);
        pathButton->callbackObject1 = filenameCopy;
        btnAddButtonToButtonList(pathButton, &folderButtons);
        free(pathButton);
    }
    
    free(contents);
}

