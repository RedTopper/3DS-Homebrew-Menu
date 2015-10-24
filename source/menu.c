#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <3ds.h>
#include <math.h>

#include "filesystem.h"
#include "menu.h"
#include "smdh.h"
#include "regionfree.h"
#include "regionfree_bin.h"
#include "folders.h"
#include "background.h"
#include "titles.h"

#include "appshadow_bin.h"
#include "appiconalphamask_bin.h"
#include "appbackgroundalphamask_bin.h"
#include "cartbackgroundalphamask_bin.h"
#include "pageiconalphamask_bin.h"

#include "pageControlPanelLeftAlphaMask_bin.h"
#include "pageControlPanelRightAlphaMask_bin.h"

#include "config.h"
#include "colours.h"
#include "button.h"

#include "MAFontRobotoRegular.h"

#include "statusbar.h"
#include "alert.h"
#include "MAGFX.h"

#include "tickalphamask_bin.h"

#include "touchblock.h"

#include "themegfx.h"

#define buttonTagTopLeft 10
#define buttonTagTopRight 20
#define buttonTagBottomLeft 30
#define buttonTagBottomRight 40

bool displayTitleID = false;

bool menuForceReturnTrue = false;

int menuStatusIcons = 0;
int menuStatusSettings = 1;
int menuStatusHelp = 2;
int menuStatusFolders = 3;
int menuStatusFolderChanged = 4;
//int menuStatusFoldersHelp = 5;
int menuStatusTitleBrowser = 6;
int menuStatusOpenHomeMenuApps = 7;
int menuStatusHomeMenuApps = 8;
int menuStatusColourSettings = 9;
int menuStatusColourAdjust = 10;
int menuStatusTranslucencyTop = 11;
int menuStatusTranslucencyBottom = 12;
int menuStatusPanelSettings = 13;
int menuStatusThemeSelect = 14;
int menuStatusWaterSettings = 15;
int menuStatusThemeSettings = 16;
int menuStatusGridSettings = 17;

bool killTitleBrowser = false;
bool thirdRowVisible = false;
bool dPadNavigation = true;

int translucencyTopBar = 255;
int translucencyBottomBar = 255;
int translucencyBarIcons = 255;
int translucencyBarIconSymbols = 255;
int translucencyAppBackgrounds = 255;
int translucencyPageControls = 255;
int translucencyWater = 255;
int translucencyAppShadow = 255;

bool toolbarNeedsUpdate = true;

int menuStatus = 0;

//int previousSelectedEntry = -1;
int dPadSelectedToolbarButton = -1;

menuEntry_s regionfreeEntry;
menu_s menu;

//int rowPosition = 0;
//int colPosition = 0;
//int pagePosition = 0;

int totalCols = 4;
int totalRows = 2;
//int totalPages = 0;

int touchX = 0;
int touchY = 0;


// See the "REALLY WEIRD BUG ALERT!" in initMenu()
button fakeButton;

button pageArrowLeft;
button pageArrowRight;

buttonList toolbarButtons;

int indexOfFirstVisibleMenuEntry(menu_s *m) {
    if (!showRegionFree) {
        return 1;
    }
    return 0;
}

void setMenuStatus(int status) {
    dPadSelectedToolbarButton = -1;
    btnListUnHighlight(&toolbarButtons);
    startBlockingTouches();
    menuStatus = status;
}

void menuRegionFreeToggled() {
    menu.entries[0].hidden = !showRegionFree;
    updateMenuIconPositions(&menu);
    gotoFirstIcon(&menu);
}

bool menuReloadRequired = false;

void toggleThirdRow() {
    totalRows = (totalRows == 3) ? 2 : 3;
    thirdRowVisible = (totalRows == 3);
    updateMenuIconPositions(&settingsMenu);
}

void addToolbarButton(int buttonType, void (*callback)(), int tag) {
    button aButton;
    btnSetButtonType(&aButton, buttonType);
    aButton.callback = callback;
    aButton.tag = tag;
    btnAddButtonToButtonList(&aButton, &toolbarButtons);
}

void quitSettings(menu_s* m);
void showFolders();
void showHomeMenuApps();

void showSettings() {
    if (settingsMenuNeedsInit) {
        initConfigMenu();
    }
    
    updateMenuIconPositions(&settingsMenu);
    gotoFirstIcon(&settingsMenu);
    setMenuStatus(menuStatusSettings);
}

void checkReturnToGrid(menu_s* m) {
    if (m->rowPosition == -1) {
        gotoFirstIcon(m);
    }
}

#define menuTopLeftActionSourceKeyB 10
#define menuTopLeftActionSourceTopLeft 20

void handleMenuTopLeftActions(int source) {
    if (source == menuTopLeftActionSourceTopLeft && menuStatus == menuStatusIcons) {
        showSettings();
    }
    else if (menuStatus == menuStatusSettings) {
        quitSettings(&menu);
    }
    else if (menuStatus == menuStatusTitleBrowser || menuStatus == menuStatusHomeMenuApps) {
        killTitleBrowser = true;
    }
    else if (menuStatus == menuStatusFolders) {
        if (source == menuTopLeftActionSourceTopLeft) {
            checkReturnToGrid(&menu);
        }

        setMenuStatus(menuStatusIcons);
    }
    else if (menuStatus == menuStatusColourAdjust) {
        saveColour(settingsColour);
        setMenuStatus(menuStatusColourSettings);
    }
    else if (menuStatus == menuStatusColourSettings) {
        alphaImagesDrawn = false;
        setMenuStatus(menuStatusThemeSettings);
    }
    else if (menuStatus == menuStatusHelp) {
        handleHelpBackButton();
    }
    else if (menuStatus == menuStatusThemeSettings || menuStatus == menuStatusGridSettings) {
        setMenuStatus(menuStatusSettings);
    }
    else if (menuStatus == menuStatusTranslucencyTop || menuStatus == menuStatusTranslucencyBottom || menuStatus == menuStatusPanelSettings || menuStatus == menuStatusThemeSelect || menuStatus == menuStatusWaterSettings) {
        setMenuStatus(menuStatusThemeSettings);
    }
}

void toolbarTopLeftAction() {
    handleMenuTopLeftActions(menuTopLeftActionSourceTopLeft);
}

void toolbarTopRightAction() {
    if (menuStatus == menuStatusIcons) {
        showHelp();
    }
    else if (menuStatus == menuStatusFolders) {
        showHelpWithForcedText(foldersHelpTitle, foldersHelpBody);
    }
    else if (menuStatus == menuStatusThemeSelect) {
        showHelpWithForcedText(themesHelpTitle, themesHelpBody);
    }
}

void toolbarBottomLeftAction() {
    if (menuStatus == menuStatusIcons) {
        if (!titleMenuInitialLoadDone && !titlemenuIsUpdating) {
            updateTitleMenu(&titleBrowser, &titleMenu, "Loading titles");
            titleMenuInitialLoadDone = true;
            showHomeMenuApps();
        }
        else {
            showHomeMenuApps();
        }
    }
}

