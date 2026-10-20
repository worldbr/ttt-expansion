/* TTT Expansion V0.9.1 -- isolated GE context protection / Goku Early and Mid portraits.
 * Draw via established V0.5.3 verified three-callsite framebuffer hook.
 * X may arm the separate ID00 battle bridge; graphics stay isolated.
 * PSP uses ABGR8888; all text is simple 5x7 all-caps font rendered via GE.
 */
#include <pspgu.h>
#include <pspge.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <psputils.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <malloc.h>
#include "framecheck.h"
#include "gpu_probe.h"
#include "gu_budget.h"

#define C_OUTER UINT32_C(0xFF0B0F16)
#define C_DARK  UINT32_C(0xFF402515)
#define C_BLUE  UINT32_C(0xFF8A4C15)
#define C_LIGHT UINT32_C(0xFFD18D30)
#define C_GOLD  UINT32_C(0xFF1ED6FF)
#define C_WHITE UINT32_C(0xFFFFFFFF)
#define C_SOFT  UINT32_C(0xFFDEE7FF)
#define C_BLACK UINT32_C(0xFF13171C)

/* V0.9.8.9: prevent PSP GU display-list overrun into the plugin's BSS.
 * Evidence: HOOK installed=0x571ED6FF exactly matches a GU color-command
 * encoding of menu gold 0x1ED6FF. Old list was only 128 KiB and unbounded.
 */
static TTTGuListStorage __attribute__((aligned(16))) gu_storage;
static TTTGuBudget gu_budget;
static volatile unsigned gu_peak_primitives, gu_last_primitives, gu_last_skipped;
static volatile int gu_canary_fault;
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
    r.primitives=gu_last_primitives;
    r.peak_primitives=gu_peak_primitives;
    r.skipped_primitives=gu_last_skipped;
    r.canary_fault=gu_canary_fault;
    r.list_bytes=TTT_GU_LIST_WORDS * 4u;
    return r;
}
static volatile uint32_t menu_snapshot;
/* Dynamic RGBA portraits; all packaged models are 64x64.
 * Compacting static BSS helps avoid V0988 early black screen regression. */
#define PORT_MAX_W 64
#define PORT_MAX_LEN (PORT_MAX_W*PORT_MAX_W*4)
static uint8_t portrait_pixels[3][PORT_MAX_LEN] __attribute__((aligned(16)));
static uint8_t portrait_size[3];
static int portrait_present[3];
static const char *portrait_paths[3]={
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_early.rgba",
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_mid.rgba",
    "ms0:/PSP/PLUGINS/TTT_EXPANSION/portraits/goku_mid_ssj.rgba"
};
static int load_portrait(int id) {
    SceUID f; int bytes, count=0, expected;
    if(id<0||id>=3) return 0;
    f=sceIoOpen(portrait_paths[id],PSP_O_RDONLY,0);
    if(f<0)return 0;
    bytes=sceIoLseek(f,0,PSP_SEEK_END);
    if(bytes==64*64*4)portrait_size[id]=64;
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
    return (portrait_present[0]?1:0) | (portrait_present[1]?2:0) | (portrait_present[2]?4:0);
}


