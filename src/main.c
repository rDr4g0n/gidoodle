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
#define MAX_FILTER_LENGTH 100

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
    char filters[200];
    int logFD;
    // TODO - ffmpeg path
} Config;

Config config;

int openLogFile(){
    config.logFD = open(config.logPath, O_WRONLY|O_CREAT, 0664);
    if(config.logFD < 0){
        printf("ERROR: couldnt make that file, even a little bit\n");
        return 1;
    }
    return 0;
}

int captureVideo(rect * r){
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
    if(DEBUG){
        prettyPrint(capture);
    }

    printf("Waiting %i seconds to begin recording...\n", config.startDelay);
    fflush(stdout);
    //sleep(config.startDelay);

    pid_t pid = fork();
    if(pid == -1){
        printf("ERROR: unable to fork for some raisin.\n");
        return 1;

    // child
    } else if(pid == 0){
        // wire stdout/stderr to file
        dup2(config.logFD, STDERR_FILENO);
        close(config.logFD);
        execv(capture->list[0], capture->list);
        _exit(1);
    }

    printf("Prex any key to stop gidoodlin'\n");
    debug("ffmpeg got pid %i", pid);
    fflush(stdout);
    // wait on user
    getchar();
    printf("Wait a tick...\n");
    kill(pid, SIGTERM);
    // TODO - handle errors
    int status;
    wait(&status);
    freeArgList(capture);

    return 0;
}

int generatePalette(){
    // TODO - make palette optimization optional
    // http://blog.pkh.me/p/21-high-quality-gif-with-ffmpeg.html
    ArgList * palette = createArgList();
    pushArg(palette, "/usr/bin/ffmpeg");
    pushArg(palette, "-i");
    pushFArg(palette, "%s", config.tempVid);
    pushArg(palette, "-vf");
    pushFArg(palette, "%s,palettegen", config.filters);
    pushArg(palette, "-y");
    pushFArg(palette, "%s", config.palette);
    endArgList(palette);
    if(DEBUG){
        prettyPrint(palette);
    }

    pid_t pid = fork();
    if(pid == -1){
        printf("ERROR: unable to fork for some raisin.\n");
        return 1;

    // child
    } else if(pid == 0){
        // TODO - write some newlines to log
        // wire stdout/stderr to file
        dup2(config.logFD, STDERR_FILENO);
        close(config.logFD);
        execv(palette->list[0], palette->list);
        _exit(1);
    }

    int status;
    wait(&status);

    freeArgList(palette);

    return status;
}

// TODO - make this into a generic execute command
int gifify(){
    ArgList * gifify = createArgList();
    pushArg(gifify, "/usr/bin/ffmpeg");
    pushArg(gifify, "-i");
    pushFArg(gifify, "%s", config.tempVid);
    pushArg(gifify, "-i");
    pushFArg(gifify, "%s", config.palette);
    pushArg(gifify, "-lavfi");
    pushFArg(gifify, "%s [x]; [x][1:v] paletteuse", config.filters);
    pushArg(gifify, "-y");
    pushFArg(gifify, "%s.gif", config.tempVid);
    endArgList(gifify);
    if(DEBUG){
        prettyPrint(gifify);
    }

    pid_t pid = fork();
    if(pid == -1){
        printf("ERROR: unable to fork for some raisin.\n");
        return 1;

    // child
    } else if(pid == 0){
        // TODO - write some newlines to log
        // wire stdout/stderr to file
        // TODO - grab stdout?
        dup2(config.logFD, STDERR_FILENO);
        close(config.logFD);
        execv(gifify->list[0], gifify->list);
        _exit(1);
    }

    int status;
    wait(&status);

    freeArgList(gifify);

    return status;
}

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

    snprintf(config.filters, MAX_FILTER_LENGTH,
        "fps=%i,scale=iw*%i:ih*%i:flags=lanczos",
        config.fps, config.scale, config.scale);

    rect * r = getBoundingBox();

    if(r == NULL){
        printf("ERROR: couldn't get bounding box\n");
        return EXIT_FAILURE;
    }

    debug("got rect x:%i, y:%i, w:%i, h:%i\n", r->x, r->y, r->w, r->h);

    int retval;
    retval = openLogFile();
    if(retval != 0){
        printf("ERROR: couldn't open log file\n");
        return EXIT_FAILURE;
    }

    retval = captureVideo(r);
    if(retval != 0){
        printf("ERROR: failed while capturing video\n");
        return EXIT_FAILURE;
    }

    retval = generatePalette();
    if(retval != 0){
        printf("ERROR: failed to generate palette\n");
        return EXIT_FAILURE;
    }

    retval = gifify();
    if(retval != 0){
        printf("ERROR: failed to convert to gif\n");
        return EXIT_FAILURE;
    }

    // TODO - copy gif to output location

    // TODO - filename, duration, size, resolution, etc
    printf("Get ur gif at %s.gif\n", config.tempVid);

    // TODO - clean up temp files

    return EXIT_SUCCESS;
}
