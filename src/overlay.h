#ifndef TTT_OVERLAY_H
#define TTT_OVERLAY_H
/* PSP pixel formats are 0 RGB565, 1 RGBA5551, 2 RGBA4444, 3 RGBA8888. */
void ttt_draw_overlay(void *pixels, int stride, int pixel_format, int open);
#endif