typedef struct { int16_t x,y,z; } Vertex;
static void box(int x,int y,int w,int h,uint32_t color) {
    Vertex *v;
    if (w<=0||h<=0||x<0||y<0||x+w>480||y+h>272) return;
    if(!ttt_gu_budget_take(&gu_budget)) return;
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
    if(which<0||which>=3 || !portrait_present[which])return;
    if(x<0||y<0||x+w>480||y+h>272)return;
    if(!ttt_gu_budget_take(&gu_budget)) return;
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
/* Minimal panel helper is used only when texture assets are missing.
 * The previous UI and its outdated navigation instructions are removed. */
static void panel(int x,int y,int w,int h,uint32_t border,uint32_t inside){
    box(x,y,w,h,border);
    box(x+2,y+2,w-4,h-4,inside);
}

/* === V1.0.1 full textured menu: UI .4444 blobs are loaded AFTER opening.
 * Do not store large GUI textures in PRX .bss; a 512KiB static list caused
 * a black-screen regression. Files are raw PSP ABGR4444, NOT PNG. === */
#define TTT_MENU_BG_BYTES (256u*256u*2u)
#define TTT_FONT_BYTES (256u*128u*2u)
#define TTT_BG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/ui/menu_bg.4444"
#define TTT_FONT_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/ui/font_atlas.4444"
static uint8_t *ttt_skin_bg;
static uint8_t *ttt_skin_font;
static volatile int ttt_skin_ready;
static volatile unsigned ttt_skin_mask;

int ttt_gpu_skin_status(void) { return (int)ttt_skin_mask; }

static uint8_t *skin_load(const char *path,int expected){
    SceUID fd;uint8_t *buf;int size,count=0;
    fd=sceIoOpen(path,PSP_O_RDONLY,0);
    if(fd<0)return NULL;
    size=sceIoLseek(fd,0,PSP_SEEK_END);
    if(size!=expected){sceIoClose(fd);return NULL;}
    sceIoLseek(fd,0,PSP_SEEK_SET);
    buf=(uint8_t*)memalign(16,(size_t)expected);
    if(!buf){sceIoClose(fd);return NULL;}
    while(count<expected){
        int n=sceIoRead(fd,buf+count,expected-count);
        if(n<=0)break;
        count+=n;
    }
    sceIoClose(fd);
    if(count!=expected){free(buf);return NULL;}
    sceKernelDcacheWritebackRange(buf,(size_t)expected);
    return buf;
}
static void skin_load_all(void){
    ttt_skin_bg=skin_load(TTT_BG_PATH,(int)TTT_MENU_BG_BYTES);
    ttt_skin_font=skin_load(TTT_FONT_PATH,(int)TTT_FONT_BYTES);
    ttt_skin_mask=(ttt_skin_bg?1u:0u)|(ttt_skin_font?2u:0u);
    ttt_skin_ready=(ttt_skin_bg && ttt_skin_font)?1:0;
    if(!ttt_skin_ready){
        if(ttt_skin_bg){free(ttt_skin_bg);ttt_skin_bg=NULL;}
        if(ttt_skin_font){free(ttt_skin_font);ttt_skin_font=NULL;}
    }
}

/* One GU textured sprite, 2 vertices. All four helpers share the proven
 * 128KiB GU list and the primitive budget (no unbounded glyph rectangles). */
static void skin_sprite(int x,int y,int w,int h,int u,int v,int tw,int th){
    TTexVertex *pt;
    if(x<0||y<0||w<=0||h<=0||x+w>480||y+h>272)return;
    if(!ttt_gu_budget_take(&gu_budget))return;
    pt=(TTexVertex*)sceGuGetMemory(2*sizeof(TTexVertex));
    if(!pt)return;
    pt[0].u=(int16_t)u;pt[0].v=(int16_t)v;pt[0].x=(int16_t)x;pt[0].y=(int16_t)y;pt[0].z=0;
    pt[1].u=(int16_t)(u+tw);pt[1].v=(int16_t)(v+th);
    pt[1].x=(int16_t)(x+w);pt[1].y=(int16_t)(y+h);pt[1].z=0;
    sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,NULL,pt);
}
static void skin_bind_background(void){
    sceGuEnable(GU_TEXTURE_2D);
    sceGuDisable(GU_BLEND);
    sceGuTexMode(GU_PSM_4444,0,0,0);
    sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
    sceGuTexFilter(GU_LINEAR,GU_LINEAR);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexImage(0,256,256,256,ttt_skin_bg);
    sceGuColor(C_WHITE);
}
static void skin_bind_font(void){
    sceGuEnable(GU_TEXTURE_2D);
    sceGuEnable(GU_BLEND);
    sceGuBlendFunc(GU_ADD,GU_SRC_ALPHA,GU_ONE_MINUS_SRC_ALPHA,0,0);
    sceGuTexMode(GU_PSM_4444,0,0,0);
    sceGuTexFunc(GU_TFX_MODULATE,GU_TCC_RGBA);
    sceGuTexFilter(GU_NEAREST,GU_NEAREST);
    sceGuTexWrap(GU_CLAMP,GU_CLAMP);
    sceGuTexImage(0,256,128,256,ttt_skin_font);
}
/* Font sprite atlas: ASCII 32..127, 16 columns, 16x16 cells.
 * Actual visible glyph in each cell is ~9x12 pixels. */
