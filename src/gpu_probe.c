/* TTT Expansion V0.8.2 -- isolated GE context test / Goku Early and Mid portraits.
 * Draw via established V0.5.3 verified three-callsite framebuffer hook.
 * This is UI ONLY: X confirms to a log; no game IDs are modified.
 * PSP uses ABGR8888; all text is simple 5x7 all-caps font rendered via GE.
 */
#include <pspgu.h>
#include <pspge.h>
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
/* Save the complete GPU context, not merely the GU enable bits.  The GE is
 * SHARED with TTT; sceGuInit() issues a hardware reset list.  Never call it
 * without first capturing and then restoring the game's state.  This is an
 * experimental PSP/PPSSPP-safe-context probe, NOT a proven final overlay. */
static PspGeContext __attribute__((aligned(64))) ge_saved;
static volatile int guard_failed;
static volatile int ge_init_save_rc, ge_init_restore_rc;
static volatile int ge_frame_save_rc, ge_frame_restore_rc;
static volatile unsigned ge_frame_attempts, ge_frame_restores;
TTTGeGuardStats ttt_gpu_guard_stats(void) {
    TTTGeGuardStats r;
    r.init_save=ge_init_save_rc;
    r.init_restore=ge_init_restore_rc;
    r.frame_save=ge_frame_save_rc;
    r.frame_restore=ge_frame_restore_rc;
    r.attempts=ge_frame_attempts;
    r.restores=ge_frame_restores;
    r.failed=guard_failed;
    return r;
}
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


/* V0.8.1: visual inspired by the original TTT Expansion V0.2 HTML
 * six-card layout (two active + four reserved), detail panel, bright tabs,
 * gold selected frames. Only 480x272 PSP screen coordinates are used.
 * IMPORTANT: no IDs are written to the game; X only marks a preview choice.
 */
static void panel(int x,int y,int w,int h,uint32_t border,uint32_t inside) {
    box(x,y,w,h,border);
    box(x+2,y+2,w-4,h-4,inside);
}
static void card_grid(int idx,int x,int y,int selected) {
    int active = idx<2;
    const char *label=idx==0?"EARLY":idx==1?"MID":"FUTURO";
    panel(x,y,70,52,selected?C_GOLD:active?C_LIGHT:C_BLUE,selected?C_BLUE:C_DARK);
    if(active) {
        box(x+14,y+4,42,35,selected?C_GOLD:C_LIGHT);
        box(x+16,y+6,38,31,C_BLACK);
        if(portrait_present[idx])portrait(idx,x+16,y+6,38,31);
        else txt(x+29,y+16,idx==0?"E":"M",1,C_WHITE);
        box(x+3,y+4,16,11,C_DARK);
        txt(x+5,y+6,idx==0?"00":"01",1,C_GOLD);
        txt(x+19,y+42,label,1,selected?C_GOLD:C_WHITE);
    } else {
        box(x+14,y+4,42,35,C_DARK);
        txt(x+29,y+16,"+",2,C_LIGHT);
        txt(x+13,y+42,label,1,C_SOFT);
    }
    /* Tiny inset stripe, echoes the original V0.2 active card. */
    if(selected) box(x+4,y+49,62,1,C_GOLD);
}
static void player_chip(int idx,int x,int selected,int marked) {
    char nm[3]="1P";
    nm[0]=(char)('1'+idx);
    panel(x,29,35,20,selected?C_GOLD:C_BLUE,selected?C_DARK:C_LIGHT);
    txt(x+11,35,nm,1,selected?C_GOLD:C_WHITE);
    if(marked) box(x+29,32,3,3,C_GOLD);
}
static const char *const map_subtitle[3]={
    "MAPAS EXTRAS - EM BREVE", "EXTRA MAPS - COMING SOON", "MAPAS EXTRA - PROXIMAMENTE"
};
static const char *const settings_subtitle[3]={
    "SELECT PARA TROCAR IDIOMA", "PRESS SELECT TO CHANGE LANGUAGE", "SELECT PARA CAMBIAR IDIOMA"
};
static const char *const selected_title[3]={
    "FORMA SELECIONADA", "SELECTED CHARACTER", "FORMA SELECCIONADA"
};
static const char *const nav_v2[3]={
    "CIMA/BAIXO: PERS   ESQ/DIR: COLOR   L/R: JOG",
    "UP/DOWN: CHAR   LEFT/RIGHT: COLOR   L/R: PLAYER",
    "ARR/ABA: PERS   IZQ/DER: TRAJE   L/R: JUG"
};
static const char *const footer_v2[3]={
    "TRIANG: ABA   SELECT: IDIOMA   O: FECHAR",
    "TRIANGLE: TAB   SELECT: LANG   O: CLOSE",
    "TRIANG: PEST   SELECT: IDIOMA   O: CERRAR"
};
static void render_menu(uint32_t data) {
    int character=(int)(data&1u),color=(int)((data>>1)&3u);
    int tab=(int)((data>>3)&3u),lang=(int)((data>>5)&3u);
    int player=(int)((data>>7)&3u),marked=(int)((data>>(9+player))&1u),i;
    char idtext[6]="ID 00",active[3]="1P";
    if(tab>2)tab=0;
    if(lang>2)lang=0;
    idtext[4]=(char)('0'+character);
    active[0]=(char)('1'+player);
    /* Outer blue edge and thin cyan/gold highlights: close to HTML preview. */
    box(16,7,448,259,C_OUTER);
    panel(19,9,442,255,C_LIGHT,C_DARK);
    box(24,13,432,3,C_GOLD);
    box(24,16,432,1,C_LIGHT);
    box(26,19,428,39,C_BLUE);
    box(26,56,428,2,C_LIGHT);
    txt(34,27,"TTT EXPANSION",2,C_WHITE);
    txt(224,30,"V0.8.2",1,C_GOLD);
    for(i=0;i<4;i++)player_chip(i,302+i*37,player==i,(int)((data>>(9+i))&1u));
    /* Styled navigation tabs as in the V0.2 HTML prototype. */
    for(i=0;i<3;i++) {
        int x=30+i*141;
        panel(x,62,136,24,tab==i?C_GOLD:C_LIGHT,tab==i?C_BLUE:C_DARK);
        if(tab==i)box(x+4,66,3,16,C_GOLD);
        txt(x+11,70,titles[lang][i],1,tab==i?C_GOLD:C_WHITE);
    }
    panel(29,90,229,135,C_LIGHT,C_DARK);
    panel(262,90,188,135,C_LIGHT,C_DARK);
    if(tab==0) {
        txt(37,96,"PERSONAGENS EXTRAS",1,C_SOFT);
        txt(210,96,"02/06",1,C_GOLD);
        /* Vertical progression: Early above Mid, future slots at right. */
        for(i=0;i<6;i++) {
            int x=35+(i/2)*74,y=107+(i%2)*57;
            card_grid(i,x,y,i==character);
        }
        box(270,95,174,2,C_GOLD);
        txt(271,101,selected_title[lang],1,C_WHITE);
        box(273,112,72,69,C_GOLD);
        box(275,114,68,65,C_BLACK);
        if(portrait_present[character])portrait(character,275,114,68,65);
        else txt(283,139,character==0?"EARLY":"MID",1,C_WHITE);
        txt(351,114,"GOKU",1,C_WHITE);
        txt(351,127,character==0?"EARLY":"MID",1,C_GOLD);
        txt(351,141,idtext,1,C_WHITE);
        txt(351,153,active,1,C_SOFT);
        txt(351,166,marked?"MARCADO":"PREVIA",1,marked?C_GOLD:C_SOFT);
        box(271,184,172,1,C_LIGHT);
        txt(271,187,lang==1?"COLOR":"TRAJE",1,C_WHITE);
        for(i=0;i<4;i++){
            char c[3]="C1";
            int xx=271+i*43;
            c[1]=(char)('1'+i);
            panel(xx,197,38,13,color==i?C_GOLD:C_LIGHT,color==i?C_BLUE:C_DARK);
            txt(xx+12,200,c,1,color==i?C_GOLD:C_WHITE);
        }
        txt(271,214,"PREVIA",1,C_SOFT);
        panel(338,212,104,11,C_GOLD,C_BLUE);
        txt(354,214,lang==0?"X MARCAR":lang==1?"X MARK":"X MARCAR",1,C_GOLD);
    } else if(tab==1) {
        txt(44,115,"TTT EXPANSION - MAPAS",1,C_GOLD);
        txt(44,147,map_subtitle[lang],1,C_WHITE);
        txt(281,115,"MAPS",2,C_WHITE);
    } else {
        txt(44,115,"TTT EXPANSION - AJUSTES",1,C_GOLD);
        txt(44,147,settings_subtitle[lang],1,C_WHITE);
        txt(280,118,lang==0?"PORTUGUES":lang==1?"ENGLISH":"ESPANOL",1,C_GOLD);
    }
    /* Keep all help text inside the 480x272 visible PSP screen. */
    panel(28,229,424,26,C_BLUE,C_OUTER);
    txt(35,232,nav_v2[lang],1,C_WHITE);
    txt(35,244,footer_v2[lang],1,C_SOFT);
    box(25,258,430,3,C_GOLD);
}

