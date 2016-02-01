#ifndef MOUSEBOX_H
#define MOUSEBOX_H

typedef struct rect{
    int x;
    int y;
    int h;
    int w;
} rect;

rect * getBoundingBox();

#endif
