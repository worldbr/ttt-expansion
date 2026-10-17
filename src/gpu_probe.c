/* TTT Expansion V0.7 -- Goku Early and Goku Mid with replaceable portraits.
 * Draw via established V0.5.3 verified three-callsite framebuffer hook.
 * This is UI ONLY: X confirms to a log; no game IDs are modified.
 * PSP uses ABGR8888; all text is simple 5x7 all-caps font rendered via GE.
 */
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <psputils.h>
#include <stdint.h>
#include <stddef.h>
#include "framecheck.h"
#include "gpu_probe.h"

#define C_OUTER UINT32_C(0xFF0B0F16)
#define C_DARK  UINT32_C(0xFF402515)
#define C_BLUE  UINT32_C(0xFF8A4C15)
#define C_LIGHT UINT32_C(0xFFD18D30)
#define C_GOLD  UINT32_C(0xFF1ED6FF)
#define C_WHITE UINT32_C(0xFFFFFFFF)
#define C_SOFT  UINT32_C(0xFFDEE7FF)
#define C_BLACK UINT32_C(0xFF13171C)

/* Enough room for text rendered as horizontal bitmap runs. */
static uint32_t __attribute__((aligned(16))) gu_list[32768];
static int gu_initialized;
static volatile uint32_t menu_snapshot;
/* These RGBA8888 textures live in stable, 16-byte-aligned user RAM.
 * A plain 64x64 RGBA file is used instead of runtime PNG decoding.
 * Pixel bytes R,G,B,A match the PSP's little-endian GU_PSM_8888 order. */
#define PORT_W 64
#define PORT_H 64
#define PORT_LEN (PORT_W * PORT_H * 4)
static uint8_t portrait_pixels[2][PORT_LEN] __attribute__((aligned(16)));
static int portrait_present[2];
static const char *portrait_paths[2]={
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_early.rgba",
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_mid.rgba"
};
static int load_portrait(int id) {
    SceUID f; int count=0;
    if(id<0||id>=2) return 0;
    f=sceIoOpen(portrait_paths[id],PSP_O_RDONLY,0);
    if(f<0)return 0;
    while(count<PORT_LEN){
        int n=sceIoRead(f,portrait_pixels[id]+count,PORT_LEN-count);
        if(n<=0)break;
        count+=n;
    }
    sceIoClose(f);
    if(count!=PORT_LEN)return 0;
    sceKernelDcacheWritebackRange(portrait_pixels[id],PORT_LEN);
    return 1;
}
int ttt_gpu_portrait_status(void) {
    return (portrait_present[0]?1:0) | (portrait_present[1]?2:0);
}