void toolbarBottomRightAction() {
    if (menuStatus == menuStatusIcons) {
        showFolders();
    }
}

void loadThemeConfig() {
    waterEnabled = getConfigBoolForKey("waterEnabled", true, configTypeTheme);
    panelAlphaTop = getConfigIntForKey("panelAlphaTop", 0, configTypeTheme);
    panelAlphaBottom = getConfigIntForKey("panelAlphaBottom", 0, configTypeTheme);
    
    translucencyTopBar = getConfigIntForKey("translucencyTopBar", 255, configTypeTheme);
    translucencyBottomBar = getConfigIntForKey("translucencyBottomBar", 255, configTypeTheme);
    translucencyBarIcons = getConfigIntForKey("translucencyBarIcons", 255, configTypeTheme);
    translucencyBarIconSymbols = getConfigIntForKey("translucencyBarIconSymbols", 255, configTypeTheme);
    translucencyAppBackgrounds = getConfigIntForKey("translucencyAppBackgrounds", 255, configTypeTheme);
    translucencyPageControls = getConfigIntForKey("translucencyPageControls", 255, configTypeTheme);
    translucencyWater = getConfigIntForKey("translucencyWater", 255, configTypeTheme);
    translucencyAppShadow = getConfigIntForKey("translucencyAppShadow", 255, configTypeTheme);
    
    panelLeftOffsetTop = getConfigIntForKey("panelLeftOffsetTop", 0, configTypeTheme);
}

void initMenu(menu_s* m)
{
    if(!m) {
        return;
    }
    
    /*
     Initial menu status should always be to show the icons
     */
    setMenuStatus(menuStatusIcons);
    
    /*
     Configure the toolbar buttons and page select buttons
     */
    addToolbarButton(btnButtonTypeToolbarLeft, &toolbarTopLeftAction, buttonTagTopLeft);
    addToolbarButton(btnButtonTypeToolbarRight, &toolbarTopRightAction, buttonTagTopRight);
    addToolbarButton(btnButtonTypeToolbarBottomRight, &toolbarBottomRightAction, buttonTagBottomRight);
    addToolbarButton(btnButtonTypeToolbarBottomLeft, &toolbarBottomLeftAction, buttonTagBottomLeft);
    
    btnSetButtonType(&pageArrowLeft, btnButtonTypePageArrowLeft);
    btnSetButtonType(&pageArrowRight, btnButtonTypePageArrowRight);
    
    
    /*
     
     REALLY WEIRD BUG ALERT!
     
     If the last toolbar button to be drawn is not highlighted then the text wrapping when drawing the
     description of the currently selected app does not work.
     
     I simply cannot work out why this is happening, so I have cheated.
     
     This fake button is configured exactly the same way as another toolbar button,
     but it gets drawn off screen. It is set to be highlighted all the time, so this
     allows the text wrapping to work properly.
     
     I'm guessing that this is 'undefined behaviour' due to some bad memory management,
     so if anyone can figure out what is actually going on then this can be removed.
     
     */

    btnSetButtonType(&fakeButton, btnButtonTypeToolbarBottomLeft);
    fakeButton.x = -50;
    fakeButton.y = 100;
    fakeButton.visible = true;
    fakeButton.highlighted = true;
    
    /*
     If the region free launcher is available, load the setting which determines whether it should be shown in the grid
     */
    if (regionFreeAvailable) {
        showRegionFree = getConfigBoolForKey("showRegionFree", regionFreeAvailable, configTypeMain);
    }

    /*
     Load other settings
     */
    sortAlpha = getConfigBoolForKey("sortAlpha", false, configTypeMain);
    totalRows = getConfigIntForKey("totalRows", 3, configTypeMain);
    clock24 = getConfigBoolForKey("clock24", false, configTypeMain);
    waterAnimated = getConfigBoolForKey("animatedWater", true, configTypeMain);
    showAppBackgrounds = getConfigBoolForKey("showAppBackgrounds", true, configTypeMain);
    wrapScrolling = getConfigBoolForKey("wrapScrolling", true, configTypeMain);
    keysExciteWater = getConfigBoolForKey("keysExciteWater", true, configTypeMain);
    dPadNavigation = getConfigBoolForKey("dPadNavigation", true, configTypeMain);
    displayTitleID = getConfigBoolForKey("displayTitleID", false, configTypeMain);
    
    loadThemeConfig();
    
    //Used for ticking the icon in settings
    thirdRowVisible = (totalRows == 3);
    
    /*
     Menu init
     */
	m->entries=NULL;
	m->numEntries=0;
    m->selectedEntry=indexOfFirstVisibleMenuEntry(m);
	m->scrollLocation=0;
	m->scrollVelocity=0;
	m->scrollBarSize=0;
	m->scrollBarPos=0;
	m->scrollTarget=0;
	m->atEquilibrium=false;

    
    /*
     Add the region free entry to the menu if applicable
     */
	if(regionFreeAvailable) {
        regionfreeEntry.isRegionFreeEntry = true;
        regionfreeEntry.hidden = !showRegionFree;
        
		extractSmdhData((smdh_s*)regionfree_bin, regionfreeEntry.name, regionfreeEntry.description, regionfreeEntry.author, regionfreeEntry.iconData);
		strcpy(regionfreeEntry.executablePath, REGIONFREE_PATH);
		addMenuEntryCopy(m, &regionfreeEntry);
	}
}

static inline s16 getEntryLocationPx(menu_s* m, int px)
{
	return 240-px+fptToInt(m->scrollLocation);
}

static inline s16 getEntryLocation(menu_s* m, int n)
{
	return getEntryLocationPx(m, (n+1)*ENTRY_WIDTH);
}

u8 bottomBar[18*320*4];

