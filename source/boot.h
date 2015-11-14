#ifndef BOOT_H
#define BOOT_H

#include <3ds.h>
#include "scanner.h"
#include "titles.h"

extern int targetProcessId;
extern titleInfo_s target_title;
bool hansTitleBoot;

bool isNinjhax2(void);
//int bootApp(char* executablePath, executableMetadata_s* em);
int bootApp(char* executablePath, executableMetadata_s* em, char* arg);

#endif
