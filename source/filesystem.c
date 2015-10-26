#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include "stdlib.h"

#include "installerIcon_bin.h"

#include "filesystem.h"
#include "smdh.h"
#include "utils.h"

#include "addmenuentry.h"
#include "config.h"

FS_archive sdmcArchive;

void initFilesystem(void)
{
	fsInit();
	sdmcInit();
}

void exitFilesystem(void)
{
	sdmcExit(); //Crashes here
	fsExit();
}

void openSDArchive()
{
	sdmcArchive=(FS_archive){0x00000009, (FS_path){PATH_EMPTY, 1, (u8*)""}};
	FSUSER_OpenArchive(NULL, &sdmcArchive);
}

void closeSDArchive()
{
	FSUSER_CloseArchive(NULL, &sdmcArchive);
}

int loadFile(char* path, void* dst, FS_archive* archive, u64 maxSize)
{
	if(!path || !dst || !archive)return -1;

	u64 size;
	u32 bytesRead;
	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(NULL, &fileHandle, *archive, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret!=0)return ret;

	ret=FSFILE_GetSize(fileHandle, &size);
	if(ret!=0)goto loadFileExit;
	if(size>maxSize){ret=-2; goto loadFileExit;}

	ret=FSFILE_Read(fileHandle, &bytesRead, 0x0, dst, size);
	if(ret!=0)goto loadFileExit;
	if(bytesRead<size){ret=-3; goto loadFileExit;}

	loadFileExit:
	FSFILE_Close(fileHandle);
	return ret;
}

bool fileExists(char* path, FS_archive* archive)
{
	if(!path || !archive)return false;

	Result ret;
	Handle fileHandle;

	ret=FSUSER_OpenFile(NULL, &fileHandle, *archive, FS_makePath(PATH_CHAR, path), FS_OPEN_READ, FS_ATTRIBUTE_NONE);
	if(ret!=0)return false;

	ret=FSFILE_Close(fileHandle);
	if(ret!=0)return false;

	return true;
}

extern int debugValues[100];

void addFileToMenu(menu_s* m, char* execPath)
{
	if(!m || !execPath)return;

	static menuEntry_s tmpEntry;

	if(!fileExists(execPath, &sdmcArchive))return;

	int i, l=-1; for(i=0; execPath[i]; i++) if(execPath[i]=='/')l=i;

	initMenuEntry(&tmpEntry, execPath, &execPath[l+1], execPath, "", (u8*)installerIcon_bin);

	static char xmlPath[128];
	snprintf(xmlPath, 128, "%s", execPath);
	l = strlen(xmlPath);
	xmlPath[l-1] = 0;
	xmlPath[l-2] = 'l';
	xmlPath[l-3] = 'm';
	xmlPath[l-4] = 'x';
	if(fileExists(xmlPath, &sdmcArchive)) loadDescriptor(&tmpEntry.descriptor, xmlPath);

	addMenuEntryCopy(m, &tmpEntry);
}

void addDirectoryToMenu(menu_s* m, char* path)
{
	if(!m || !path)return;

	static menuEntry_s tmpEntry;
	static smdh_s tmpSmdh;
	static char execPath[128];
	static char iconPath[128];
	static char xmlPath[128];

	int i, l=-1; for(i=0; path[i]; i++) if(path[i]=='/') l=i;

	snprintf(execPath, 128, "%s/boot.3dsx", path);
	if(!fileExists(execPath, &sdmcArchive))
	{
		snprintf(execPath, 128, "%s/%s.3dsx", path, &path[l+1]);
		if(!fileExists(execPath, &sdmcArchive))return;
	}

	bool icon=false;
	snprintf(iconPath, 128, "%s/icon.bin", path);
	if(!icon && !(icon=fileExists(iconPath, &sdmcArchive)))snprintf(iconPath, 128, "%s/icon.smdh", path);
	if(!icon && !(icon=fileExists(iconPath, &sdmcArchive)))snprintf(iconPath, 128, "%s/icon.icn", path);
	if(!icon && !(icon=fileExists(iconPath, &sdmcArchive)))snprintf(iconPath, 128, "%s/%s.smdh", path, &path[l+1]);
	if(!icon && !(icon=fileExists(iconPath, &sdmcArchive)))snprintf(iconPath, 128, "%s/%s.icn", path, &path[l+1]);

	int ret=loadFile(iconPath, &tmpSmdh, &sdmcArchive, sizeof(smdh_s));
	
	if(!ret)
	{
		initEmptyMenuEntry(&tmpEntry);
		ret=extractSmdhData(&tmpSmdh, tmpEntry.name, tmpEntry.description, tmpEntry.author, tmpEntry.iconData);
		strncpy(tmpEntry.executablePath, execPath, ENTRY_PATHLENGTH);
	}

	if(ret)initMenuEntry(&tmpEntry, execPath, &path[l+1], execPath, "", (u8*)installerIcon_bin);

	snprintf(xmlPath, 128, "%s/descriptor.xml", path);
	if(!fileExists(xmlPath, &sdmcArchive))snprintf(xmlPath, 128, "%s/%s.xml", path, &path[l+1]);
	loadDescriptor(&tmpEntry.descriptor, xmlPath);


	addMenuEntryCopy(m, &tmpEntry);
}