void drawBottomStatusBar(char* title) {
    int height = 18;

    if (toolbarNeedsUpdate) {
        toolbarNeedsUpdate = false;
        rgbColour * tintCol = tintColour();
        MAGFXTranslucentRect(height, 320, tintCol->r, tintCol->g, tintCol->b, translucencyBottomBar, bottomBar);
    }
    
    gfxDrawSpriteAlphaBlend(GFX_BOTTOM, GFX_LEFT, bottomBar, height, 320, 240-height, 0);
    
    
//    gfxDrawRectangle(GFX_BOTTOM, GFX_LEFT, (u8[]){tintCol->r, tintCol->g, tintCol->b}, 240-height, 0, height, 400);
    
    rgbColour * light = lightTextColour();
    
    int stringLength = MATextWidthInPixels(title, &MAFontRobotoRegular10);
    MADrawText(GFX_BOTTOM, GFX_LEFT, 240-height-2, (320/2)-(stringLength/2), title, &MAFontRobotoRegular10, light->r, light->g, light->b);
    
    button * leftToolbarButton = btnButtonInListWithTag(&toolbarButtons, buttonTagTopLeft);
    button * rightToolbarButton = btnButtonInListWithTag(&toolbarButtons, buttonTagTopRight);
    button * bottomLeftButton = btnButtonInListWithTag(&toolbarButtons, buttonTagBottomLeft);
    button * bottomRightButton = btnButtonInListWithTag(&toolbarButtons, buttonTagBottomRight);
    
    leftToolbarButton->visible = true;
    rightToolbarButton->visible = false;
    bottomRightButton->visible = false;
    bottomLeftButton->visible = false;
    
    if (menuStatus == menuStatusIcons) {
        rightToolbarButton->visible = true;
        bottomRightButton->visible = true;
        bottomLeftButton->visible = true;
    }
    else if (menuStatus == menuStatusFolders || menuStatus == menuStatusThemeSelect) {
        rightToolbarButton->visible = true;
    }
    
    int buttonIconLeft = 0;
    int buttonIconRight = 0;
    
    if (menuStatus == menuStatusIcons) {
        buttonIconLeft = btnButtonIconSpanner;
        buttonIconRight = btnButtonIconQuestionMark;
    }
    else if (menuStatus == menuStatusHelp || menuStatus == menuStatusSettings || menuStatus == menuStatusTitleBrowser || menuStatus == menuStatusHomeMenuApps) { //|| menuStatus == menuStatusFoldersHelp
        buttonIconLeft = btnButtonIconBackArrow;
    }
    else if (menuStatus == menuStatusFolders || menuStatus == menuStatusThemeSelect) {
        buttonIconLeft = btnButtonIconBackArrow;
        buttonIconRight = btnButtonIconQuestionMark;
    }
    else if (menuStatus == menuStatusColourSettings || menuStatus == menuStatusColourAdjust || menuStatus == menuStatusTranslucencyTop || menuStatus == menuStatusTranslucencyBottom || menuStatus == menuStatusPanelSettings || menuStatus == menuStatusWaterSettings || menuStatus == menuStatusThemeSettings || menuStatus == menuStatusGridSettings) {
        buttonIconLeft = btnButtonIconBackArrow;
    }

    leftToolbarButton->buttonIcon = buttonIconLeft;
    rightToolbarButton->buttonIcon = buttonIconRight;
    
    bottomLeftButton->buttonIcon = btnButtonIconHome;
    bottomRightButton->buttonIcon = btnButtonIconFolder;
    
    btnDrawButtonList(&toolbarButtons);
    btnDrawButton(&fakeButton);
}

int * coordsForMenuEntry(int row, int col, menu_s *m) {
    int xOffset = 121;
    int yOffset = 33;
    int gap = 15;
    
//    int totalRows = totalRowsForMenu(m);
    
    if (totalRows == 3) {
        xOffset += 31;
    }
    
    int x = xOffset - ( row * ENTRY_ICON_HEIGHT ) - (row * gap);
    int y = yOffset + (( col * ENTRY_ICON_WIDTH ) + (col * gap));
    
    static int  r[2];
    r[0] = x;
    r[1] = y;
    return r;
}

u8 appBackground[56*56*4];
u8 pageSelected[13*13*4];
u8 pageUnselected[13*13*4];
u8 cartBackground[59*59*4];
u8 cartBackgroundSelected[59*59*4];
u8 appBackgroundSelected[56*56*4];
u8 tick [48*48*4];
bool alphaImagesDrawn = false;

u8 pageControlPanelLeft[81*36*4];
u8 pageControlPanelRight[81*36*4];
bool pageControlPanelsDrawn = false;

void drawGrid(menu_s* m) {
    rgbColour * inactiveCol = inactiveColour();
    rgbColour * tintCol = tintColour();
    
    if (!alphaImagesDrawn) {
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)appbackgroundalphamask_bin, appBackground, 56, 56);
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)pageiconalphamask_bin, pageUnselected, 13, 13);
        MAGFXImageWithRGBAndAlphaMask(tintCol->r, tintCol->g, tintCol->b, (u8*)pageiconalphamask_bin, pageSelected, 13, 13);
        MAGFXImageWithRGBAndAlphaMask(tintCol->r, tintCol->g, tintCol->b, (u8*)cartbackgroundalphamask_bin, cartBackgroundSelected, 59, 59);
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)cartbackgroundalphamask_bin, cartBackground, 59, 59);
        MAGFXImageWithRGBAndAlphaMask(tintCol->r, tintCol->g, tintCol->b, (u8*)appbackgroundalphamask_bin, appBackgroundSelected, 56, 56);
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)tickalphamask_bin, tick, 48, 48);
        
        alphaImagesDrawn = true;
    }
    
    if (!pageControlPanelsDrawn) {
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)pageControlPanelLeftAlphaMask_bin, pageControlPanelLeft, 81, 36);
        MAGFXImageWithRGBAndAlphaMask(inactiveCol->r, inactiveCol->g, inactiveCol->b, (u8*)pageControlPanelRightAlphaMask_bin, pageControlPanelRight, 81, 36);
        
        pageControlPanelsDrawn = true;
    }
    
    MAGFXDrawPanel(GFX_TOP);
    
    if (m->totalPages > 1) {
        if (m->pagePosition > 0 || wrapScrolling) {
            gfxDrawSpriteAlphaBlendFade(GFX_BOTTOM, GFX_LEFT, (u8*)pageControlPanelLeft, 81, 36, 80, 0, panelAlphaBottom);
            btnDrawButton(&pageArrowLeft);
        }
        
        if (m->pagePosition < (m->totalPages - 1) || wrapScrolling) {
            gfxDrawSpriteAlphaBlendFade(GFX_BOTTOM, GFX_LEFT, (u8*)pageControlPanelRight, 81, 36, 80, 284, panelAlphaBottom);
            btnDrawButton(&pageArrowRight);
        }
        
        int totalIndicatorWidth = 13 * m->totalPages;
        int pageIndicatorX = 5;
        int pageIndicatorY = (320-totalIndicatorWidth)/2;
        
        int count;
        
        for (count=0; count < m->pagePosition; count++) {
            gfxDrawSpriteAlphaBlendFade(GFX_BOTTOM, GFX_LEFT, pageUnselected, 13, 13, pageIndicatorX, pageIndicatorY, translucencyPageControls);
            pageIndicatorY += 13;
        }
        
        gfxDrawSpriteAlphaBlend(GFX_BOTTOM, GFX_LEFT, pageSelected, 13, 13, pageIndicatorX, pageIndicatorY);
        pageIndicatorY += 13;
        
        for (count=0; count<(m->totalPages - m->pagePosition - 1); count++) {
            gfxDrawSpriteAlphaBlendFade(GFX_BOTTOM, GFX_LEFT, pageUnselected, 13, 13, pageIndicatorX, pageIndicatorY, translucencyPageControls);
            pageIndicatorY += 13;
        }
    }
    
    int totalDrawn = 0;
    
    menuEntry_s* me=m->entries;
    int i=0;
    int h=0;
    while(me) {
        if (!me->hidden && me->page == m->pagePosition) {
            h+=drawMenuEntry(me, GFX_BOTTOM, (i==m->selectedEntry && m->rowPosition>-1), m);
            totalDrawn++;
        }
        
        me=me->next;
        i++;
    }
    
    if (showAppBackgrounds) {
//        int totalRows = m->totalRows;
        int totalSpaces = totalRows * totalCols;
        int numPadding = totalSpaces - totalDrawn;

        int i;
        int r = totalRows-1;
        int c = totalCols-1;
        for (i=0; i<numPadding; i++) {
            int * coords = coordsForMenuEntry(r, c, m);
            int x = coords[0];
            int y = coords[1];
            
            if (themeHasAppBackgroundImage) {
                drawThemeImage(themeImageAppBackground, GFX_BOTTOM, x+3, y+4);
            }
            else {
                gfxDrawSpriteAlphaBlendFade(GFX_BOTTOM, GFX_LEFT, appBackground, 56, 56, x+3, y+4, translucencyAppBackgrounds);
            }

            c--;
            if (c < 0) {
                c = totalCols-1;
                r--;
            }
            
            if (r<0) {
                //                break;
            }
            
        }
    }
}

