#pragma once
#include <3ds.h>
#include "button.h"

buttonList folderButtons;

char * currentFolder();
char * currentFolderName();

void buildFoldersList();
void drawFoldersList();
//void checkFolderTouches();