int ttt_gpu_init(void) {
    int rc;
    if(gu_initialized) return 0;
    if(guard_failed) return -160;
    ge_init_save_rc=sceGeSaveContext(&ge_saved);
    if(ge_init_save_rc<0){guard_failed=1;return -161;}
    rc=sceGuInit();
    /* This restore is ESSENTIAL: sceGuInit initializes the global GE with
     * default state, which otherwise survives the overlay and breaks 3D. */
    ge_init_restore_rc=sceGeRestoreContext(&ge_saved);
    if(ge_init_restore_rc<0){guard_failed=1;return -162;}
    if(rc<0){guard_failed=1;return rc;}
    portrait_present[0]=load_portrait(0);
    portrait_present[1]=load_portrait(1);
    gu_initialized=1;
    return 0;
}
void ttt_gpu_set_menu(uint32_t snapshot) {menu_snapshot=snapshot;}
int ttt_gpu_frame(void *fb,int stride,int fmt) {
    uintptr_t offset;
    int start,finish,sync,ret=0;
    uint32_t snap;
    if(!gu_initialized || guard_failed) return -160;
    if(ttt_vram_check((uintptr_t)fb,stride,fmt)!=0)return -11;
    ++ge_frame_attempts;
    ge_frame_save_rc=sceGeSaveContext(&ge_saved);
    if(ge_frame_save_rc<0){guard_failed=1;return -163;}
    offset=(uintptr_t)fb & (uintptr_t)0x003fffff;
    snap=menu_snapshot;
    start=sceGuStart(GU_DIRECT,gu_list);
    if(start<0){ret=start;goto restore;}
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
    finish=sceGuFinish();
    if(finish<0) {ret=finish;goto restore;}
    sync=sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
    if(sync<0) ret=sync;
restore:
    /* The original hook only restored its JALs; here we also restore the
     * entire GE state after each overlay frame.  Stop the hook if restore
     * fails, to avoid persistent corruption. */
    ge_frame_restore_rc=sceGeRestoreContext(&ge_saved);
    if(ge_frame_restore_rc<0){guard_failed=1;return -164;}
    ++ge_frame_restores;
    if(ret<0)guard_failed=1;
    return ret;
}
void ttt_gpu_shutdown(void) { }
