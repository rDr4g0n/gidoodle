#define _POSIX_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include "debug.h"
#include "gidoo.h"
#include "arglist.h"
#include "mousebox.h"

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
    pushArg(cmd, "-hide_banner");
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

int captureScreenshot(char * ffmpegpath, rect * r, char * sspath, int startDelay, int logFD) {
    ArgList * cmd = createArgList();

    pushArg(cmd, ffmpegpath);
    //pushArg(cmd, "-framerate");
    //pushFArg(cmd, "%i", fps);
    pushArg(cmd, "-t");
    pushFArg(cmd, "%f", 0.1);
    pushArg(cmd, "-video_size");
    pushFArg(cmd, "%ix%i", r->w, r->h);
    pushArg(cmd, "-f");
    pushArg(cmd, "x11grab");
    pushArg(cmd, "-i");
    pushFArg(cmd, ":0.0+%i,%i", r->x, r->y);
    pushArg(cmd, "-y");
    pushArg(cmd, "-hide_banner");
    pushArg(cmd, "-vframes");
    pushArg(cmd, "1");
    pushFArg(cmd, "%s", sspath);
    endArgList(cmd);
    if(DEBUG){
        prettyPrint(cmd);
    }

    if(startDelay > 0){
        printf("Waiting %i seconds to take screenshot...\n", startDelay);
        fflush(stdout);
        sleep(startDelay);
    }

    doAThing(cmd, logFD);

    int status;
    wait(&status);

    freeArgList(cmd);

    return status;
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
    pushArg(cmd, "-hide_banner");
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
    pushArg(cmd, "-hide_banner");
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
