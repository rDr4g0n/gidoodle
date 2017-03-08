#ifndef GIDOO_H
#define GIDOO_H

#include "mousebox.h"

int captureVideo(char * ffmpegpath, rect * r, int fps, char * vidpath, int startDelay, int logFD);
int captureScreenshot(char * ffmpegpath, rect * r, char * sspath, int logFD);
int generatePalette(char * ffmpegpath, char * vidpath, char * filters, char * palette, int logFD);
int gifify(char * ffmpegpath, char * vidpath, char * palette, char * filters, int logFD);

#endif
