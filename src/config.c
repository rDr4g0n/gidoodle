#include <stdbool.h>
#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include "config.h"
#include "debug.h"

const char *argp_program_version = "0.1.1alpha";
const char *argp_program_bug_address = "jmatos@zenoss.com";

static int timestamp(){
    return (unsigned)time(NULL);
}

int parse_opt(int key, char * arg, struct argp_state * state){
    if(arg != NULL){
        debug("key: %c(%i), arg: %s\n", key, key, arg);
    }
    struct Config * config = state->input;

    switch(key){
        // output file
        case 'o':;
            char absolutePath[MAX_PATH_LENGTH];
            realpath(arg, absolutePath);
            strcpy(config->outputPath, absolutePath);
            break;

        // scale
        case 's':
            sscanf(arg, "%f", &config->scale);
            break;
        // start delay
        case 'd':
            sscanf(arg, "%i", &config->startDelay);
            break;
        // debug messages
        case 'v':
            DEBUG = true;
            break;
        // preserve-temp
        case 128:
            config->preserveTemp = true;
            break;
        // fps
        case 129:
            sscanf(arg, "%i", &config->fps);
            break;
        // screenshot
        case 130:
            config->screenshot = true;
            break;

        case ARGP_KEY_ARG:
            // shorthand for output file
            // TODO - this probably aint the right
            // way to gather remaining args
            return parse_opt('o', arg, state);
        case ARGP_KEY_END:
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

Config * buildConfig(int argc, char **argv){
    Config * config = malloc(sizeof(Config));

    // default configs
    strcpy(config->outputPath, "/home/john/gidoos/mygif.gif");
    config->fps = 25;
    config->scale = 1;
    config->startDelay = 0;
    config->preserveTemp = true;
    config->screenshot = false;
    // TODO - let user specify capture rectangle via cli
    // TODO - upload gif to places

    // apply cli opts
    // NOTE - overrides defaults
    static struct argp_option options[] = {
        // TODO - grab output as the last arg with no flag needed
        {"output", 'o', "OUTFILE", 0, "Output file path and name", 0},
        {"scale", 's', "SCALE", 0, "Apply scaling to gif, 0 - 1 (default 1)", 0},
        {"delay", 'd', "DELAY", 0, "Delay in seconds before recording begins (default 0)", 0},
        {"verbose", 'v', 0, 0, "Enable debug messages", 0},
        {"preserve-temp", 128, 0, 0, "Preserve temp files (default false)", 0},
        {"screenshot", 130, 0, 0, "Take a single screenshot (default false)", 0},
        {"fps", 129, "FPS", 0, "Capture and output framerate (default 30)", 0},
        {0}
    };
    static struct argp argp = {options, parse_opt, "outfile.gif", "Draw a rectangle, do stuff, get a gif!", 0, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, config);

    // calculated configy stuff
    // NOTE - overrides defaults and CLI configs
    strcpy(config->tempDir, "/tmp");
    strcpy(config->ffmpegpath, "/usr/bin/ffmpeg");
    snprintf(config->id, MAX_ID_LENGTH, "%i", timestamp());
    // tmp video
    strcpy(config->vidpath, config->tempDir);
    strcat(config->vidpath, "/");
    strcat(config->vidpath, config->id);
    if(config->screenshot){
        strcat(config->vidpath, ".png");
    } else {
        strcat(config->vidpath, ".mkv");
    }
    // tmp log
    // TODO - append to a running log of all jobs?
    strcpy(config->logPath, config->tempDir);
    strcat(config->logPath, "/");
    strcat(config->logPath, config->id);
    strcat(config->logPath, ".log");
    // tmp palette
    strcpy(config->palette, config->tempDir);
    strcat(config->palette, "/");
    strcat(config->palette, config->id);
    strcat(config->palette, ".png");
    // filter string
    snprintf(config->filters, MAX_FILTER_LENGTH,
        "fps=%i,scale=iw*%f:ih*%f:flags=lanczos",
        config->fps, config->scale, config->scale);

    return config;
}

