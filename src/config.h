#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

#define MAX_PATH_LENGTH 1024
#define MAX_ID_LENGTH 25
#define MAX_FILTER_LENGTH 100

typedef struct Config {
    // for argp
    char * args[2];

    char tempDir[MAX_PATH_LENGTH];
    char outputPath[MAX_PATH_LENGTH];
    int fps;
    float scale;
    int startDelay;
    bool preserveTemp;

    bool screenshot;

    char id[MAX_ID_LENGTH];
    char vidpath[MAX_PATH_LENGTH];
    char logPath[MAX_PATH_LENGTH];
    char palette[MAX_PATH_LENGTH];
    char filters[200];
    char ffmpegpath[MAX_PATH_LENGTH];
    // TODO - skip palette optimization?
} Config;

Config * buildConfig(int argc, char **argv);

#endif