void scanHomebrewDirectory(menu_s* m, char* path) {
    if(!path)return;
    
    Handle dirHandle;
    FS_path dirPath=FS_makePath(PATH_CHAR, path);
    FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
    
    static char fullPath[1024][1024];
    u32 entriesRead;
    int totalentries = 0;
    do
    {
        static FS_dirent entry;
        memset(&entry,0,sizeof(FS_dirent));
        entriesRead=0;
        FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
        if(entriesRead)
        {
            strncpy(fullPath[totalentries], path, 1024);
            int n=strlen(fullPath[totalentries]);
            unicodeToChar(&fullPath[totalentries][n], entry.name, 1024-n);
            if(entry.isDirectory) //directories
            {
                //addDirectoryToMenu(m, fullPath[totalentries]);
                totalentries++;
            }else{ //stray executables
                n=strlen(fullPath[totalentries]);
                if(n>5 && !strcmp(".3dsx", &fullPath[totalentries][n-5])){
                    //addFileToMenu(m, fullPath[totalentries]);
                    totalentries++;
                }
            }
        }
    }while(entriesRead);
    
    FSDIR_Close(dirHandle);
    
    bool sortAlpha = getConfigBoolForKey("sortAlpha", false, configTypeMain);
    addMenuEntries(fullPath, totalentries, strlen(path), m, sortAlpha);
    
    updateMenuIconPositions(m);
}

char * currentThemePath() {
    char * currentThemeName = getConfigStringForKey("currentTheme", "Default", configTypeMain);
    int len = strlen(themesPath) + strlen(currentThemeName) + 2;
    char * path = malloc(len);
    sprintf(path, "%s%s/", themesPath, currentThemeName);
    return path;
}

int compareStrings(const void *stringA, const void *stringB) {
    const char *a = *(const char**)stringA;
    const char *b = *(const char**)stringB;
    return strcmp(a, b);
}

directoryContents * contentsOfDirectoryAtPath(char * path, bool dirsOnly) {
    directoryContents * contents = malloc(sizeof(directoryContents));
    
    int numPaths = 0;
    
    Handle dirHandle;
    FS_path dirPath=FS_makePath(PATH_CHAR, path);
    FSUSER_OpenDirectory(NULL, &dirHandle, sdmcArchive, dirPath);
    
    u32 entriesRead;
    do
    {
        static FS_dirent entry;
        memset(&entry,0,sizeof(FS_dirent));
        entriesRead=0;
        FSDIR_Read(dirHandle, &entriesRead, 1, &entry);
        if(entriesRead) {
            if(!dirsOnly || (dirsOnly && entry.isDirectory)) {
                char fullPath[1024];
                strncpy(fullPath, path, 1024);
                int n=strlen(path);
                unicodeToChar(&fullPath[n], entry.name, 1024-n);

                strcpy(contents->paths[numPaths], fullPath);
                numPaths++;
            }
        }
    }while(entriesRead);
    
    FSDIR_Close(dirHandle);
    
//    qsort(contents->paths, numPaths, 1024, compareStrings);
    
    contents->numPaths = numPaths;
    return contents;
}





