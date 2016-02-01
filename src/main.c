#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "mousebox.h"

// TODO - get config from args and/or prefs file
// TODO - use actual vars, not macros
#define TEMP_DIR "~/tmp"
#define OUTPUT_DIR "~/tmp"
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

    // TODO - use ffmpeg's c lib instead of popen
    // build the ffmpeg command
    char *capture = malloc(maxCommandLength);
    snprintf(capture, maxCommandLength, "\
ffmpeg \
-framerate %i \
-video_size %ix%i \
-f x11grab \
-i :0.0+%i,%i \
-an \
-vcodec libx264 -crf 0 -preset ultrafast \
%s",
    FPS, r->w, r->h, r->x, r->y, TEMP_VID);

    printf("capture command\n %s\n", capture);

    printf("Waiting %i seconds to being recording...\n", START_DELAY);
    fflush(stdout);
    sleep(START_DELAY);

    FILE * captureFile;
    captureFile = popen(capture, "w");
    // TODO - handle ffmpeg waiting on user input
    // in some cases
    // TODO - hide stdout/stderr

    printf("Prex any key to stop gidoodlin'\n");
    fflush(stdout);
    // wait on user
    getchar();

    // send quit to ffmpeg
    fprintf(captureFile, "q");
    // TODO - ensure process ended

    int captureRet = pclose(captureFile);
    if(captureRet != 0){
        printf("ERROR: problem stopping ffmpeg\n");
        return EXIT_FAILURE;
    }

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
