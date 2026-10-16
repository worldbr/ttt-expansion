#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "toggle.h"
#include "overlay.h"

static void test_toggle(void) {
    TTTToggle t;
    ttt_toggle_init(&t);
    assert(ttt_toggle_frame(&t,0x100|0x200|0x1000)==0); /* Triangle wrong. */
    assert(ttt_toggle_frame(&t,0x100|0x200)==0);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==1 && t.open==1);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==0 && t.open==1);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==0);
    assert(ttt_toggle_frame(&t,0)==0);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==1 && t.open==0);
}
static void test_framebuffer(void) {
    const int stride = 512, height = 272, guard = 64;
    for(int fmt=0;fmt<4;fmt++) {
       int depth = fmt==3 ? 4 : 2;
       size_t size = (size_t)stride*height*depth;
       unsigned char *buf=malloc(size+2*guard);
       assert(buf);
       memset(buf,0xA5,size+2*guard);
       ttt_draw_overlay(buf+guard,stride,fmt,1);
       for(int i=0;i<guard;i++) {
           assert(buf[i]==0xA5);
           assert(buf[guard+size+i]==0xA5);
       }
       /* topmost row remains untouched; diagnostic pixel changed */
       assert(buf[guard]==0xA5);
       assert(buf[guard+((size_t)250*stride+8)*depth]!=0xA5);
       free(buf);
    }
}
int main(void) {test_toggle();test_framebuffer();puts("OK: controles e overlay (4 formatos) testados no PC");return 0;}
