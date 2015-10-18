#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <3ds.h>

#include "titles.h"
#include "smdh.h"
#include "menu.h"
#include "filesystem.h"
#include "MAGFX.h"

extern int debugValues[100];

menu_s titleMenu;
titleBrowser_s titleBrowser;

#define maxIgnoreTitleIDs 100
bool ignoreTitleIDsLoaded = false;
char ignoreTitleIDs[maxIgnoreTitleIDs][17];
int numIgnoreTitleIDs;

bool titlemenuIsUpdating = false;
bool titleMenuInitialLoadDone = false;
bool preloadTitles = true;

void titlesInit()
{
	amInit();
}

void titlesExit()
{
	amExit();
}

void initTitleInfo(titleInfo_s* ti, u8 mediatype, u64 title_id)
{
	if(!ti)return;

	ti->mediatype = mediatype;
	ti->title_id = title_id;
	ti->icon = NULL;
}

void freeTitleInfo(titleInfo_s* ti)
{
	if(!ti)return;

	ti->title_id = 0x0;
	if(ti->icon)free(ti->icon);
	ti->icon = NULL;
}

Result loadTitleInfoIcon(titleInfo_s* ti)
{
	if(!ti || ti->icon)return -1;

	ti->icon = malloc(sizeof(smdh_s));
	if(!ti->icon)return -1;

	Handle fileHandle;
	u32 archivePath[] = {ti->title_id & 0xFFFFFFFF, (ti->title_id >> 32) & 0xFFFFFFFF, ti->mediatype, 0x00000000};
	static const u32 filePath[] = {0x00000000, 0x00000000, 0x00000002, 0x6E6F6369, 0x00000000};	
	Result ret = FSUSER_OpenFileDirectly(NULL, &fileHandle, (FS_archive){0x2345678a, (FS_path){PATH_BINARY, 0x10, (u8*)archivePath}}, (FS_path){PATH_BINARY, 0x14, (u8*)filePath}, FS_OPEN_READ, FS_ATTRIBUTE_NONE);

	if(ret)
	{
		free(ti->icon);
		ti->icon = NULL;
		return ret;
	}

	u32 bytesRead;	
	ret = FSFILE_Read(fileHandle, &bytesRead, 0x0, ti->icon, sizeof(smdh_s));

	if(ret)
	{
		free(ti->icon);
		ti->icon = NULL;
	}

	FSFILE_Close(fileHandle);

	return ret;
}

bool application_filter(u64 tid)
{
	u32 tid_high = tid >> 32;
	return (tid_high == 0x00040010 || tid_high == 0x00040000 || tid_high == 0x00040002);
}

void initTitleList(titleList_s* tl, titleFilter_callback filter, u8 mediatype)
{
	if(!tl)return;

	tl->mediatype = mediatype;

	if(tl->filter) tl->filter = filter;
	else tl->filter = &application_filter;

	tl->num = 0;
	tl->titles = NULL;

	populateTitleList(tl);
}

void freeTitleList(titleList_s* tl)
{
	if(!tl)return;

	int i;
	for(i=0; i<tl->num; i++)freeTitleInfo(&tl->titles[i]);

	tl->num = 0;
	if(tl->titles)free(tl->titles);
	tl->titles = NULL;
}

int populateTitleList(titleList_s* tl)
{
	if(!tl)return 0;
	Result ret;

	u32 old_num = tl->num;

	u32 num;
	ret = AM_GetTitleCount(tl->mediatype, &num);

	if(ret)
	{
		freeTitleList(tl);
	}else if(num){
		// temp buffer is not ideal, but i like modularity
		u64* tmp = (u64*)malloc(sizeof(u64) * num);

		if(!tmp)
		{
			tl->num = 0;
			return 1;
		}

		ret = AM_GetTitleIdList(tl->mediatype, num, tmp);

		if(!ret)
		{
			int i;

			// apply tid filter
			for(i=0; i<num; i++)
			{
				if(!tl->filter(tmp[i]))
				{
					num--;
					tmp[i] = tmp[num];
					i--;
				}
			}

			if(tl->num != num || tl->mediatype == 2)
			{
				freeTitleList(tl);

				tl->num = num;

				if(tl->num) tl->titles = malloc(sizeof(titleInfo_s) * tl->num);

				if(tl->titles)
				{
					for(i=0; i<tl->num; i++)
					{
						initTitleInfo(&tl->titles[i], tl->mediatype, tmp[i]);
					}
				}
			}
		}else tl->num = 0;

		free(tmp);
	}

	return old_num != tl->num;
}

