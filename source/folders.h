#pragma once
#include <3ds.h>
//#include "button.h"
#include "menu.h"

extern menu_s foldersMenu;

//buttonList folderButtons;

char * currentFolder();
char * currentFolderName();

void buildFoldersList();
void setFolder(char * folderName);
//void drawFoldersList();
//void checkFolderTouches();