void drawMenu(menu_s* m)
{
    if(!m) {
        return;
    }
    
    if (!showRegionFree && m->selectedEntry == 0) {
        m->selectedEntry = 1;
    }

    
    if (menuStatusIcons == menuStatusIcons) {
        char * cfn = currentFolderName();
        char title[strlen(cfn) + strlen("Folder: ")];
        strcpy(title, "Folder: ");
        strcat(title, cfn);
        drawBottomStatusBar(title);
        free(cfn);
        
        if (m->numEntries == 1) {
            char * emptyFolderText = "Empty folder";
            int len = MATextWidthInPixels(emptyFolderText, &MAFontRobotoRegular16);
            rgbColour * dark = darkTextColour();
            MADrawText(GFX_BOTTOM, GFX_LEFT, 110, (320/2)-(len/2), emptyFolderText, &MAFontRobotoRegular16, dark->r, dark->g, dark->b);
        }
        else {
            drawGrid(m);
        }
	}
}

void updateMenuIconPositions(menu_s* m) {
    int currentColumn = 0;
    int currentRow = 0;
    int currentPage = 0;
    m->totalPages = 0;
    int i=0;
//    int totalRows = m->totalRows;
    
    menuEntry_s* me=m->entries;
    while(me)
    {
        if (!me->hidden) {
            me->row = currentRow;
            me->col = currentColumn;
            me->page = currentPage;
            
            
            if (currentRow == 0 && currentColumn == 0) {
                m->totalPages = m->totalPages + 1;
            }
            
            currentColumn++;
            
            if (currentColumn > 3) {
                currentColumn = 0;
                currentRow++;
            }
            
            if (currentRow >= totalRows) {
                currentRow = 0;
                currentColumn = 0;
                currentPage++;
            }
        }
        
        me=me->next;
        i++;
    }
}

void addMenuEntry(menu_s* m, menuEntry_s* me)
{
	if(!m || !me)return;

	// add to the end of the list
	menuEntry_s** l = &m->entries;
	while(*l)l=&(*l)->next;
	*l = me;
	me->next = NULL;
	m->numEntries++;
}

void addMenuEntryCopy(menu_s* m, menuEntry_s* me)
{
	if(!m || !me)return;

	menuEntry_s* me2=malloc(sizeof(menuEntry_s));
	if(!me2)return;

	memcpy(me2, me, sizeof(menuEntry_s));
	
	addMenuEntry(m, me2);
}

void freeMenuEntry(menuEntry_s* me)
{
	if(!me)return;

	freeDescriptor(&me->descriptor);
}

void clearMenuEntries(menu_s* m)
{
	if(!m)return;

	m->selectedEntry=indexOfFirstVisibleMenuEntry(m);

	menuEntry_s* me = m->entries;
	menuEntry_s* temp = NULL;
	while(me)
	{
		temp=me->next;
		me->next = NULL;
		freeMenuEntry(me);
		free(me);
		me = temp;
	}

	m->numEntries = 0;
	m->entries = NULL;

	if(regionFreeAvailable)
	{
		// should always be available
		addMenuEntryCopy(m, &regionfreeEntry);
	}
}

void createMenuEntry(menu_s* m, char* execPath, char* name, char* description, char* author, u8* iconData)
{
	if(!m || !name || !description || !iconData)return;

	menuEntry_s* me=malloc(sizeof(menuEntry_s));
	if(!me)return;

	initMenuEntry(me, execPath, name, description, author, iconData);
	
	addMenuEntry(m, me);
}

menuEntry_s* getMenuEntry(menu_s* m, u16 n)
{
	if(!m || n>=m->numEntries)return NULL;
	menuEntry_s* me=m->entries;
	while(n && me){me=me->next; n--;}
	return me;
}

int indexOfMenuEntryAtPageRowColInMenu(int page, int row, int col, menu_s* m) {
    if (!m) return -1;
    
    menuEntry_s* me = m->entries;
    int i=0;
    while(me) {
        if (me->page == page && me->row == row && me->col == row) {
            if (me->hidden) {
                i++;
            }
            
            return i;
        }
        me=me->next;
        i++;
    }
    
    return -1;
}

void checkGotoNextPage(menu_s* m, s8 *move) {
    m->pagePosition = m->pagePosition + 1;
    bool pageChanged = false;
    
    if (m->pagePosition > (m->totalPages - 1)) {
        if (wrapScrolling) {
            m->pagePosition = 0;
            pageChanged = true;
        }
        else {
            m->pagePosition = m->pagePosition - 1;
            pageChanged = false;
        }
    }
    else {
        pageChanged = true;
    }
    
    if (pageChanged) {
        btnListUnHighlight(&toolbarButtons);
        dPadSelectedToolbarButton = -1;
        
        int oldRowPosition = m->rowPosition;
        m->rowPosition = 0;
        m->colPosition = 0;
        int newSelectedIndex = indexOfMenuEntryAtPageRowColInMenu(m->pagePosition, 0, 0, m);
        m->selectedEntry = newSelectedIndex;
        
        if ((m->selectedEntry + (4*oldRowPosition)) <= (m->numEntries - 1)) {
            *move += (4*oldRowPosition);
            m->colPosition = 0;
            m->rowPosition = oldRowPosition;
        }
    }
}