titleInfo_s* findTitleList(titleList_s* tl, u64 tid)
{
	if(!tl)return NULL;

	// special case : gamecard mediatype with 0 tid
	if(!tid && tl->mediatype == 2 && tl->num)return &tl->titles[0];

	int i;
	for(i=0; i<tl->num; i++)
	{
		if(tl->titles[i].title_id == tid)return &tl->titles[i];
	}

	return NULL;
}

titleInfo_s* findTitleBrowser(titleBrowser_s* tb, u8 mediatype, u64 tid)
{
	if(!tb || mediatype > 2)return NULL;

	return findTitleList(&tb->lists[mediatype], tid);
}

void initTitleBrowser(titleBrowser_s* tb, titleFilter_callback filter)
{
	if(!tb)return;

	int i;
	for(i=0; i<3; i++)
	{
		initTitleList(&tb->lists[i], filter, (u8)2-i);
	}

	tb->total = 0;
	tb->nextCheck = 0;
	tb->selectedId = 0;
	tb->selected = NULL;
}

void refreshTitleBrowser(titleBrowser_s* tb) {
    tb->total = 0;
    tb->selectedId = 0;
    
    int i;
    
    for(i=0; i<3; i++) {
        freeTitleList(&tb->lists[i]);
        populateTitleList(&tb->lists[i]);
        tb->total += tb->lists[i].num;
    }
}

void getIgnoredTitleIDs() {
    if (!ignoreTitleIDsLoaded) {
        ignoreTitleIDsLoaded = true;
        numIgnoreTitleIDs = 0;
        int maxTextSize = 20 * maxIgnoreTitleIDs;
        char ignoreText[maxTextSize];
        
        FILE *fp = fopen(ignoredTitlesPath, "r");
        
        if (fp != NULL) {
            fgets(ignoreText, maxTextSize, fp);
            char * split;
            split = strtok(ignoreText, ",");
            while (split != NULL) {
                strcpy(ignoreTitleIDs[numIgnoreTitleIDs], split);
                numIgnoreTitleIDs++;
                
                split = strtok(NULL, ",");
            }
        }
        fclose(fp);
    }
}

void saveIgnoredTitleIDs() {
    int n = 0;
    
    menuEntry_s * me = titleMenu.entries;
    while (me) {
        if (!me->showTick) {
            n++;
        }
        
        me = me->next;
    }
    
    char ignored[n*20];
    strcpy(ignored, "");
    
    int c=0;
    
    me = titleMenu.entries;
    while (me) {
        if (!me->showTick) {
            char sTitleId[17];
            sprintf(sTitleId, "%llu", me->title_id);
            strcat(ignored, sTitleId);
            
            if (c < n-1) {
                strcat(ignored, ",");
            }
            
            c++;
        }
        
        me = me->next;
    }
    
    FILE* fSave = fopen(ignoredTitlesPath, "w" );
    if (fSave != NULL) {
        fputs(ignored, fSave);
    }
    fclose(fSave);
    
    ignoreTitleIDsLoaded = false;
    getIgnoredTitleIDs();
}

bool titleIgnored(u64 titleID) {
    char sTitleId[17];
    sprintf(sTitleId, "%llu", titleID);
    
    int iIgnoreTitleID;
    for (iIgnoreTitleID=0; iIgnoreTitleID<numIgnoreTitleIDs; iIgnoreTitleID++) {
        if (strcmp(sTitleId, ignoreTitleIDs[iIgnoreTitleID]) == 0) {
            return true;
        }
    }
    
    return false;
}

