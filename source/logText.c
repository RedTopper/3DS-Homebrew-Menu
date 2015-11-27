#include "logText.h"
#include <stdio.h>
#include <string.h>

//#include <cstdio>
//#include <cstdlib>
//#include <cstring>
//#include <map>
//#include <string>
//#include <vector>

FILE* fSave;

char* logpath = "/bootgrid.log";

void logInit() {
	char old[128]; //plenty long
	sprintf(old,"%s.lastlaunch",logpath); 
	remove(old);
	rename(logpath, old);
	FILE* logfile = fopen( logpath, "w" );
	fclose(logfile); //quickly clears the new file.
	fSave = fopen( logpath, "a" );
	logTextBootln("****************BEGIN LOG************************");
	logTextBootln("Done initializing file system!");
}

void logTextBootln(char *text) {
    if (fSave != NULL) {
        fprintf(fSave, "%s\n", text);
    }
}

void logTextBoot(char *text) {
    if (fSave != NULL) {
        fprintf(fSave, "%s", text);
    }
}

void logQuit() {
	logTextBootln("******************END LOG************************");
	logTextBootln("Masher's log closed cleanly.");
	fclose(fSave);
}

#warning Depricated, Try to use logTextBoot or LogTextBootln instead.
void logTextP(char *text, char const * path, bool append) {
    char * mode;

    if (append)
        mode = "a";
    else
        mode = "w";

    FILE* fSave = fopen( path, mode );
    if (fSave != NULL) {
        fprintf(fSave, "%s\n", text);
    }
    fclose(fSave);

    /*
    FILE* fLoad = fopen( LOG_PATH, "r" );

    if (fLoad != NULL) {
        fseek(fLoad, 0L, SEEK_END);
        int currentSize = ftell(fLoad);
        fseek(fLoad, 0L, SEEK_SET);

        char currentLog[currentSize];
        fgets(currentLog, currentSize, fLoad);

        int newSize = sizeof(text);
        char newLog[currentSize + newSize + 2];

        strcpy(newLog, currentLog);
//        strcat(newLog, currentLog);
        strcat(newLog, "\n");
        strcat(newLog, text);

        fclose(fLoad);
        write(newLog);
    }
    else {
        fclose(fLoad);
        write(text);
    }
     */
}

//void logU8(uint8_t i, char * label) {
//    char s[strlen(label)+8];
//    sprintf(s, "%s: %u", label, i);
//    logText(s);
//}

void logIntP(int i, char * label, char const * path) {
    char s[strlen(label)+8];
    sprintf(s, "%s: %d", label, i);
    logTextP(s, path, true);
}

void logInt(int i, char * label) {
    char s[strlen(label)+8];
    sprintf(s, "%s: %d", label, i);
    logText(s);
}

void logText(char *text) {
    char const * path = "sdmc:/log.txt";
    logTextP(text, path, true);
}
