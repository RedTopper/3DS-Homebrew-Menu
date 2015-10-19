#ifndef LOGTEXT_H
#define LOGTEXT_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
    
    void logTextP(char *text, char * path);
    void logText(char *text);
    void logInt(int i, char * label);
    void logU8(uint8_t i, char * label);
    
#ifdef __cplusplus
}
#endif

#endif