void checkGotoPreviousPage(menu_s* m, s8 *move) {
    m->pagePosition = m->pagePosition - 1;
    bool pageChanged = false;
    
    if (m->pagePosition < 0) {
        if (wrapScrolling) {
            m->pagePosition = (m->totalPages - 1);
            pageChanged = true;
        }
        else {
            m->pagePosition = m->pagePosition + 1;
            pageChanged = false;
        }
    }
    else {
        pageChanged = true;
    }
    
    if (pageChanged) {
        btnListUnHighlight(&toolbarButtons);
        dPadSelectedToolbarButton = -1;
        
        //Store the old row which was selected
        int oldRowPosition = m->rowPosition;
        
        //Move the selection to the first icon on this page
        m->rowPosition = 0;
        m->colPosition = 0;
        int newSelectedIndex = indexOfMenuEntryAtPageRowColInMenu(m->pagePosition, 0, 0, m);
        m->selectedEntry = newSelectedIndex;
        
        //Move the selection to the same position on the new page
        *move += (3 + (4*oldRowPosition));
        m->colPosition = 3;
        m->rowPosition = oldRowPosition;
        
        //If we moved back to the last page
        if (m->pagePosition == (m->totalPages - 1)) {
            
            //If the current position on the last page is not occupied by an app icon
            if (*move + m->selectedEntry >= m->numEntries) {
                
                //While the current position on the page is still not occupied by an icon
                while (*move + m->selectedEntry >= m->numEntries) {
                    //Go left by one icon
                    *move -= 1;
                    m->colPosition = m->colPosition - 1;
                    
                    //If we have got to the beginning of the row, go to the end of the next row up
                    if (m->colPosition < 0) {
                        m->colPosition = 3;
                        m->rowPosition = m->rowPosition - 1;
                    }
                    
                    //If we have dropped off the left and edge of the first row then we must be on an empty page
                    //This really shouldn't ever happen, but just in case, this code will reset back to the first
                    //icon on the page and break out of the while loop so we don't hang here
                    if (m->rowPosition < 0) {
                        m->colPosition = 0;
                        m->rowPosition = 0;
                        int newSelectedIndex = indexOfMenuEntryAtPageRowColInMenu(m->pagePosition, 0, 0, m);
                        m->selectedEntry = newSelectedIndex;
                        break;
                    }
                }
            }
        }
    }
}

bool touchWithin(int touchX, int touchY, int targetX, int targetY, int targetW, int targetH) {
    if (touchX > targetY && touchX < (targetY+targetW) && (240-touchY) > targetX && (240-touchY) < (targetX+targetH)) {
        return true;
    }
    
    return false;
}

void gotoFirstIcon(menu_s* m) {
    m->rowPosition = 0;
    m->colPosition = 0;
    m->pagePosition = 0;
    
    int selectedEntry = 0;
    if (m->entries[0].hidden) {
        selectedEntry++;
    }

    m->selectedEntry = selectedEntry;
}

void reloadMenu(menu_s* m) {
    clearMenuEntries(m);
    scanHomebrewDirectory(m, currentFolder());
    m->entries[0].hidden = !showRegionFree;
    updateMenuIconPositions(m);
    gotoFirstIcon(m);
    menuReloadRequired = false;
}

void quitSettings(menu_s* m) {
    setConfigBool("showRegionFree", showRegionFree, configTypeMain);
    setConfigBool("sortAlpha", sortAlpha, configTypeMain);
    setConfigInt("totalRows", totalRows, configTypeMain);
    setConfigBool("clock24", clock24, configTypeMain);
    setConfigBool("animatedWater", waterAnimated, configTypeMain);
    setConfigBool("showAppBackgrounds", showAppBackgrounds, configTypeMain);
    setConfigBool("wrapScrolling", wrapScrolling, configTypeMain);
    setConfigBool("keysExciteWater", keysExciteWater, configTypeMain);
    setConfigBool("dPadNavigation", dPadNavigation, configTypeMain);
    setConfigBool("randomTheme", randomTheme, configTypeMain);
    setConfigBool("displayTitleID", displayTitleID, configTypeMain);
    
    setConfigBool("waterEnabled", waterEnabled, configTypeTheme);
    setConfigBool("showLogo", showLogo, configTypeTheme);
    setConfigInt("panelAlphaTop", panelAlphaTop, configTypeTheme);
    setConfigInt("panelAlphaBottom", panelAlphaBottom, configTypeTheme);
    
    setConfigInt("translucencyTopBar", translucencyTopBar, configTypeTheme);
    setConfigInt("translucencyWater", translucencyWater, configTypeTheme);
    setConfigInt("translucencyAppShadow", translucencyAppShadow, configTypeTheme);
    
    setConfigInt("translucencyBottomBar", translucencyBottomBar, configTypeTheme);
    setConfigInt("translucencyBarIcons", translucencyBarIcons, configTypeTheme);
    setConfigInt("translucencyBarIconSymbols", translucencyBarIconSymbols, configTypeTheme);
    setConfigInt("translucencyAppBackgrounds", translucencyAppBackgrounds, configTypeTheme);
    setConfigInt("translucencyPageControls", translucencyPageControls, configTypeTheme);
    
    setConfigInt("preloadTitles", preloadTitles, configTypeMain);
    
    setConfigInt("panelLeftOffsetTop", panelLeftOffsetTop, configTypeTheme);
    
    saveConfig();
    
    if (menuReloadRequired) {
        reloadMenu(m);
    }
    
    updateMenuIconPositions(m);
    setMenuStatus(menuStatusIcons);
    checkReturnToGrid(m);
}

bool trueBool = true;

void updateMenuTicks(menu_s* m, char * selectedString) {
    menuEntry_s* me = m->entries;
    
    if (m == &themesMenu && randomTheme) {
        bool first = true;
        while (me) {
            if (first) {
                me->showTick = &trueBool;
                first = false;
            }
            else {
                me->showTick = NULL;
            }
            
            me=me->next;
        }
    }
    else {
        
        
        while(me) {
            if (strcmp(me->name, selectedString) == 0) {
                me->showTick = &trueBool;
            }
            else {
                me->showTick = NULL;
            }
            
            me=me->next;
        }
    }
    
    
    
    //    free(cfn);
}

void showFolders() {
    buildFoldersList();
    char * cfn = currentFolderName();
    updateMenuTicks(&foldersMenu, cfn);
    free(cfn);
    checkReturnToGrid(&foldersMenu);
    setMenuStatus(menuStatusFolders);
}

void showHomeMenuApps() {
    setMenuStatus(menuStatusOpenHomeMenuApps);
}

void keyBAction() {
    handleMenuTopLeftActions(menuTopLeftActionSourceKeyB);
}

void switchToolbarButtons() {
    btnListUnHighlight(&toolbarButtons);
    
    if (dPadSelectedToolbarButton == 0) {
        dPadSelectedToolbarButton = 1;
    }
    else if (dPadSelectedToolbarButton == 1) {
        dPadSelectedToolbarButton = 0;
    }
    else if (dPadSelectedToolbarButton == 2) {
        dPadSelectedToolbarButton = 3;
    }
    else if (dPadSelectedToolbarButton == 3) {
        dPadSelectedToolbarButton = 2;
    }
}