void populateTitleMenu(menu_s* aTitleMenu, titleBrowser_s *tb, bool filter, bool forceHideRegionFree) {
    getIgnoredTitleIDs();
    
    if (!aTitleMenu) {
        return;
    }
    
    aTitleMenu->entries=NULL;
    aTitleMenu->numEntries=0;
    aTitleMenu->selectedEntry=0;
    aTitleMenu->scrollLocation=0;
    aTitleMenu->scrollVelocity=0;
    aTitleMenu->scrollBarSize=0;
    aTitleMenu->scrollBarPos=0;
    aTitleMenu->scrollTarget=0;
    aTitleMenu->atEquilibrium=false;
    
    if (!forceHideRegionFree) {
        addMenuEntryCopy(aTitleMenu, &regionfreeEntry);
        updateMenuIconPositions(aTitleMenu);
    }
    
    u64 cartTitleId = 0;
    
    int i;
    
    for(i=0; i<3; i++) {
        const titleList_s* tl = &tb->lists[i];
        titleInfo_s *titles = tl->titles;
        
        int titleNum;
        int count = tl->num;
        
        for (titleNum = 0; titleNum < count; titleNum++) {
            
            titleInfo_s aTitle = titles[titleNum];
            
            if (i == 2 && titleNum == count-1) {
                cartTitleId = aTitle.title_id;
            }
            else {
                if (filter && titleIgnored(aTitle.title_id)) {
                    continue;
                }
                
//                if (titleIgnored) {
//                    continue;
//                }
                
                if (!aTitle.icon) {
                    loadTitleInfoIcon(&aTitle);
                }
                
                if (!aTitle.icon) {
                    continue;
                }
                
                static menuEntry_s me;
                
                me.hidden = false;
                me.isTitleEntry = false;
                me.isRegionFreeEntry = false;
                
                extractSmdhData(aTitle.icon, me.name, me.description, me.author, me.iconData);
                
                me.title_id = aTitle.title_id;
                
                if (me.title_id == 1125968626461184) {
                    strcpy(me.name, "System Transfer");
                    strcpy(me.description, "System Transfer");
                }
                
                addMenuEntryCopy(aTitleMenu, &me);
//                titleMenu->numEntries = titleMenu->numEntries + 1;
                updateMenuIconPositions(aTitleMenu);
            }
        }
    }
    
    if (!forceHideRegionFree) {
        menuEntry_s * rf = &aTitleMenu->entries[0];
        rf->title_id = cartTitleId;
        rf->hidden = false;
    }
    
    updateMenuIconPositions(aTitleMenu);
}

titleInfo_s* getTitleWithID(titleBrowser_s* tb, u64 tid) {
    int i;
    for(i=0; i<3; i++) {
        const titleList_s* tl = &tb->lists[i];
        titleInfo_s *titles = tl->titles;
        
        int titleNum;
        int count = tl->num;// sizeof(titles) / sizeof(titleInfo_s);
                
        for (titleNum = 0; titleNum < count; titleNum++) {
            titleInfo_s * aTitle = &titles[titleNum];
            if (aTitle->title_id == tid) {
                return aTitle;
            }
        }
        
    }
    
    return NULL;
}

void updateTitleMenu(titleBrowser_s * aTitleBrowser, menu_s * aTitleMenu, char * titleText, bool filter, bool forceHideRegionFree) {
    titlemenuIsUpdating = true;
    
    if (!preloadTitles) {
        u8 dimmer[4];
        dimmer[0] = 128;
        dimmer[1] = 128;
        dimmer[2] = 128;
        dimmer[3] = 200;

        int x, y;
        
        int totalWidth = 240;
        int totalHeight = 320;
        
        for (y = 0; y < totalHeight; y++) {
            for (x = 0; x < totalWidth; x++) {
                gfxDrawSpriteAlphaBlend(GFX_BOTTOM, GFX_LEFT, dimmer, 1, 1, x, y);
            }
        }
        
        drawDisk(titleText);
        gfxFlip();
    }
    
    refreshTitleBrowser(aTitleBrowser);
    clearMenuEntries(aTitleMenu);
    populateTitleMenu(aTitleMenu, aTitleBrowser, filter, forceHideRegionFree);
    
    titlemenuIsUpdating = false;
}

//bool trueBool = true;

void updateFilterTicks(menu_s * aTitleMenu) {
    menuEntry_s * me = aTitleMenu->entries;
    while (me) {
        if (titleIgnored(me->title_id)) {
            me->showTick = NULL;
        }
        else {
            me->showTick = &trueBool;
        }
        
        me = me->next;
    }
}

void toggleTitleFilter(menuEntry_s *me, menu_s * m) {
    if (me->showTick == NULL) {
        me->showTick = &trueBool;
    }
    else {
        me->showTick = NULL;
    }
}





