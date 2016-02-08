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
    char outputPath[MAX_PATH_LENGTH];
    int fps;
    float scale;
    int startDelay;
    bool preserveTemp;

    char id[MAX_ID_LENGTH];
    char vidpath[MAX_PATH_LENGTH];
    char logPath[MAX_PATH_LENGTH];
    char palette[MAX_PATH_LENGTH];
    char filters[200];
    char ffmpegpath[MAX_PATH_LENGTH];
    // TODO - skip palette optimization?
} Config;

Config * buildConfig(){
    Config * config = malloc(sizeof(Config));

    // TODO - take command line stuff
    // temp dir, output file, fps, scale, start delay, preserve temp
    // TODO - let user specify capture rectangle via cli
    strcpy(config->tempDir, "/home/jay/tmp");
    strcpy(config->outputPath, "/home/jay/tmp/mygif.gif");
    // TODO - get ffmpeg path from os
    strcpy(config->ffmpegpath, "/usr/bin/ffmpeg");
    config->fps = 25;

    snprintf(config->id, MAX_ID_LENGTH, "%i", timestamp());

    strcpy(config->vidpath, config->tempDir);
    strcat(config->vidpath, "/");
    strcat(config->vidpath, config->id);
    strcat(config->vidpath, ".mkv");

    strcpy(config->logPath, config->tempDir);
    strcat(config->logPath, "/");
    strcat(config->logPath, config->id);
    strcat(config->logPath, ".log");

    strcpy(config->palette, config->tempDir);
    strcat(config->palette, "/");
    strcat(config->palette, config->id);
    strcat(config->palette, ".png");

    config->scale = 1;
    config->startDelay = 0;
    config->preserveTemp = true;

    snprintf(config->filters, MAX_FILTER_LENGTH,
        "fps=%i,scale=iw*%f:ih*%f:flags=lanczos",
        config->fps, config->scale, config->scale);

    return config;
}

int openLogFile(char * filePath){
    int fd = open(filePath, O_WRONLY|O_CREAT, 0664);
    if(fd < 0){
        printf("ERROR: couldnt make that file, even a little bit\n");
        return -1;
    }
    return fd;
}

// a terribly named function that forks
// and execl's the passed in command
pid_t doAThing(ArgList * command, int fd){
    pid_t pid = fork();
    if(pid == -1){
        printf("ERROR: unable to fork for some raisin.\n");
        return 1;

    // child
    } else if(pid == 0){
        // wire stdout/stderr to file
        dup2(fd, STDERR_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        execv(command->list[0], command->list);
        _exit(1);
    }

    // parent will return the child pid
    return pid;
}

int captureVideo(char * ffmpegpath, rect * r, int fps, char * vidpath, int startDelay, int logFD){
    ArgList * cmd = createArgList();
    pushArg(cmd, ffmpegpath);
    pushArg(cmd, "-framerate");
    pushFArg(cmd, "%i", fps);
    pushArg(cmd, "-video_size");
    pushFArg(cmd, "%ix%i", r->w, r->h);
    pushArg(cmd, "-f");
    pushArg(cmd, "x11grab");
    pushArg(cmd, "-i");
    pushFArg(cmd, ":0.0+%i,%i", r->x, r->y);
    pushArg(cmd, "-an");
    pushArg(cmd, "-vcodec");
    pushArg(cmd, "libx264");
    pushArg(cmd, "-crf");
    pushArg(cmd, "0");
    pushArg(cmd, "-preset");
    pushArg(cmd, "ultrafast");
    pushArg(cmd, "-y");
    pushFArg(cmd, "%s", vidpath);
    endArgList(cmd);
    if(DEBUG){
        prettyPrint(cmd);
    }

    if(startDelay > 0){
        printf("Waiting %i seconds to begin recording...\n", startDelay);
        fflush(stdout);
        sleep(startDelay);
    }

    pid_t pid = doAThing(cmd, logFD);
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
    freeArgList(cmd);

    return 0;
}

int generatePalette(char * ffmpegpath, char * vidpath, char * filters, char * palette, int logFD){
    // http://blog.pkh.me/p/21-high-quality-gif-with-ffmpeg.html
    ArgList * cmd = createArgList();
    pushArg(cmd, ffmpegpath);
    pushArg(cmd, "-i");
    pushFArg(cmd, "%s", vidpath);
    pushArg(cmd, "-vf");
    pushFArg(cmd, "%s,palettegen", filters);
    pushArg(cmd, "-y");
    pushFArg(cmd, "%s", palette);
    endArgList(cmd);
    if(DEBUG){
        prettyPrint(cmd);
    }

    doAThing(cmd, logFD);

    int status;
    wait(&status);

    freeArgList(cmd);

    return status;
}

int gifify(char * ffmpegpath, char * vidpath, char * palette, char * filters, int logFD){
    ArgList * cmd = createArgList();
    pushArg(cmd, ffmpegpath);
    pushArg(cmd, "-i");
    pushFArg(cmd, "%s", vidpath);
    pushArg(cmd, "-i");
    pushFArg(cmd, "%s", palette);
    pushArg(cmd, "-lavfi");
    pushFArg(cmd, "%s [x]; [x][1:v] paletteuse", filters);
    pushArg(cmd, "-y");
    pushFArg(cmd, "%s.gif", vidpath);
    endArgList(cmd);
    if(DEBUG){
        prettyPrint(cmd);
    }

    doAThing(cmd, logFD);

    int status;
    wait(&status);

    freeArgList(cmd);

    return status;
}

int main(){
    Config * config = buildConfig();

    printf("Draw a rectangle over the area you want to capture\n");
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
    strcpy(gifPath, config->vidpath);
    strcat(gifPath, ".gif");
    rename(gifPath, config->outputPath);

    // TODO - filename, duration, size, resolution, etc
    printf("Get ur gif at %s\n", config->outputPath);

    close(fd);

    return EXIT_SUCCESS;
}