void handleDPadToolbarActions(menu_s* m) {
    if (dPadSelectedToolbarButton == 0) {
        toolbarTopLeftAction();
    }
    else if (dPadSelectedToolbarButton == 1) {
        toolbarTopRightAction();
    }
    else if (dPadSelectedToolbarButton == 2) {
        toolbarBottomRightAction();
    }
    else if (dPadSelectedToolbarButton == 3) {
        toolbarBottomLeftAction();
    }
}

int indexOfFirstVisibleMenuEntryOnPage(int page, menu_s* m) {
    int first = totalRows * totalCols * page;
    
    if (!showRegionFree) {
        first++;
    }
    
    if (first >= m->numEntries) {
        first = m->numEntries - 1;
    }
    
    return first;
}

int indexOfLastVisibleMenuEntryOnPage(int page, menu_s* m) {
    int last = totalRows * totalCols * (page+1);
    last--;
    
    if (!showRegionFree) {
        last++;
    }
    
    if (last >= m->numEntries) {
        last = m->numEntries - 1;
    }
    
    return last;
}

#include "logText.h"

bool updateGrid(menu_s* m) {
    if(!m) {
        return false;
    }
    if(!m->numEntries) {
        return false;
    }
    
    s8 move=0;
    circlePosition cstick;
    touchPosition touch;
    hidCstickRead(&cstick);
    hidTouchRead(&touch);
    cstick.dy=(abs(cstick.dy)<5)?0:cstick.dy;
    
    touchX = touch.px;
    touchY = touch.py;
    
    if (dPadSelectedToolbarButton > -1) {
        toolbarButtons.buttons[dPadSelectedToolbarButton]->highlighted = true;
    }
    
    if (hidKeysDown()&KEY_B) {
        keyBAction();
        return false;
    }
    
    if(hidKeysDown()&KEY_RIGHT) {
        if (m->rowPosition == -1) {
            switchToolbarButtons();
        }
        else if (m->colPosition < 3 && m->selectedEntry < (m->numEntries-1)) {
            move++;
            m->colPosition = m->colPosition + 1;
        }
        else if (m->colPosition == 3) {
            checkGotoNextPage(m, &move);
        }
        else if (m->selectedEntry == m->numEntries-1) {
            if (wrapScrolling) {
                checkGotoNextPage(m, &move);
            }
        }
    }
    
    if(hidKeysDown()&KEY_LEFT) {
        if (m->rowPosition == -1) {
            switchToolbarButtons();
        }
        else if (m->colPosition > 0) {
            move--;
            m->colPosition = m->colPosition - 1;
        }
        else if (m->colPosition == 0) {
            checkGotoPreviousPage(m, &move);
        }
    }
    if(hidKeysDown()&KEY_UP) {
        if (dPadNavigation && m->rowPosition == 0) {
            if (toolbarButtons.buttons[1]->visible && m->colPosition > 1) {
                dPadSelectedToolbarButton = 1;
                m->rowPosition = -1;
                m->selectedEntry = -1;
            }
            else {
                dPadSelectedToolbarButton = 0;
                m->rowPosition = -1;
                m->selectedEntry = -1;
            }

            return false;
        }
        
        else if (dPadNavigation && m->rowPosition == -1) {
            int last = indexOfLastVisibleMenuEntryOnPage(m->pagePosition, m);
            
            if (dPadSelectedToolbarButton == 2) {
                m->selectedEntry = last;
            }
            else {
                menuEntry_s *lastMenuEntry = getMenuEntry(m, last);
//                logInt(lastMenuEntry->col, "Col of last menu entry on page is");
                last -= lastMenuEntry->col;
                m->selectedEntry = last;
            }
            
            menuEntry_s *currentMenuEntry = getMenuEntry(m, m->selectedEntry);
            m->rowPosition = currentMenuEntry->row;
            m->colPosition = currentMenuEntry->col;
            
            dPadSelectedToolbarButton = -1;
            btnListUnHighlight(&toolbarButtons);
            return false;
        }
        
        else if (m->rowPosition > 0) {
            move-=4;
            m->rowPosition = m->rowPosition - 1;
        }
    }
    
    if(hidKeysDown()&KEY_DOWN) {
        if (dPadNavigation && m->rowPosition == -1) {
            if (dPadSelectedToolbarButton == 0) {
                m->selectedEntry = indexOfFirstVisibleMenuEntryOnPage(m->pagePosition, m);
            }
            else {
                int first = indexOfFirstVisibleMenuEntryOnPage(m->pagePosition, m);
                first += 3;
                if (first >= m->numEntries) {
                    first = m->numEntries-1;
                }
                
                m->selectedEntry = first;
            }
            
            menuEntry_s *currentMenuEntry = getMenuEntry(m, m->selectedEntry);
            m->rowPosition = currentMenuEntry->row;
            m->colPosition = currentMenuEntry->col;
            
            dPadSelectedToolbarButton = -1;
            btnListUnHighlight(&toolbarButtons);
            return false;
        }
        else if (dPadNavigation && (m->rowPosition == totalRows-1 || (m->selectedEntry + totalCols) >= m->numEntries)) {
            
            
            if (toolbarButtons.buttons[2]->visible && m->colPosition > 1) {
                dPadSelectedToolbarButton = 2;
                m->rowPosition = -1;
                m->selectedEntry = -1;
            }
            if (toolbarButtons.buttons[3]->visible && m->colPosition < 2) {
                dPadSelectedToolbarButton = 3;
                m->rowPosition = -1;
                m->selectedEntry = -1;
            }
            
            return false;
        }
        
        if (m->rowPosition < (totalRows - 1) && m->selectedEntry < (m->numEntries-4)) {
            move+=4;
            m->rowPosition = m->rowPosition + 1;
        }
    }
    
    if(hidKeysDown()&KEY_R) {
        checkGotoNextPage(m, &move);
    }
    
    if(hidKeysDown()&KEY_L) {
        checkGotoPreviousPage(m, &move);
    }
    
    u16 oldEntry=m->selectedEntry;
    
    if (hidKeysDown()&KEY_TOUCH) {
        m->firstTouch=touch;
        
        menuEntry_s* me=m->entries;
        int i=0;
        int newRow = 0;
        int newCol = 0;
        bool gotAppMatch = false;
        while(me) {
            if (me->page == m->pagePosition) {
                newRow = me->row;
                newCol = me->col;
                
                int iconX = me->iconX;
                int iconY = me->iconY;
                int iconW = me->iconW;
                int iconH = me->iconH;
                
                if (touchWithin(touchX, touchY, iconX, iconY, iconW, iconH)) {
                    gotAppMatch = true;
                    break;
                }
            }
            
            me=me->next;
            i++;
        }
        
        if (gotAppMatch) {
            btnListUnHighlight(&toolbarButtons);
            dPadSelectedToolbarButton = -1;
            
            if(m->selectedEntry==i) {
                return true;
            }
            
            else {
                m->selectedEntry=i;
                m->rowPosition = newRow;
                m->colPosition = newCol;
            }
        }
        else {
            if (btnTouchWithin(touchX, touchY, &pageArrowLeft)) {
                checkGotoPreviousPage(m, &move);
            }
            else if (btnTouchWithin(touchX, touchY, &pageArrowRight)) {
                checkGotoNextPage(m, &move);
            }
            
            btnListCheckHighlight(&toolbarButtons, touchX, touchY);
        }
    }
    else if(hidKeysHeld()&KEY_TOUCH){
        
        btnListCheckHighlight(&toolbarButtons, touchX, touchY);
        
        //condition to make sure previousTouch is valid
        cstick.dy+=(touch.py-m->previousTouch.py)*16;
        m->touchTimer++;
    }
    else if (hidKeysUp()&KEY_TOUCH) {
        btnListCheckHighlight(&toolbarButtons, touchX, touchY);
        btnListCheckRunCallback(&toolbarButtons, m->previousTouch.px, m->previousTouch.py);
    }
    
    if (move + m->selectedEntry < 0) {
        m->selectedEntry=indexOfFirstVisibleMenuEntry(m);
    }
    else if (move + m->selectedEntry >= m->numEntries) {
        m->selectedEntry=m->numEntries-1;
    }
    else {
        m->selectedEntry+=move;
    }
    
    if(m->selectedEntry!=oldEntry)m->atEquilibrium=false;
    
    if(hidKeysDown()&KEY_A) {
        /*
         We're about to launch a title from the A button
         
         We need to re-scan the buttons to clear the A button's down state.
         If we don't do this, then the first title on the title menu
         will launch straight away.
         
         */
        
        hidScanInput();
        
        if (m->rowPosition == -1) {
            handleDPadToolbarActions(m);
            return false;
        }
        
        return true;
    }
    
    m->previousTouch=touch;
    
    return false;
}

