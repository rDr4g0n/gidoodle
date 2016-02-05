#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>

#include "arglist.h"
#include "mousebox.h"

#define DEBUG 0

// for constructing ffmpeg commands
const int maxCommandLength = 2000;

void debug(const char *fmt, ...) {
    if(DEBUG){
        va_list args;
        va_start(args, fmt);
        vprintf(fmt, args);
        va_end(args);
    }
}

int timestamp(){
    return (unsigned)time(NULL);
}

#define MAX_PATH_LENGTH 1024
#define MAX_ID_LENGTH 25

typedef struct Config {
    char tempDir[MAX_PATH_LENGTH];
    char outputDir[MAX_PATH_LENGTH];
    int fps;
    char id[MAX_ID_LENGTH];
    char tempVid[MAX_PATH_LENGTH];
    char logPath[MAX_PATH_LENGTH];
    char palette[MAX_PATH_LENGTH];
    int scale;
    int startDelay;
    bool deleteTemp;
} Config;

Config config;

int main(){
    printf("Draw a rectangle over the area you want to capture\n");

    strcpy(config.tempDir, "/home/jay/tmp");
    strcpy(config.outputDir, "/home/jay/tmp");
    config.fps = 25;

    snprintf(config.id, MAX_ID_LENGTH, "%i", timestamp());

    strcpy(config.tempVid, config.tempDir);
    strcat(config.tempVid, "/");
    strcat(config.tempVid, config.id);
    strcat(config.tempVid, ".mkv");

    strcpy(config.logPath, config.tempDir);
    strcat(config.logPath, "/");
    strcat(config.logPath, config.id);
    strcat(config.logPath, ".log");

    strcpy(config.palette, config.tempDir);
    strcat(config.palette, "/");
    strcat(config.palette, config.id);
    strcat(config.palette, ".png");

    config.scale = 1;
    config.startDelay = 2;
    config.deleteTemp = false;

    rect * r = getBoundingBox();

    if(r == NULL){
        printf("ERROR: couldn't get bounding box\n");
        return EXIT_FAILURE;
    }

    debug("got rect x:%i, y:%i, w:%i, h:%i\n", r->x, r->y, r->w, r->h);

    ArgList * capture = createArgList();
    pushArg(capture, "/usr/bin/ffmpeg");
    pushArg(capture, "-framerate");
    pushFArg(capture, "%i", config.fps);
    pushArg(capture, "-video_size");
    pushFArg(capture, "%ix%i", r->w, r->h);
    pushArg(capture, "-f");
    pushArg(capture, "x11grab");
    pushArg(capture, "-i");
    pushFArg(capture, ":0.0+%i,%i", r->x, r->y);
    pushArg(capture, "-an");
    pushArg(capture, "-vcodec");
    pushArg(capture, "libx264");
    pushArg(capture, "-crf");

    pushArg(capture, "0");
    pushArg(capture, "-preset");
    pushArg(capture, "ultrafast");
    pushFArg(capture, "%s", config.tempVid);
    endArgList(capture);

    printf("Waiting %i seconds to begin recording...\n", config.startDelay);
    fflush(stdout);
    sleep(config.startDelay);
    
    int fd = open(config.logPath, O_WRONLY|O_CREAT, 0664);
    if(fd < 0){
        printf("ERROR: couldnt make that file, even a little bit\n");
        return EXIT_FAILURE;
    }

    // fork time guys
    pid_t pid = fork();
    if(pid == -1){
        printf("ERROR: unable to fork for some raisin.\n");
        return EXIT_FAILURE;

    // child
    } else if(pid == 0){
        // wire stdout/stderr to file
        dup2(fd, STDERR_FILENO);
        close(fd);
        execv(capture->list[0], capture->list);
        _exit(EXIT_FAILURE);
    }

    printf("Prex any key to stop gidoodlin'\n");
    debug("ffmpeg got pid %i", pid);
    fflush(stdout);
    // wait on user
    getchar();
    kill(pid, SIGTERM);
    int status;
    wait(&status);
    freeArgList(capture);

    char *filters = malloc(maxCommandLength);
    snprintf(filters, maxCommandLength, "fps=%i,scale=iw*%i:ih*%i:flags=lanczos",
    config.fps, config.scale, config.scale);

    // http://blog.pkh.me/p/21-high-quality-gif-with-ffmpeg.html
    // TODO - make palette optimization optional
    char *palette = malloc(maxCommandLength);
    snprintf(palette, maxCommandLength, "\
ffmpeg -i %s \
-vf '%s,palettegen' \
-y %s",
    config.tempVid, filters, config.palette);

    debug("palette command\n %s\n", palette);

    int paletteRet = system(palette);
    if(paletteRet != 0){
        printf("ERROR: palette generation returned %i\n", paletteRet);
        return EXIT_FAILURE;
    }

    free(palette);

    char *gifify = malloc(maxCommandLength);
    snprintf(gifify, maxCommandLength, "\
ffmpeg -i %s -i %s \
-lavfi '%s [x]; [x][1:v] paletteuse' \
-y %s.gif",
    config.tempVid, config.palette, filters, config.tempVid);

    debug("gifify command\n %s\n", gifify);

    int gififyRet = system(gifify);
    if(gififyRet != 0){
        printf("ERROR: gifification returned %i\n", gififyRet);
        return EXIT_FAILURE;
    }

    free(gifify);
    free(filters);

    // TODO - filename, duration, size, resolution, etc
    printf("Did it!\n");

    // TODO - clean up temp files

    return EXIT_SUCCESS;
}