typedef struct { int16_t x,y,z; } Vertex;
static void box(int x,int y,int w,int h,uint32_t color) {
    Vertex *v;
    if (w<=0||h<=0||x<0||y<0||x+w>480||y+h>272) return;
    v=(Vertex*)sceGuGetMemory(2*sizeof(Vertex));
    if(!v) return;
    v[0].x=(int16_t)x;v[0].y=(int16_t)y;v[0].z=0;
    v[1].x=(int16_t)(x+w);v[1].y=(int16_t)(y+h);v[1].z=0;
    sceGuColor(color);
    sceGuDrawArray(GU_SPRITES,GU_VERTEX_16BIT | GU_TRANSFORM_2D,2,NULL,v);
}
static const uint8_t nums[10][7]={
 {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},
 {14,17,1,2,4,8,31},{30,1,1,14,1,1,30},
 {2,6,10,18,31,2,2},{31,16,16,30,1,1,30},
 {6,8,16,30,17,17,14},{31,1,2,4,8,8,8},
 {14,17,17,14,17,17,14},{14,17,17,15,1,2,12}
};
static const uint8_t alpha[26][7]={
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
static const uint8_t zero[7]={0,0,0,0,0,0,0};
static const uint8_t dash[7]={0,0,0,31,0,0,0};
static const uint8_t plus[7]={0,4,4,31,4,4,0};
static const uint8_t colon[7]={0,4,4,0,4,4,0};
static const uint8_t dot[7]={0,0,0,0,0,12,12};
static const uint8_t slash[7]={1,1,2,4,8,16,16};
static const uint8_t *glyph(char c) {
    if(c>='A'&&c<='Z')return alpha[c-'A'];
    if(c>='0'&&c<='9')return nums[c-'0'];
    if(c=='+')return plus;
    if(c=='-')return dash;
    if(c==':')return colon;
    if(c=='.')return dot;
    if(c=='/')return slash;
    return zero;
}
/* Horizontal runs avoid issuing one GE sprite per text pixel. */
static void txt(int x,int y,const char *s,int scale,uint32_t color) {
    int i,j,k,begin;const uint8_t *bits;
    if(!s||scale<1||scale>2)return;
    for(i=0;s[i]&&i<54;i++) {
        bits=glyph(s[i]);
        for(j=0;j<7;j++) {
            for(k=0;k<5;) {
                if(!(bits[j]&(1u<<(4-k)))){k++;continue;}
                begin=k;
                do{k++;}while(k<5 && (bits[j]&(1u<<(4-k))));
                box(x+(i*6+begin)*scale,y+j*scale,(k-begin)*scale,scale,color);
            }
        }
    }
}
typedef struct { int16_t u,v,x,y,z; } TTexVertex;
static void portrait(int which,int x,int y,int w,int h) {
    TTexVertex *v;
    if(which<0||which>=2 || !portrait_present[which])return;
    if(x<0||y<0||x+w>480||y+h>272)return;
    v=(TTexVertex*)sceGuGetMemory(2*sizeof(TTexVertex));
    if(!v)return;
    v[0].u=0;v[0].v=0;v[0].x=x;v[0].y=y;v[0].z=0;
    v[1].u=64;v[1].v=64;v[1].x=x+w;v[1].y=y+h;v[1].z=0;
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexMode(GU_PSM_8888,0,0,0);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
    sceGuTexFilter(GU_LINEAR,GU_LINEAR);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexImage(0,PORT_W,PORT_H,PORT_W,portrait_pixels[which]);
    sceGuColor(C_WHITE);
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D,2,NULL,v);
    sceGuDisable(GU_TEXTURE_2D);
}
static const char *titles[3][3]={
    {"PERSONAGENS","MAPAS","AJUSTES"},
    {"CHARACTERS","MAPS","SETTINGS"},
    {"PERSONAJES","MAPAS","AJUSTES"}
};
static const char *colors[3]={"TRAJE","COSTUME","TRAJE"};
static const char *maps_pending[3]={"MAPAS EM DESENVOLVIMENTO","MAPS UNDER DEVELOPMENT","MAPAS EN DESARROLLO"};
static const char *settings_hint[3]={"SELECT MUDA IDIOMA","SELECT CHANGES LANGUAGE","SELECT CAMBIA IDIOMA"};
static const char *preview[3]={"X TESTE  SEM ID","X TEST  NO ID","X PRUEBA SIN ID"};
static const char *nav_hints[3]={
    "ESQ/DIR PERSONAGEM  CIMA/BAIXO TRAJE",
    "LEFT/RIGHT CHARACTER  UP/DOWN COSTUME",
    "IZQ/DER PERSONA  ARR/ABA TRAJE"
};
static const char *foot_hints[3]={
    "TRIANGULO ABA  SELECT IDIOMA  CIRCULO FECHA",
    "TRIANGLE TAB  SELECT LANGUAGE  CIRCLE CLOSE",
    "TRIANGULO MENU  SELECT IDIOMA  CIRCULO CIERRA"
};

