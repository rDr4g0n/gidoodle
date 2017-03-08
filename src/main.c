#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"
#include "config.h"
#include "mousebox.h"
#include "gidoo.h"

bool DEBUG = true;

int openLogFile(char * filePath){
    int fd = open(filePath, O_WRONLY|O_CREAT, 0664);
    if(fd < 0){
        printf("ERROR: couldnt make that file, even a little bit\n");
        return -1;
    }
    return fd;
}

int main(int argc, char **argv){
    Config * config = buildConfig(argc, argv);

    debug("Config\n outputPath: '%s'\n, fps: %i\n, scale: %f\n, startDelay: %i\n, verbose: %i\n",
            config->outputPath, config->fps, config->scale, config->startDelay, DEBUG);

    // make sure ffmpeg exists and stuff
    char ffmpegTestCommand[MAX_PATH_LENGTH];
    strcpy(ffmpegTestCommand, config->ffmpegpath);
    strcat(ffmpegTestCommand, " -version");
    strcat(ffmpegTestCommand, " >/dev/null");
    if(system(ffmpegTestCommand)){
        printf("ERROR: couldn't find ffmpeg at '%s'\n", config->ffmpegpath);
        return EXIT_FAILURE;
    }

    printf("Draw a rectangle over the area you want to capture\nPress [Escape] to cancel\n");
    rect * r = getBoundingBox();
    if(r == NULL){
        printf("ERROR: couldn't get bounding box\n");
        return EXIT_FAILURE;
    }
    debug("got rect x:%i, y:%i, w:%i, h:%i\n", r->x, r->y, r->w, r->h);

    int fd = openLogFile(config->logPath);
    if(fd == -1){
        printf("ERROR: couldn't open log file\n");
        return EXIT_FAILURE;
    }

    if(config->screenshot){
        int retval;
        retval = captureScreenshot(config->ffmpegpath, r, config->vidpath, fd);
        if(retval != 0){
            printf("ERROR: failed while capturing screenshot\n");
            return EXIT_FAILURE;
        }

        // copy gif to output location
        char gifPath[MAX_PATH_LENGTH];
        // TODO - dont automatically overwrite?
        // TODO - verify if path is valid before recording? (fail early)
        strcpy(gifPath, config->vidpath);
        rename(gifPath, config->outputPath);

        // TODO - filename, duration, size, resolution, etc
        printf("Get ur png at %s\n", config->outputPath);

        close(fd);
        return EXIT_SUCCESS;
    }

    int retval;
    retval = captureVideo(config->ffmpegpath, r, config->fps, config->vidpath, config->startDelay, fd);
    if(retval != 0){
        printf("ERROR: failed while capturing video\n");
        return EXIT_FAILURE;
    }

    retval = generatePalette(config->ffmpegpath, config->vidpath, config->filters, config->palette, fd);
    if(retval != 0){
        printf("ERROR: failed to generate palette\n");
        return EXIT_FAILURE;
    }

    retval = gifify(config->ffmpegpath, config->vidpath, config->palette, config->filters, fd);
    if(retval != 0){
        printf("ERROR: failed to convert to gif\n");
        return EXIT_FAILURE;
    }

    // copy gif to output location
    char gifPath[MAX_PATH_LENGTH];
    // TODO - dont automatically overwrite?
    // TODO - verify if path is valid before recording? (fail early)
    strcpy(gifPath, config->vidpath);
    strcat(gifPath, ".gif");
    rename(gifPath, config->outputPath);

    // TODO - filename, duration, size, resolution, etc
    printf("Get ur gif at %s\n", config->outputPath);

    close(fd);

    return EXIT_SUCCESS;
}