static void skin_text(int x,int y,const char *str,int large,uint32_t color){
    int i,dx=large?11:7,dy=large?17:11;
    int cw=large?12:8;
    if(!str)return;
    sceGuColor(color);
    for(i=0;str[i]&&i<58;i++){
        unsigned ch=(unsigned char)str[i];int at;
        if(ch<32||ch>127)ch='?';
        at=(int)(ch-32);
        if(x+i*dx+cw>480)break;
        if(ch!=' ')skin_sprite(x+i*dx,y,cw,dy,(at%16)*16,(at/16)*16,11,14);
    }
}
static void skin_outline(int x,int y,int w,int h,uint32_t color){
    box(x,y,w,2,color);box(x,y+h-2,w,2,color);
    box(x,y,2,h,color);box(x+w-2,y,2,h,color);
}
static const char *const skin_tabs[3][3]={
    {"PERSONAGENS","MAPAS","AJUSTES"},
    {"CHARACTERS","MAPS","SETTINGS"},
    {"PERSONAJES","MAPAS","AJUSTES"}
};
/* The three real cards form a horizontal row.  The bottom three are
 * explicitly non-interactive slots reserved for future characters.
 * Focus is a separate row: character cards or costume chips (snapshot bit31).
 * No static selection text is baked into the texture. */
static const char *const skin_name[3]={"Goku (Inicio)","Goku (Meio)","Goku (Meio)"};
static const char *const skin_subtitle[3]={"","","Super Saiyajin"};
static const char *const skin_card_a[3]={"Goku","Goku","Goku (Meio)"};
static const char *const skin_card_b[3]={"(Inicio)","(Meio)","Super Saiyajin"};
static const char *const skin_pending[3]={"FUTURO","FUTURE","FUTURO"};
/* A compact atlas sprite is much less expensive than the old vector glyphs.
 * Each glyph uses exactly one GU sprite; text remains readable at 480x272. */