//void handleGridButtonTouches(menu_s *m, buttonList *aButtonList) {
//    touchPosition touch;
//    hidTouchRead(&touch);
//    touchX = touch.px;
//    touchY = touch.py;
//    
//    if (hidKeysDown()&KEY_TOUCH || hidKeysHeld()&KEY_TOUCH) {
//        btnListCheckHighlight(aButtonList, touchX, touchY);
//        m->previousTouch.px = touchX;
//        m->previousTouch.py = touchY;
//    }
//    else if (hidKeysUp()&KEY_TOUCH) {
//        btnListCheckHighlight(aButtonList, touchX, touchY);
//        btnListCheckRunCallback(aButtonList, m->previousTouch.px, m->previousTouch.py);
//    }
//}

#warning Try to get rid of this if possible
void handleNonGridToolbarNavigation() {
    btnListUnHighlight(&toolbarButtons);
    
    if (hidKeysDown()&KEY_UP && toolbarButtons.buttons[0]->visible == true) {
        if (dPadSelectedToolbarButton == -1) {
            dPadSelectedToolbarButton = 0;
        }
    }
    
    if (hidKeysDown()&KEY_DOWN) {
        dPadSelectedToolbarButton = -1;
    }
    
    if (hidKeysDown()&KEY_LEFT || hidKeysDown()&KEY_RIGHT) {
        if (dPadSelectedToolbarButton == 0 && toolbarButtons.buttons[1]->visible == true) {
            dPadSelectedToolbarButton = 1;
        }
        else if (dPadSelectedToolbarButton == 1 && toolbarButtons.buttons[0]->visible == true) {
            dPadSelectedToolbarButton = 0;
        }
    }
    
    if (hidKeysDown()&KEY_A) {
        if (dPadSelectedToolbarButton == 0) {
            hidScanInput();
            toolbarTopLeftAction();
        }
        else if (dPadSelectedToolbarButton == 1) {
            hidScanInput();
            toolbarTopRightAction();
        }
    }
    
    if (hidKeysDown()&KEY_B) {
        keyBAction();
    }
    
    if (dPadSelectedToolbarButton > -1) {
        toolbarButtons.buttons[dPadSelectedToolbarButton]->highlighted = true;
    }
    
    touchPosition touch;
    hidTouchRead(&touch);
    touchX = touch.px;
    touchY = touch.py;
    
    if (hidKeysDown()&KEY_TOUCH) {
        btnListCheckHighlight(&toolbarButtons, touchX, touchY);
    }
    
    else if(hidKeysHeld()&KEY_TOUCH){
        btnListCheckHighlight(&toolbarButtons, touchX, touchY);
    }
    
    else if (hidKeysUp()&KEY_TOUCH) {
        btnListCheckHighlight(&toolbarButtons, touchX, touchY);
        btnListCheckRunCallback(&toolbarButtons, menu.previousTouch.px, menu.previousTouch.py);
    }
    
    menu.previousTouch.px = touchX;
    menu.previousTouch.py = touchY;
}

bool updateMenu(menu_s* m) {
    if (menuStatus == menuStatusIcons) {
        if (menuForceReturnTrue) {
            menuForceReturnTrue = false;
            return true;
        }
        
        return updateGrid(m);
    }
    else if (menuStatus == menuStatusFolderChanged) {
//        logText("Reloading main menu");
        
        reloadMenu(m);
        gotoFirstIcon(m);
        setMenuStatus(menuStatusIcons);
        
//        logText("Done reloading");
        
        return false;
    }
    else {
        updateGrid(m);
        return false;
    }
}

void initEmptyMenuEntry(menuEntry_s* me)
{
	if(!me)return;

	me->name[0]=0x00;
	me->description[0]=0x00;
	me->executablePath[0]=0x00;
    
	initDescriptor(&me->descriptor);

	me->next=NULL;
}

void initMenuEntry(menuEntry_s* me, char* execPath, char* name, char* description, char* author, u8* iconData)
{
	if(!me)return;

	initEmptyMenuEntry(me);

	strncpy(me->executablePath, execPath, ENTRY_PATHLENGTH);
	strncpy(me->name, name, ENTRY_NAMELENGTH);
	strncpy(me->description, description, ENTRY_DESCLENGTH);
	strncpy(me->author, author, ENTRY_AUTHORLENGTH);
	if(iconData)memcpy(me->iconData, iconData, ENTRY_ICONSIZE);
    
	initDescriptor(&me->descriptor);
    
    me->title_id = 0;
}