static void card(int slot,int x,int y,int selected,const char *name,const char *detail) {
    box(x,y,168,54,selected?C_GOLD:C_OUTER);
    box(x+2,y+2,164,50,selected?C_BLUE:C_DARK);
    box(x+6,y+6,43,42,selected?C_GOLD:C_LIGHT);
    box(x+8,y+8,39,38,C_BLACK);
    if(portrait_present[slot])portrait(slot,x+8,y+8,39,38);
    else txt(x+21,y+18,slot==0?"E":"M",2,C_WHITE);
    txt(x+54,y+10,name,1,C_WHITE);
    txt(x+54,y+29,detail,1,selected?C_GOLD:C_SOFT);
}
static void render_menu(uint32_t data) {
    int character=(int)(data&1u),color=(int)((data>>1)&3u);
    int tab=(int)((data>>3)&3u),lang=(int)((data>>5)&3u),i;
    char clr[]="1/4";
    if(tab>2)tab=0;
    if(lang>2)lang=0;
    clr[0]=(char)('1'+color);
    /* Same original blue/gold identity; enlarged photo cards and preview. */
    box(35,12,410,250,C_OUTER);
    box(39,16,402,242,C_DARK);
    box(44,17,392,7,C_GOLD);
    box(44,248,392,5,C_GOLD);
    box(47,29,386,27,C_BLUE);
    txt(59,36,"TTT EXPANSION",2,C_WHITE);
    txt(345,37,"V0.7",1,C_GOLD);
    for(i=0;i<3;i++) {
        int xx=57+123*i;
        box(xx,62,118,21,tab==i?C_GOLD:C_BLUE);
        box(xx+2,64,114,17,tab==i?C_DARK:C_LIGHT);
        txt(xx+9,69,titles[lang][i],1,tab==i?C_GOLD:C_WHITE);
    }
    box(56,89,368,149,C_BLUE);
    if(tab==0) {
        card(0,64,96,character==0,"GOKU EARLY","SAGA SAIYAN");
        card(1,64,155,character==1,"GOKU MID","SAGA NAMEK");
        box(239,96,177,117,C_DARK);
        box(244,100,167,5,C_GOLD);
        box(244,110,88,88,C_GOLD);
        box(246,112,84,84,C_BLACK);
        if(portrait_present[character]) portrait(character,246,112,84,84);
        else txt(259,140,character==0?"EARLY":"MID",1,C_WHITE);
        txt(339,112,"GOKU",1,C_WHITE);
        txt(339,124,character==0?"EARLY":"MID",1,C_GOLD);
        txt(339,145,"ID TBD",1,C_SOFT);
        txt(339,156,colors[lang],1,C_SOFT);
        txt(339,168,clr,1,C_GOLD);
        txt(339,184,"EXTRA",1,C_WHITE);
        txt(252,200,preview[lang],1,C_SOFT);
        txt(69,219,nav_hints[lang],1,C_WHITE);
    } else if(tab==1) {
        txt(77,112,"TTT EXTRA MAPS",2,C_GOLD);
        txt(78,153,maps_pending[lang],1,C_WHITE);
        txt(78,186,"SEM ALTERAR A ISO",1,C_SOFT);
    } else {
        txt(78,111,"IDIOMA / LANGUAGE",2,C_GOLD);
        txt(78,150,settings_hint[lang],1,C_WHITE);
        txt(78,174,lang==0?"PORTUGUES":lang==1?"ENGLISH":"ESPANOL",1,C_GOLD);
    }
    txt(58,240,foot_hints[lang],1,C_WHITE);
}
int ttt_gpu_init(void) {
    int rc;if(gu_initialized)return 0;
    rc=sceGuInit();if(rc>=0) {
        portrait_present[0]=load_portrait(0);
        portrait_present[1]=load_portrait(1);
        gu_initialized=1;
    }
    return rc;
}
void ttt_gpu_set_menu(uint32_t snapshot) {menu_snapshot=snapshot;}
int ttt_gpu_frame(void *fb,int stride,int fmt) {
    uintptr_t offset;int start,finish,sync;
    uint32_t snap;
    if(!gu_initialized) return -10;
    if(ttt_vram_check((uintptr_t)fb,stride,fmt)!=0)return -11;
    offset=(uintptr_t)fb & (uintptr_t)0x003fffff;
    snap=menu_snapshot;
    start=sceGuStart(GU_DIRECT,gu_list);if(start<0)return start;
    sceGuDrawBufferList(fmt,(void*)offset,stride);
    sceGuOffset(2048-240,2048-136);
    sceGuViewport(2048,2048,480,272);
    sceGuScissor(0,0,480,272);
    sceGuEnable(GU_SCISSOR_TEST);
    sceGuDisable(GU_TEXTURE_2D);sceGuDisable(GU_DEPTH_TEST);
    sceGuDisable(GU_LIGHTING);sceGuDisable(GU_CULL_FACE);
    sceGuDisable(GU_ALPHA_TEST);sceGuDisable(GU_BLEND);
    sceGuDisable(GU_STENCIL_TEST);sceGuDisable(GU_COLOR_TEST);
    sceGuDisable(GU_COLOR_LOGIC_OP);sceGuDisable(GU_FOG);
    sceGuDepthMask(1);sceGuShadeModel(GU_FLAT);
    render_menu(snap);
    finish=sceGuFinish();if(finish<0)return finish;
    sync=sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
    if(sync<0)return sync;
    return 0;
}
void ttt_gpu_shutdown(void) { }