static void skin_text_compact(int x,int y,const char *str,int step,int glyph_w,int glyph_h,uint32_t color){
    int i;
    sceGuColor(color);
    if(!str) return;
    for(i=0;str[i]&&i<62;i++){
        unsigned char ch=(unsigned char)str[i];int at;
        if(ch<32u||ch>127u)ch='?';
        if(x+i*step+glyph_w>480)break;
        at=(int)ch-32;
        if(ch!=' ')skin_sprite(x+i*step,y,glyph_w,glyph_h,(at%16)*16,(at/16)*16,11,14);
    }
}
static void skin_frame_fallback(uint32_t bits){
    /* Installed PRX must NEVER silently show the old menu.  In the absence
     * of external texture blobs, keep a minimal NEW layout and show why.
     * The actual menu navigation remains functional. */
    int character=(int)(bits&3u),color=(int)((bits>>2)&3u);
    int player=(int)((bits>>8)&3u),focus=(int)((bits>>31)&1u),i;
    char num[3]="1P",label[4]="C1";
    box(8,8,464,256,C_DARK);panel(12,12,456,248,C_LIGHT,C_OUTER);
    txt(23,23,"TTT EXPANSION V1.0.1",1,C_WHITE);
    txt(25,50,"TEXTURAS AUSENTES - VERIFIQUE PASTA UI",1,C_GOLD);
    for(i=0;i<3;i++){
        int x=22+153*i;
        panel(x,83,146,79,(character==i&&!focus)?C_GOLD:C_LIGHT,C_DARK);
        if(portrait_present[i])portrait(i,x+7,91,49,49);
        txt(x+60,98,i==0?"GOKU":i==1?"GOKU":"GOKU",1,C_WHITE);
        txt(x+60,112,i==0?"INICIO":i==1?"MEIO":"MEIO SSJ",1,C_GOLD);
    }
    txt(25,177,"JOGADOR",1,C_WHITE);
    num[0]=(char)('1'+player);txt(90,177,num,1,C_GOLD);
    txt(184,177,"COLOR",1,C_WHITE);
    label[1]=(char)('1'+color);txt(236,177,label,1,C_GOLD);
    txt(25,204,focus?"FOCO: COLORS":"FOCO: PERSONAGENS",1,C_GOLD);
    txt(24,235,"ESQ/DIR SELECIONA  CIMA/BAIXO TROCA FOCO",1,C_WHITE);
}
static void render_menu_textured(uint32_t bits){
    int character=(int)(bits&3u),color=(int)((bits>>2)&3u);
    int tab=(int)((bits>>4)&3u),lang=(int)((bits>>6)&3u);
    int player=(int)((bits>>8)&3u),bridge=(int)((bits>>30)&1u);
    int focus=(int)((bits>>31)&1u),i;
    char number[3]="00",idline[9]="ID 00",slot[3]="1P",col[3]="C1";
    if(character>2)character=0;
    if(tab>2)tab=0;
    if(lang>2)lang=0;
    /* Layer 1: actual PSP 4444 background texture. */
    skin_bind_background();
    skin_sprite(0,0,480,272,0,0,256,256);
    sceGuDisable(GU_TEXTURE_2D);
    /* Layer 2: selective, geometry-aligned interaction highlights. */
    skin_outline(13+153*tab,61,147,26,C_GOLD);
    skin_outline(309+38*player,18,35,31,C_GOLD);
    for(i=0;i<4;i++)if((bits>>(10+i))&1u)box(338+i*38,21,3,3,C_GOLD);
    if(tab==0){
        skin_outline(19+character*77,117,73,51,focus?C_LIGHT:C_GOLD);
        skin_outline(270+color*48,202,42,20,focus?C_GOLD:C_LIGHT);
        /* Small focus indicator follows active navigation row. */
        box(focus?262:15,focus?206:130,3,11,C_GOLD);
    }
    /* Layer 3: all portraits use 64x64 RGBA files supplied with the plugin. */
    if(tab==0){
        for(i=0;i<3;i++)if(portrait_present[i])portrait(i,26+77*i,122,32,25);
        if(portrait_present[character])portrait(character,269,120,82,69);
    }
    /* Layer 4: single font atlas, no per-pixel rectangles and no overflow. */
    skin_bind_font();
    skin_text_compact(23,22,"TTT EXPANSION",11,11,17,C_WHITE);
    skin_text(244,31,"V1.0.1",0,C_GOLD);
    for(i=0;i<4;i++){
        slot[0]=(char)('1'+i);
        skin_text(319+i*38,27,slot,0,i==player?C_GOLD:C_WHITE);
    }
    for(i=0;i<3;i++)skin_text(23+i*153,70,skin_tabs[lang][i],0,i==tab?C_GOLD:C_WHITE);
    if(tab==0){
        skin_text(20,101,lang==1?"EXTRA CHARACTERS":lang==2?"PERSONAJES EXTRA":"PERSONAGENS EXTRAS",0,C_WHITE);
        skin_text(207,101,"03/06",0,C_GOLD);
        skin_text(263,101,lang==1?"PREVIEW":lang==2?"VISTA":"PREVIA",0,C_WHITE);
        for(i=0;i<6;i++){
            int x=19+(i%3)*77,y=117+(i/3)*55;
            number[0]='0';number[1]=(char)('0'+i);
            skin_text_compact(x+51,y+3,number,5,5,9,i==character?C_GOLD:C_SOFT);
            if(i<3){
                /* Two lines make Super Saiyajin a real subtitle. */
                skin_text_compact(x+4,y+29,skin_card_a[i],5,5,9,i==character?C_GOLD:C_WHITE);
                skin_text_compact(x+4,y+39,skin_card_b[i],4,5,8,i==character?C_GOLD:C_WHITE);
            }else{
                skin_text_compact(x+29,y+13,"+",9,9,14,C_LIGHT);
                skin_text_compact(x+11,y+39,skin_pending[lang],5,5,8,C_SOFT);
            }
        }
        /* Preview description: exact requested names, SSJ subtitle below. */
        skin_text_compact(359,124,skin_name[character],6,6,11,C_WHITE);
        if(character==2)skin_text_compact(359,138,skin_subtitle[character],6,6,10,C_GOLD);
        idline[3]='0';idline[4]=(char)('0'+character);
        skin_text(360,155,idline,0,C_SOFT);
        slot[0]=(char)('1'+player);
        skin_text(359,169,slot,0,C_WHITE);
        skin_text_compact(359,182,bridge?"ATIVO":((bits>>(10+player))&1u)?"CONFIRMADO":"PREVIA",6,6,10,bridge?C_GOLD:C_SOFT);
        for(i=0;i<4;i++){
            col[1]=(char)('1'+i);
            skin_text(282+i*48,207,col,0,(i==color)?C_GOLD:C_WHITE);
        }
    }else if(tab==1){
        skin_text(27,116,lang==1?"EXTRA MAPS":lang==2?"MAPAS EXTRA":"MAPAS EXTRAS",1,C_WHITE);
        skin_text(28,150,skin_pending[lang],0,C_GOLD);
    }else{
        skin_text(27,116,lang==1?"SETTINGS":lang==2?"AJUSTES":"CONFIGURACOES",1,C_WHITE);
        skin_text(27,150,lang==0?"SELECT = IDIOMA":lang==1?"SELECT = LANGUAGE":"SELECT = IDIOMA",0,C_GOLD);
    }
    /* Two compact instructions; the menu is 480 pixels wide on the PSP. */
    skin_text_compact(22,242,lang==1?"LEFT/RIGHT: SELECT  UP/DOWN: FOCUS  L/R: PLAYER":lang==2?"IZQ/DER: ELEGIR  ARR/ABA: FOCO  L/R: JUG":"ESQ/DIR: SELECIONAR  CIMA/BAIXO: FOCO  L/R: JOG",6,6,10,C_WHITE);
    skin_text_compact(22,254,"X: CONFIRMAR  TRIANGULO: ABA  SELECT: IDIOMA  O: SAIR",6,6,10,C_SOFT);
    sceGuDisable(GU_BLEND);
    sceGuDisable(GU_TEXTURE_2D);
}

