/* TTT Expansion V0.8 -- Goku Early and Goku Mid with replaceable portraits.
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
/* Portraits may be 128x128 (preferred) or 64x64 (legacy fallback).
 * Bytes are raw RGBA8888, not PNG. The PPSSPP TEX replacement folder is
 * different from this plugin directory and does not automatically load here. */
#define PORT_MAX_W 128
#define PORT_MAX_LEN (PORT_MAX_W*PORT_MAX_W*4)
static uint8_t portrait_pixels[2][PORT_MAX_LEN] __attribute__((aligned(16)));
static uint8_t portrait_size[2];
static int portrait_present[2];
static const char *portrait_paths[2]={
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_early.rgba",
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_mid.rgba"
};
static int load_portrait(int id) {
    SceUID f; int bytes, count=0, expected;
    if(id<0||id>=2) return 0;
    f=sceIoOpen(portrait_paths[id],PSP_O_RDONLY,0);
    if(f<0)return 0;
    bytes=sceIoLseek(f,0,PSP_SEEK_END);
    if(bytes==128*128*4)portrait_size[id]=128;
    else if(bytes==64*64*4)portrait_size[id]=64;
    else {sceIoClose(f);return 0;}
    expected=(int)portrait_size[id]*(int)portrait_size[id]*4;
    sceIoLseek(f,0,PSP_SEEK_SET);
    while(count<expected){
        int n=sceIoRead(f,portrait_pixels[id]+count,expected-count);
        if(n<=0)break;
        count+=n;
    }
    sceIoClose(f);
    if(count!=expected)return 0;
    sceKernelDcacheWritebackRange(portrait_pixels[id],expected);
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
    v[1].u=portrait_size[which];v[1].v=portrait_size[which];v[1].x=x+w;v[1].y=y+h;v[1].z=0;
    sceGuEnable(GU_TEXTURE_2D);
    sceGuTexMode(GU_PSM_8888,0,0,0);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
    sceGuTexFilter(GU_LINEAR,GU_LINEAR);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexImage(0,portrait_size[which],portrait_size[which],portrait_size[which],portrait_pixels[which]);
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
    "CIMA/BAIXO PERS  ESQ/DIR TRAJE  L/R JOG",
    "UP/DOWN CHAR  LEFT/RIGHT COLOR  L/R PLAYER",
    "ARR/ABA PERSONA  IZQ/DER TRAJE  L/R JUG"
};
static const char *foot_hints[3]={
    "X MARCA   TRIANG ABA   SELECT IDIOMA   O SAI",
    "X MARK   TRIANGLE TAB   SELECT LANG   O EXIT",
    "X MARCA   TRIANG ABA   SELECT IDIOMA   O SALE"
};

static void card(int slot,int x,int y,int selected,const char *name,const char *detail) {
    box(x,y,169,42,selected?C_GOLD:C_OUTER);
    box(x+2,y+2,165,38,selected?C_BLUE:C_DARK);
    box(x+5,y+5,35,32,selected?C_GOLD:C_LIGHT);
    box(x+7,y+7,31,28,C_BLACK);
    if(portrait_present[slot]) portrait(slot,x+7,y+7,31,28);
    else txt(x+18,y+16,slot==0?"E":"M",1,C_WHITE);
    txt(x+46,y+6,name,1,C_WHITE);
    txt(x+46,y+23,detail,1,selected?C_GOLD:C_SOFT);
}
static void render_menu(uint32_t data) {
    int character=(int)(data&1u),color=(int)((data>>1)&3u);
    int tab=(int)((data>>3)&3u),lang=(int)((data>>5)&3u);
    int player=(int)((data>>7)&3u),i;
    char active[3]="1P", idtext[6]="ID 00", curcolor[2]="1";
    if(tab>2)tab=0;
    if(lang>2)lang=0;
    active[0]=(char)('1'+player);
    idtext[4]=(char)('0'+character);
    curcolor[0]=(char)('1'+color);
    box(28,10,424,252,C_OUTER);
    box(32,14,416,244,C_DARK);
    box(37,16,406,6,C_GOLD);
    box(37,251,406,4,C_GOLD);
    box(41,27,398,29,C_BLUE);
    txt(52,36,"TTT EXPANSION",2,C_WHITE);
    txt(369,37,"V0.8",1,C_GOLD);
    for(i=0;i<3;i++) {
        int xx=48+130*i;
        box(xx,61,126,21,tab==i?C_GOLD:C_BLUE);
        box(xx+2,63,122,17,tab==i?C_DARK:C_LIGHT);
        txt(xx+9,68,titles[lang][i],1,tab==i?C_GOLD:C_WHITE);
    }
    box(47,87,385,155,C_BLUE);
    if(tab==0) {
        txt(55,93,lang==0?"JOGADOR":lang==1?"PLAYER":"JUGADOR",1,C_WHITE);
        for(i=0;i<4;i++) {
            char ptxt[]="1P";
            int xx=131+72*i;
            ptxt[0]=(char)('1'+i);
            box(xx,90,62,18,player==i?C_GOLD:C_DARK);
            box(xx+2,92,58,14,player==i?C_DARK:C_LIGHT);
            txt(xx+22,96,ptxt,1,player==i?C_GOLD:C_WHITE);
            /* Selected assignments are preview only; they cannot spawn IDs 00/01 yet. */
            if(data & (1u<<(9+i)))box(xx+52,95,4,4,C_GOLD);
        }
        card(0,55,113,character==0,"GOKU EARLY","SAGA SAIYAN");
        card(1,55,160,character==1,"GOKU MID","SAGA NAMEK");
        box(231,113,193,90,C_DARK);
        box(236,117,183,4,C_GOLD);
        box(236,126,74,73,C_GOLD);
        box(238,128,70,69,C_BLACK);
        if(portrait_present[character])portrait(character,238,128,70,69);
        else txt(248,157,character==0?"EARLY":"MID",1,C_WHITE);
        txt(316,130,character==0?"EARLY":"MID",1,C_GOLD);
        txt(316,146,idtext,1,C_WHITE);
        txt(316,161,active,1,C_WHITE);
        txt(316,178,"EXTRA",1,C_SOFT);
        txt(57,210,colors[lang],1,C_WHITE);
        for(i=0;i<4;i++){
            char digit[]="1";
            int xx=148+i*47;
            digit[0]=(char)('1'+i);
            box(xx,206,40,19,color==i?C_GOLD:C_DARK);
            box(xx+2,208,36,15,color==i?C_DARK:C_LIGHT);
            txt(xx+17,212,digit,1,color==i?C_GOLD:C_WHITE);
        }
        txt(57,229,nav_hints[lang],1,C_WHITE);
    } else if(tab==1) {
        txt(72,123,"TTT EXTRA MAPS",2,C_GOLD);
        txt(72,163,maps_pending[lang],1,C_WHITE);
    } else {
        txt(72,120,"IDIOMA / LANGUAGE",2,C_GOLD);
        txt(72,158,settings_hint[lang],1,C_WHITE);
        txt(72,179,lang==0?"PORTUGUES":lang==1?"ENGLISH":"ESPANOL",1,C_GOLD);
    }
    txt(52,242,foot_hints[lang],1,C_WHITE);
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
