#define _POSIX_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdarg.h>

#include "arglist.h"
#include "mousebox.h"

// TODO - get config from args and/or prefs file
// TODO - use actual vars, not macros
#define TEMP_DIR "/home/jay/tmp"
#define OUTPUT_DIR "/home/jay/tmp"
#define FPS 25
#define ID "1234"
#define TEMP_VID TEMP_DIR "/temp_" ID ".mkv"
#define LOG TEMP_DIR "/log_" ID ".log"
#define PALETTE TEMP_DIR "/palette_" ID ".png"
#define SCALE 1
#define START_DELAY 2
#define DELETE_TEMP 1

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

int main(){
    printf("Draw a rectangle over the area you want to capture\n");

    rect * r = getBoundingBox();

    if(r == NULL){
        printf("ERROR: couldn't get bounding box\n");
        return EXIT_FAILURE;
    }

    debug("got rect x:%i, y:%i, w:%i, h:%i\n", r->x, r->y, r->w, r->h);

    ArgList * capture = createArgList();
    pushArg(capture, "/usr/bin/ffmpeg");
    pushArg(capture, "-framerate");
    pushFArg(capture, "%i", FPS);
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
    pushFArg(capture, "%s", TEMP_VID);
    endArgList(capture);
    prettyPrint(capture);

    //printf("Waiting %i seconds to begin recording...\n", START_DELAY);
    //fflush(stdout);
    //sleep(START_DELAY);
    
    int fd = open("/home/jay/tmp/out.log", O_WRONLY|O_CREAT);
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
        printf("horses");
        dup2(fd, STDERR_FILENO);
        close(fd);

        printf("starting ffmpeg\n");
        execv(capture->list[0], capture->list);
        printf("aww crap");
        _exit(EXIT_FAILURE);
    }

    printf("Prex any key to stop gidoodlin'\n");
    fflush(stdout);
    // wait on user
    getchar();
    printf("killin %i\n", pid);
    kill(pid, SIGTERM);
    int status;
    wait(&status);
    printf("pid %i returned status %i\n", pid, status);
    freeArgList(capture);

    char *filters = malloc(maxCommandLength);
    snprintf(filters, maxCommandLength, "fps=%i,scale=iw*%i:ih*%i:flags=lanczos",
    FPS, SCALE, SCALE);

    // http://blog.pkh.me/p/21-high-quality-gif-with-ffmpeg.html
    // TODO - make palette optimization optional
    char *palette = malloc(maxCommandLength);
    snprintf(palette, maxCommandLength, "\
ffmpeg -i %s \
-vf '%s,palettegen' \
-y %s",
    TEMP_VID, filters, PALETTE);

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
    TEMP_VID, PALETTE, filters, TEMP_VID);

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