static void render_menu(uint32_t bits){
    if(ttt_skin_ready)render_menu_textured(bits);
    else skin_frame_fallback(bits); /* never silently use old layout */
}

int ttt_gpu_init(void) {
    int rc;
    if(gu_initialized) return 0;
    if(guard_failed) return -160;
    ttt_gu_canaries_init(&gu_storage);
    ttt_gu_budget_begin(&gu_budget);
    gu_peak_primitives=gu_last_primitives=gu_last_skipped=0;
    gu_canary_fault=0;
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
    portrait_present[2]=load_portrait(2);
    skin_load_all();
    gu_initialized=1;
    return 0;
}
void ttt_gpu_set_menu(uint32_t snapshot) {menu_snapshot=snapshot;}
int ttt_gpu_frame(void *fb,int stride,int fmt) {
    uintptr_t offset;
    int start,finish,sync,ret=0;
    uint32_t snap;
    if(!gu_initialized || guard_failed) return -160;
    if(!ttt_gu_canaries_ok(&gu_storage)){
        gu_canary_fault=guard_failed=1;
        return -165;
    }
    if(ttt_vram_check((uintptr_t)fb,stride,fmt)!=0)return -11;
    ++ge_frame_attempts;
    ge_frame_save_rc=sceGeSaveContext(&ge_saved);
    if(ge_frame_save_rc<0){guard_failed=1;return -163;}
    offset=(uintptr_t)fb & (uintptr_t)0x003fffff;
    snap=menu_snapshot;
    ttt_gu_budget_begin(&gu_budget);
    start=sceGuStart(GU_DIRECT,gu_storage.words);
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
    gu_last_primitives=gu_budget.used;
    gu_last_skipped=gu_budget.skipped;
    if(gu_last_primitives>gu_peak_primitives)gu_peak_primitives=gu_last_primitives;
    finish=sceGuFinish();
    if(finish<0) {ret=finish;goto restore;}
    sync=sceGuSync(GU_SYNC_FINISH,GU_SYNC_WHAT_DONE);
    if(sync<0) ret=sync;
restore:
    /* Do not continue after an observed overwrite. Restore GE first so that
     * the game's GPU is left in its original context when possible. */
    if(!ttt_gu_canaries_ok(&gu_storage)){
        gu_canary_fault=guard_failed=1;
        ret=-165;
    }
    /* The original hook only restored its JALs; here we also restore the
     * entire GE state after each overlay frame.  Stop the hook if restore
     * fails, to avoid persistent corruption. */
    ge_frame_restore_rc=sceGeRestoreContext(&ge_saved);
    if(ge_frame_restore_rc<0){guard_failed=1;return -164;}
    ++ge_frame_restores;
    /* A transient GU command error is retryable after the GE context was
     * restored successfully. Only save/restore failures poison the guard. */
    return ret;
}
void ttt_gpu_shutdown(void) {
    /* Called after the frame hook is no longer drawing. */
    if(ttt_skin_bg){free(ttt_skin_bg);ttt_skin_bg=NULL;}
    if(ttt_skin_font){free(ttt_skin_font);ttt_skin_font=NULL;}
    ttt_skin_ready=0;ttt_skin_mask=0;
}