int drawMenuEntry(menuEntry_s* me, gfxScreen_t screen, bool selected, menu_s *m) {
    if(!me) {
        return 0;
    }
    
	const int totalWidth=selected?ENTRY_WIDTH_SELECTED:ENTRY_WIDTH;
    
    int * coords = coordsForMenuEntry(me->row, me->col, m);
    int x = coords[0];
    int y = coords[1];
    
    me->iconX = x;
    me->iconY = y;
    me->iconW = ENTRY_ICON_WIDTH;
    me->iconH = ENTRY_ICON_HEIGHT;

    //Draw backgrounds for the icons if they haven't been drawn by drawMenu
    //(i.e. if showing app backgrounds for empty slots has been disabled)
    
    /*
     Draw the icon on the bottom screen (if necessary)
     */
    
    if (me != &gamecardMenuEntry && !me->isTitleEntry) {
        if (me->isRegionFreeEntry) {
            if (selected) {
                if (themeHasCartBackgroundImageSelected) {
                    drawThemeImage(themeImageCartBackgroundSelected, screen, x+3, y+4);
                }
                else {
                    gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, (u8*)cartBackgroundSelected, 59, 59, x+3, y+4);
                }
            }
            else {
                if (themeHasCartBackgroundImage) {
                    drawThemeImage(themeImageCartBackground, screen, x+3, y+4);
                }
                else {
                    gfxDrawSpriteAlphaBlendFade(screen, GFX_LEFT, (u8*)cartBackground, 59, 59, x+3, y+4, translucencyAppBackgrounds);
                }
            }
        }
        else {
            if (selected) {
                if (themeHasAppBackgroundImageSelected) {
                    drawThemeImage(themeImageAppBackgroundSelected, screen, x+3, y+4);
                }
                else {
                    gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, appBackgroundSelected, 56, 56, x+3, y+4);
                }
            }
            else {
                if (themeHasAppBackgroundImage) {
                    drawThemeImage(themeImageAppBackground, screen, x+3, y+4);
                }
                else {
                    gfxDrawSpriteAlphaBlendFade(screen, GFX_LEFT, appBackground, 56, 56, x+3, y+4, translucencyAppBackgrounds);
                }
                
            }
        }
        
        //Highlight the icon if it is selected
        
        
        //This is where the masked image will be put (rounding the corners)
        u8 transparentIcon[48*48*4];
        
        //Mask whichever icon is going to be drawn (either the game card icon or a homebrew app's icon
        if (me->isRegionFreeEntry && regionFreeGamecardIn) {
            MAGFXApplyAlphaMask(gamecardMenuEntry.iconData, (u8*)appiconalphamask_bin, transparentIcon, 48, 48, false);
        }
        else {
            MAGFXApplyAlphaMask(me->iconData, (u8*)appiconalphamask_bin, transparentIcon, 48, 48, false);
        }
        
        //Draw the icon on the bottom screen
        gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, transparentIcon, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, x+7, y+8);
        
        if (me->drawFirstLetterOfName) {
            char firstLetter[2];
            memcpy(firstLetter, &me->name, 1);
            firstLetter[1] = '\0';
            
            rgbColour * light = lightTextColour();
            
            MADrawText(screen, GFX_LEFT, x+9, y+27, firstLetter, &MAFontRobotoRegular16, light->r, light->g, light->b);
        }
    }
    
    /*
     Draw a tick on the icon if it is ticked (e.g. for settings)
     */
    if (me->showTick != NULL && *(me->showTick)) {
        gfxDrawSpriteAlphaBlend(screen, GFX_LEFT, tick, 48, 48, x+7, y+8);
    }
    
    /*
     Top screen stuff for the selected item
     */
    if (selected) {
        rgbColour * dark = darkTextColour();
        
        int top = 240-50-18;
        
        int xPos = 30;
                
        int displayIconX = (240-ENTRY_ICON_HEIGHT)/2;
        int displayIconY = 0;
        
        /*
         Draw the shadow
         */
        gfxDrawSpriteAlphaBlendFade(GFX_TOP, GFX_LEFT, (u8*)appshadow_bin, 27, 161, displayIconX-16, displayIconY-3, translucencyAppShadow);
        
        displayIconY += (156-ENTRY_ICON_HEIGHT)/2;
        
        /*
         Draw the app icon
         */
        u8 transparentIcon[48*48*4];
        
        if (me->isRegionFreeEntry && regionFreeGamecardIn) {
            MAGFXApplyAlphaMask(gamecardMenuEntry.iconData, (u8*)appiconalphamask_bin, transparentIcon, 48, 48, false);
        }
        else {
            MAGFXApplyAlphaMask(me->iconData, (u8*)appiconalphamask_bin, transparentIcon, 48, 48, false);
        }

        gfxDrawSpriteAlphaBlend(GFX_TOP, GFX_LEFT, transparentIcon, ENTRY_ICON_WIDTH, ENTRY_ICON_HEIGHT, displayIconX, displayIconY);
        
        xPos += (ENTRY_ICON_WIDTH*2)+30;
        
        int yAdjust = 20;
        
        int maximumTextWidth = 400-xPos-20;
        
        /*
         Draw the app title
         */
        
        rgbColour *titleColour = titleTextColour();
        
        int maxLines = (me->drawFullTitle) ? 100 : 1;
        
        if (me == &gamecardMenuEntry || (me->isRegionFreeEntry && regionFreeGamecardIn)) {
            MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, gamecardMenuEntry.name, &MAFontRobotoRegular14, titleColour->r, titleColour->g, titleColour->b, maximumTextWidth, maxLines);
        }
        else {
            MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, me->name, &MAFontRobotoRegular14, titleColour->r, titleColour->g, titleColour->b, maximumTextWidth, maxLines);
        }
        
        /*
         Draw the app author
         */
        
        if (me->isRegionFreeEntry && regionFreeGamecardIn) {
            yAdjust += 25;
            MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, gamecardMenuEntry.author, &MAFontRobotoRegular12, dark->r, dark->g, dark->b, maximumTextWidth, 1);
        }
        else if (strlen(me->author) > 0) {
            yAdjust += 25;
            MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, me->author, &MAFontRobotoRegular12, dark->r, dark->g, dark->b, maximumTextWidth, 1);
        }
        
        yAdjust += 25;
        
        /*
         Draw the app description
         */
        
        int descriptionMaxWidth = 400-xPos-20;
        int descriptionMaxLines = 4;
        
        if (me->isRegionFreeEntry && regionFreeGamecardIn) {
            MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, gamecardMenuEntry.description, &MAFontRobotoRegular10, dark->r, dark->g, dark->b, descriptionMaxWidth, descriptionMaxLines);
        }
        else {
            if (me->isRegionFreeEntry) {
                char * insert = "Insert a game cart to play region free";
                MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, insert, &MAFontRobotoRegular10, dark->r, dark->g, dark->b, descriptionMaxWidth, descriptionMaxLines);
            }
            else {
                MADrawTextWrap(GFX_TOP, GFX_LEFT, top-yAdjust, xPos, me->description, &MAFontRobotoRegular10, dark->r, dark->g, dark->b, descriptionMaxWidth, descriptionMaxLines);
            }
        }
        
        if (m == &titleMenu && displayTitleID) {
            char titleIDString[128];
            sprintf(titleIDString, "%llu", me->title_id);
            MADrawText(GFX_TOP, GFX_LEFT, 0, 0, titleIDString, &MAFontRobotoRegular10, dark->r, dark->g, dark->b);
        }
    }

	return totalWidth;
}
