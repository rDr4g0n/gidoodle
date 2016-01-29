// taken from https://bbs.archlinux.org/viewtopic.php?pid=660547#p660547
#include<stdio.h>
#include<stdlib.h>
#include<X11/Xlib.h>
#include<X11/cursorfont.h>

typedef struct {
    Display * disp;
    Screen * scr;
    Window root;
} drawingContext;

drawingContext * setupContext(){
  drawingContext * ctx = (drawingContext*)malloc(sizeof(drawingContext));

  ctx->disp = XOpenDisplay(NULL);
  if(!ctx->disp){
    return NULL;
  }

  ctx->scr = ScreenOfDisplay(ctx->disp, DefaultScreen(ctx->disp));
  ctx->root = RootWindow(ctx->disp, XScreenNumberOfScreen(ctx->scr));
  return ctx;
}

GC setupGC(drawingContext * ctx){
  XGCValues * gcval = (XGCValues*)malloc(sizeof(XGCValues));
  gcval->foreground = XWhitePixel(ctx->disp, 0);
  gcval->function = GXxor;
  gcval->background = XBlackPixel(ctx->disp, 0);
  gcval->plane_mask = gcval->background ^ gcval->foreground;
  gcval->subwindow_mode = IncludeInferiors;

  GC gc;
  gc = XCreateGC(ctx->disp, ctx->root,
    GCFunction | GCForeground | GCBackground | GCSubwindowMode,
    gcval);

  return gc;
}

int main(void)
{
  // TODO - move these to structs
  int rx = 0, ry = 0, rw = 0, rh = 0;
  int rect_x = 0, rect_y = 0, rect_w = 0, rect_h = 0;
  int btn_pressed = 0, done = 0;

  // setup window and all that fun stuff
  drawingContext * ctx = setupContext();
  if(ctx == NULL){
      printf("Could not setup drawing context");
      return EXIT_FAILURE;
  }

  GC gc = setupGC(ctx);

  XEvent ev;

  if ((XGrabPointer
       (ctx->disp, ctx->root, False,
        ButtonMotionMask | ButtonPressMask | ButtonReleaseMask, GrabModeAsync,
        GrabModeAsync, ctx->root,
        XCreateFontCursor(ctx->disp, XC_crosshair),
        CurrentTime) != GrabSuccess))
    printf("couldn't grab pointer:");

  while (!done) {
    while (!done && XPending(ctx->disp)) {
      XNextEvent(ctx->disp, &ev);
      switch (ev.type) {
        case MotionNotify:
        /* this case is purely for drawing rect on screen */
          if (btn_pressed) {
            if (rect_w) {
              /* re-draw the last rect to clear it */
              XDrawRectangle(ctx->disp, ctx->root, gc, rect_x, rect_y, rect_w, rect_h);
            }
            rect_x = rx;
            rect_y = ry;
            rect_w = ev.xmotion.x - rect_x;
            rect_h = ev.xmotion.y - rect_y;

            if (rect_w < 0) {
              rect_x += rect_w;
              rect_w = 0 - rect_w;
            }
            if (rect_h < 0) {
              rect_y += rect_h;
              rect_h = 0 - rect_h;
            }
            /* draw rectangle */
            XDrawRectangle(ctx->disp, ctx->root, gc, rect_x, rect_y, rect_w, rect_h);
            XFlush(ctx->disp);
          }
          break;
        case ButtonPress:
          btn_pressed = 1;
          rx = ev.xbutton.x;
          ry = ev.xbutton.y;
          break;
        case ButtonRelease:
          done = 1;
          break;
      }
    }
  }

  /* clear the drawn rectangle */
  if (rect_w) {
    XDrawRectangle(ctx->disp, ctx->root, gc, rect_x, rect_y, rect_w, rect_h);
    XFlush(ctx->disp);
  }
  rw = ev.xbutton.x - rx;
  rh = ev.xbutton.y - ry;

  /* cursor moves backwards */
  if (rw < 0) {
    rx += rw;
    rw = 0 - rw;
  }
  if (rh < 0) {
    ry += rh;
    rh = 0 - rh;
  }

  XCloseDisplay(ctx->disp);

  printf("%d,%d,%d,%d\n",rx,ry,rw,rh);

  return EXIT_SUCCESS;
}
