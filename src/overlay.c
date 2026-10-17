#include "overlay.h"
#include <stdint.h>

#define SW 480
#define SH 272
/* Five-by-seven font: 0-9, A-Z, space, colon, plus, slash, dash. */
static const unsigned char digits[10][7] = {
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
 {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,17,14,17,17,14},{14,17,17,15,1,2,12}
};
static const unsigned char letters[26][7] = {
 {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},
 {15,16,16,16,16,16,15},{30,17,17,17,17,17,30},
 {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},
 {15,16,16,23,17,17,15},{17,17,17,31,17,17,17},
 {14,4,4,4,4,4,14},{7,2,2,2,18,18,12},
 {17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
 {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},
 {14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
 {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},
 {15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
 {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},
 {17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
 {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
};
static const unsigned char glyph_space[7]={0,0,0,0,0,0,0};
static const unsigned char glyph_plus[7]={0,4,4,31,4,4,0};
static const unsigned char glyph_dash[7]={0,0,0,31,0,0,0};
static const unsigned char glyph_slash[7]={1,1,2,4,8,16,16};
static const unsigned char glyph_colon[7]={0,4,4,0,4,4,0};
static const unsigned char *glyph(char ch) {
 if(ch>='0' && ch<='9') return digits[ch-'0'];
 if(ch>='A' && ch<='Z') return letters[ch-'A'];
 if(ch=='+') return glyph_plus;
 if(ch=='-') return glyph_dash;
 if(ch=='/') return glyph_slash;
 if(ch==':') return glyph_colon;
 return glyph_space;
}
static uint32_t rgb(unsigned r,unsigned g,unsigned b) {
 return UINT32_C(0xff000000) | (b<<16) | (g<<8) | r;
}
static uint16_t pixel16(unsigned r,unsigned g,unsigned b,int fmt) {
 if (fmt==0) return (uint16_t)((r>>3)|((g>>2)<<5)|((b>>3)<<11));
 if (fmt==1) return (uint16_t)((r>>3)|((g>>3)<<5)|((b>>3)<<10)|0x8000);
 return (uint16_t)((r>>4)|((g>>4)<<4)|((b>>4)<<8)|0xf000);
}
static void put(void *pixels,int stride,int fmt,int x,int y,unsigned r,unsigned g,unsigned b) {
 if(x<0||x>=SW||y<0||y>=SH) return;
 if(fmt==3) ((volatile uint32_t *)pixels)[y*stride+x]=rgb(r,g,b);
 else ((volatile uint16_t *)pixels)[y*stride+x]=pixel16(r,g,b,fmt);
}
static void rect(void *pixels,int stride,int fmt,int x,int y,int w,int h,unsigned r,unsigned g,unsigned b) {
 int xx,yy;
 if(x<0){w+=x;x=0;} if(y<0){h+=y;y=0;}
 if(x+w>SW) w=SW-x;
 if(y+h>SH) h=SH-y;
 for(yy=y;yy<y+h;yy++) for(xx=x;xx<x+w;xx++) put(pixels,stride,fmt,xx,yy,r,g,b);
}
static void text(void *pixels,int stride,int fmt,int x,int y,const char *s,int scale,unsigned r,unsigned g,unsigned b) {
 int i,row,col,dx,dy;
 for(i=0;s[i]&&i<70;i++) {
  const unsigned char *gdata=glyph(s[i]);
  for(row=0;row<7;row++) for(col=0;col<5;col++) if(gdata[row]&(1u<<(4-col))) {
   for(dy=0;dy<scale;dy++)for(dx=0;dx<scale;dx++) put(pixels,stride,fmt,x+i*(6*scale)+col*scale+dx,y+row*scale+dy,r,g,b);
  }
 }
}
void ttt_draw_overlay(void *pixels,int stride,int fmt,int open) {
 if(!pixels||stride<480||stride>2048||fmt<0||fmt>3)return;
 /* Larger bright marker in the top left (diagnostic only). */
 rect(pixels,stride,fmt,8,9,180,24,10,53,127);
 rect(pixels,stride,fmt,8,9,180,4,255,204,0);
 text(pixels,stride,fmt,14,18,"TTT V034 LOADED",1,255,255,255);
 /* Keep existing bottom-left marker. */
 rect(pixels,stride,fmt,8,250,147,17,8,54,127);
 rect(pixels,stride,fmt,8,250,147,2,255,215,42);
 text(pixels,stride,fmt,14,254,"TTT EXP PRX V034",1,255,255,255);
 if(!open)return;
 /* Intentionally small diagnostic overlay, not the full V0.2 UI yet. */
 rect(pixels,stride,fmt,95,70,292,126,12,31,76);
 rect(pixels,stride,fmt,95,70,292,6,255,211,45);
 rect(pixels,stride,fmt,103,84,276,31,16,85,153);
 text(pixels,stride,fmt,112,96,"TTT EXPANSION",2,255,255,255);
 text(pixels,stride,fmt,111,125,"PLUGIN PRX RUNNING",1,255,222,65);
 text(pixels,stride,fmt,111,143,"L+R+SQUARE TO CLOSE",1,255,255,255);
 text(pixels,stride,fmt,111,164,"DIAGNOSTIC MODE V034",1,173,213,255);
}
