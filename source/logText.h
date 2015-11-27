#include <3ds.h>

/**
* Renames previous log to *.lastlaunch and opens a new file.
*/
void logInit();

/**
* Creates a string in the logfile with a new line character appended to it.
*/
void logTextBootln(char *text);

/**
* Creates a string in the logfile.
*/
void logTextBoot(char *text);

/**
* Cleans resources.
*/
void logQuit();

void logTextP(char *text, char const * path, bool append);
void logText(char *text);
void logInt(int i, char * label);
void logIntP(int i, char * label, char const * path);
