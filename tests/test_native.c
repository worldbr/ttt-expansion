#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "toggle.h"
#include "overlay.h"
#include "framecheck.h"

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
    assert(ttt_toggle_frame(&t,0)==0);
    assert(ttt_toggle_frame(&t,TTT_SQUARE)==0);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==0 && t.open==0); /* Square held + player triggers */
    assert(ttt_toggle_frame(&t,TTT_LTRIGGER|TTT_RTRIGGER)==0);
    assert(ttt_toggle_frame(&t,TTT_HOTKEY)==1 && t.open==1); /* Square pressed last */

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
static void test_validation(void) {
    assert(ttt_vram_check(0,512,3)==1);
    assert(ttt_vram_check(0x04000000u,512,3)==0);
    assert(ttt_vram_uncached(0x04000000u)==0x44000000u);
    assert(ttt_vram_uncached(0x04088000u)==0x44088000u);
    assert(ttt_vram_uncached(0x44088000u)==0x44088000u);
    assert(ttt_vram_uncached(0x08800000u)==0u);
    assert(ttt_vram_check(0x44000000u,512,3)==0);
    assert(ttt_vram_check(0x08900000u,512,3)==4);
    assert(ttt_vram_check(0x04180000u,512,3)==5);
    assert(ttt_vram_check(0x04000000u,511,3)==0);
    assert(ttt_vram_check(0x04000000u,450,3)==3);
    assert(ttt_vram_check(0x04000000u,512,8)==2);
}
int main(void) {test_toggle();test_framebuffer();test_validation();puts("OK: controles, desenho e alias uncached testados no PC");return 0;}
