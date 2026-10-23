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
#include <pspsysmem.h>
#include <pspmodulemgr.h>
#include "framecheck.h"
#include "gpu_probe.h"
#include "dlc_story.h"
#include "ui_text.h"
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
static volatile unsigned dlc_mission_index;
static volatile uint32_t dlc_mission_completed;
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

/* DLC V2.0.2 packed skin: the user's game successfully allocates exactly
 * 128 KiB for BG but refuses another 64 KiB for the font (0x800200D9).
 * Keep one 128-KiB PSP user partition allocation, and store both textures:
 * rows 0..159 = downsampled menu BG, rows 160..255 = font rows 0..95.
 * Existing .4444 files remain unchanged on disk. This is a RAM-only atlas. */
#define TTT_MENU_BG_BYTES (256u*256u*2u)
#define TTT_FONT_BYTES (256u*128u*2u)
#define TTT_SKIN_BG_ROWS 160u
#define TTT_SKIN_FONT_ROWS 96u
#define TTT_SKIN_FONT_Y 160u
#define TTT_SKIN_FONT_USED_BYTES (256u*TTT_SKIN_FONT_ROWS*2u)
#define TTT_BG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/ui/menu_bg.4444"
#define TTT_FONT_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/ui/font_atlas.4444"
static uint8_t *ttt_skin_bg;
static uint8_t *ttt_skin_font;
/* Exactly ONE PSP allocation for both textures. The font has no separate
 * block; this matters on a game process with severely fragmented user RAM. */
static SceUID skin_bg_block=-1;
static SceUID skin_font_block=-1;
static volatile int ttt_skin_ready;
static volatile unsigned ttt_skin_mask;
static TTTSkinStats skin_stats;

int ttt_gpu_skin_status(void) { return (int)ttt_skin_mask; }
TTTSkinStats ttt_gpu_skin_stats(void) { return skin_stats; }

static void skin_free(uint8_t **ptr,SceUID *block){
    if(*block>=0) {sceKernelFreePartitionMemory(*block);*block=-1;}
    *ptr=NULL;
}

static uint8_t *skin_load(const char *path, const char *block_name,
                           int expected,SceUID *block,TTTSkinFileStats *st){
    SceUID fd;
    uint8_t *buf;
    int size,count=0;
    st->stage=0;st->open_rc=0;st->file_size=-1;
    st->alloc_rc=-1;st->read_bytes=0;
    *block=-1;
    fd=sceIoOpen(path,PSP_O_RDONLY,0);
    if(fd<0){st->stage=1;st->open_rc=fd;return NULL;}
    size=sceIoLseek(fd,0,PSP_SEEK_END);
    st->file_size=size;
    if(size!=expected || sceIoLseek(fd,0,PSP_SEEK_SET)<0){
        st->stage=2;sceIoClose(fd);return NULL;
    }
    /* PSP user-memory partition 2. Allocating outside the module libc
     * heap avoids PRX CRT heap limitations and preserves stable .bss. */
    *block=sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER,
                  block_name,PSP_SMEM_Low,(SceSize)expected,NULL);
    st->alloc_rc=*block;
    if(*block<0){st->stage=3;sceIoClose(fd);return NULL;}
    buf=(uint8_t*)sceKernelGetBlockHeadAddr(*block);
    if(!buf){
        st->stage=4;sceIoClose(fd);
        sceKernelFreePartitionMemory(*block);*block=-1;
        return NULL;
    }
    while(count<expected){
        int n=sceIoRead(fd,buf+count,expected-count);
        if(n<=0)break;
        count+=n;
    }
    st->read_bytes=count;
    sceIoClose(fd);
    if(count!=expected){
        st->stage=5;
        sceKernelFreePartitionMemory(*block);*block=-1;
        return NULL;
    }
    sceKernelDcacheWritebackRange(buf,(size_t)expected);
    return buf;
}
/* Packs both files into the single 128-KiB block that PPSSPP already
 * proved it can allocate. The original BG is compacted in-place safely:
 * source row always >= destination row (top-to-bottom). */
static uint8_t *skin_load_font_into_background(uint8_t *bg,TTTSkinFileStats *st){
    SceUID fd;
    int size,count=0;
    unsigned y,x;
    uint16_t *pixels;
    st->stage=0;st->open_rc=0;st->file_size=-1;
    st->alloc_rc=0; /* Shares BG's block; no second allocation attempted. */
    st->read_bytes=0;
    if(!bg){st->stage=6;return NULL;} /* BG failed; no shared buffer. */
    fd=sceIoOpen(TTT_FONT_PATH,PSP_O_RDONLY,0);
    if(fd<0){st->stage=1;st->open_rc=fd;return NULL;}
    size=sceIoLseek(fd,0,PSP_SEEK_END);
    st->file_size=size;
    if(size!=(int)TTT_FONT_BYTES || sceIoLseek(fd,0,PSP_SEEK_SET)<0){
        st->stage=2;sceIoClose(fd);return NULL;
    }
    /* Move 256 BG rows into the first 160 rows, preserving their full width.
     * BG's last row maps to row 159. This does not require any scratch RAM. */
    pixels=(uint16_t*)bg;
    for(y=0;y<TTT_SKIN_BG_ROWS;y++){
        unsigned source=(y*255u)/(TTT_SKIN_BG_ROWS-1u);
        if(source==y)continue;
        for(x=0;x<256u;x++)pixels[y*256u+x]=pixels[source*256u+x];
    }
    /* The ASCII font occupies six rows of 16x16 cells = 96 pixels.
     * The last 32 unused source rows are irrelevant; read exactly 96 rows. */
    while(count<(int)TTT_SKIN_FONT_USED_BYTES){
        int n=sceIoRead(fd,bg+(TTT_SKIN_FONT_Y*256u*2u)+count,
                       (int)TTT_SKIN_FONT_USED_BYTES-count);
        if(n<=0)break;
        count+=n;
    }
    st->read_bytes=count;
    sceIoClose(fd);
    if(count!=(int)TTT_SKIN_FONT_USED_BYTES){st->stage=5;return NULL;}
    sceKernelDcacheWritebackRange(bg,TTT_MENU_BG_BYTES);
    return bg;
}
static void skin_load_all(void){
    /* Called once at first menu open; keep previous graceful failure. */
    skin_free(&ttt_skin_bg,&skin_bg_block);
    skin_free(&ttt_skin_font,&skin_font_block);
    ttt_skin_mask=0;
    ttt_skin_ready=0;
    ttt_skin_bg=skin_load(TTT_BG_PATH,"TTT_UI_ATLAS",
           (int)TTT_MENU_BG_BYTES,&skin_bg_block,&skin_stats.bg);
    ttt_skin_font=skin_load_font_into_background(ttt_skin_bg,&skin_stats.font);
    ttt_skin_mask=(ttt_skin_bg?1u:0u)|(ttt_skin_font?2u:0u);
    skin_stats.mask=ttt_skin_mask;
    ttt_skin_ready=(ttt_skin_bg && ttt_skin_font)?1:0;
    if(!ttt_skin_ready){
        skin_free(&ttt_skin_bg,&skin_bg_block);
        skin_free(&ttt_skin_font,&skin_font_block);
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
    sceGuTexImage(0,256,256,256,ttt_skin_bg); /* shared 256x256 atlas */
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
        if(ch!=' ')skin_sprite(x+i*dx,y,cw,dy,(at%16)*16,(int)TTT_SKIN_FONT_Y+(at/16)*16,11,14);
    }
}
static void skin_outline(int x,int y,int w,int h,uint32_t color){
    box(x,y,w,2,color);box(x,y+h-2,w,2,color);
    box(x,y,2,h,color);box(x+w-2,y,2,h,color);
}
/* All menu labels come from TTT_EXPANSION/lang/{pt,en,es}.txt;
 * defaults live in ui_text.c and are used for missing/invalid files. */
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
        if(ch!=' ')skin_sprite(x+i*step,y,glyph_w,glyph_h,(at%16)*16,(int)TTT_SKIN_FONT_Y+(at/16)*16,11,14);
    }
}
static void __attribute__((unused)) skin_frame_fallback(uint32_t bits){
    /* Installed PRX must NEVER silently show the old menu.  In the absence
     * of external texture blobs, keep a minimal NEW layout and show why.
     * The actual menu navigation remains functional. */
    int character=(int)(bits&3u),color=(int)((bits>>2)&3u);
    int player=(int)((bits>>8)&3u),focus=(int)((bits>>31)&1u),i;
    char num[3]="1P",label[4]="C1";
    box(8,8,464,256,C_DARK);panel(12,12,456,248,C_LIGHT,C_OUTER);
    txt(23,23,"TTT DLC REBUILD V2.0.3",1,C_WHITE);
    txt(25,50,ttt_text(0,TTT_TEXT_TEXTURE_ERROR),1,C_GOLD);
    for(i=0;i<3;i++){
        int x=22+153*i;
        panel(x,83,146,79,(character==i&&!focus)?C_GOLD:C_LIGHT,C_DARK);
        if(portrait_present[i])portrait(i,x+7,91,49,49);
        txt(x+60,98,ttt_text(0,(TTTTextKey)(TTT_TEXT_CHAR0_CARD_TOP+i*2)),1,C_WHITE);
        txt(x+60,112,ttt_text(0,(TTTTextKey)(TTT_TEXT_CHAR0_CARD_BOTTOM+i*2)),1,C_GOLD);
    }
    txt(25,177,ttt_text(0,TTT_TEXT_PLAYER_LABEL),1,C_WHITE);
    num[0]=(char)('1'+player);txt(90,177,num,1,C_GOLD);
    txt(184,177,ttt_text(0,TTT_TEXT_COLOR_LABEL),1,C_WHITE);
    label[1]=(char)('1'+color);txt(236,177,label,1,C_GOLD);
    txt(25,204,focus?"FOCO: TRAJES":"FOCO: PERSONAGENS",1,C_GOLD);
    txt(24,235,"ESQ/DIR: MOVER  CIMA/BAIXO: FOCO",1,C_WHITE);
}
static void __attribute__((unused)) render_menu_textured(uint32_t bits){
    int character=(int)(bits&3u),color=(int)((bits>>2)&3u);
    int tab=(int)((bits>>4)&3u),lang=(int)((bits>>6)&3u);
    int player=(int)((bits>>8)&3u),bridge=(int)((bits>>30)&1u);
    int focus=(int)((bits>>31)&1u),i;
    char number[3]="00",idline[9]="ID 00",slot[3]="1P",col[3]="C1";
    if(character>2)character=0;
    if(tab>3)tab=0;
    if(lang>2)lang=0;
    /* Layer 1: actual PSP 4444 background texture. */
    skin_bind_background();
    skin_sprite(0,0,480,272,0,0,256,(int)TTT_SKIN_BG_ROWS);
    sceGuDisable(GU_TEXTURE_2D);
    /* Layer 2: selective, geometry-aligned interaction highlights. */
    /* The source background has three original tab frames; overpaint that
     * region with FOUR equal dynamic tabs so controls and visuals agree. */
    box(13,60,454,29,C_DARK);
    for(i=0;i<4;i++){
        skin_outline(13+114*i,61,111,26,i==tab?C_GOLD:C_LIGHT);
    }
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
    skin_text_compact(23,22,ttt_text(lang,TTT_TEXT_TITLE),11,11,17,C_WHITE);
    skin_text(244,31,"V2.0.3",0,C_GOLD);
    for(i=0;i<4;i++){
        slot[0]=(char)('1'+i);
        skin_text(319+i*38,27,slot,0,i==player?C_GOLD:C_WHITE);
    }
    for(i=0;i<4;i++){
        skin_text_compact(18+i*114,i?66:70,ttt_text(lang,(TTTTextKey)(TTT_TEXT_TAB_CHARACTERS+i)),6,6,10,i==tab?C_GOLD:C_WHITE);
        if(i)skin_text_compact(18+i*114,78,ttt_text(lang,TTT_TEXT_COMING_SOON),5,5,8,C_SOFT);
    }
    if(tab==0){
        skin_text(20,101,ttt_text(lang,TTT_TEXT_EXTRA_CHARACTERS),0,C_WHITE);
        skin_text(207,101,"03/06",0,C_GOLD);
        skin_text(263,101,ttt_text(lang,TTT_TEXT_PREVIEW),0,C_WHITE);
        for(i=0;i<6;i++){
            int x=19+(i%3)*77,y=117+(i/3)*55;
            number[0]='0';number[1]=(char)('0'+i);
            skin_text_compact(x+51,y+3,number,5,5,9,i==character?C_GOLD:C_SOFT);
            if(i<3){
                /* Two lines make Super Saiyajin a real subtitle. */
                skin_text_compact(x+4,y+29,ttt_text(lang,(TTTTextKey)(TTT_TEXT_CHAR0_CARD_TOP+i*2)),5,5,9,i==character?C_GOLD:C_WHITE);
                skin_text_compact(x+4,y+39,ttt_text(lang,(TTTTextKey)(TTT_TEXT_CHAR0_CARD_BOTTOM+i*2)),4,5,8,i==character?C_GOLD:C_WHITE);
            }else{
                skin_text_compact(x+29,y+13,"+",9,9,14,C_LIGHT);
                skin_text_compact(x+11,y+39,ttt_text(lang,TTT_TEXT_FUTURE),5,5,8,C_SOFT);
            }
        }
        /* Preview description: exact requested names, SSJ subtitle below. */
        skin_text_compact(359,124,ttt_text(lang,(TTTTextKey)(TTT_TEXT_CHAR0_NAME+character)),6,6,11,C_WHITE);
        if(character==2)skin_text_compact(359,138,ttt_text(lang,TTT_TEXT_CHAR2_SUBTITLE),6,6,10,C_GOLD);
        idline[3]='0';idline[4]=(char)('0'+character);
        skin_text(360,155,idline,0,C_SOFT);
        slot[0]=(char)('1'+player);
        skin_text(359,169,slot,0,C_WHITE);
        skin_text_compact(359,182,ttt_text(lang,bridge?TTT_TEXT_STATUS_ACTIVE:((bits>>(10+player))&1u)?TTT_TEXT_STATUS_CONFIRMED:TTT_TEXT_STATUS_PREVIEW),6,6,10,bridge?C_GOLD:C_SOFT);
        for(i=0;i<4;i++){
            col[1]=(char)('1'+i);
            skin_text(282+i*48,207,col,0,(i==color)?C_GOLD:C_WHITE);
        }
    }else {
        TTTTextKey heading=(tab==1)?TTT_TEXT_MISSIONS_SOON_TITLE:(tab==2)?TTT_TEXT_MAPS_SOON_TITLE:TTT_TEXT_OPTIONS_SOON_TITLE;
        TTTTextKey detail=(tab==1)?TTT_TEXT_MISSIONS_SOON_DETAIL:(tab==2)?TTT_TEXT_MAPS_SOON_DETAIL:TTT_TEXT_OPTIONS_SOON_DETAIL;
        /* Pages remain visible but cannot activate experimental story/map hooks. */
        box(17,93,446,145,C_DARK);
        skin_outline(17,93,446,145,C_LIGHT);
        skin_text_compact(27,124,ttt_text(lang,heading),8,8,14,C_GOLD);
        skin_text_compact(27,161,ttt_text(lang,detail),6,6,10,C_WHITE);
    }
    /* Clear 3-line footer: never rely on hard-to-read long sentences. */
    skin_text_compact(22,233,ttt_text(lang,TTT_TEXT_FOOTER_MOVE),6,6,9,C_WHITE);
    skin_text_compact(22,244,ttt_text(lang,TTT_TEXT_FOOTER_CONFIRM),6,6,9,C_SOFT);
    skin_text_compact(22,255,ttt_text(lang,TTT_TEXT_FOOTER_CLOSE),6,6,9,C_SOFT);
    sceGuDisable(GU_BLEND);
    sceGuDisable(GU_TEXTURE_2D);
}


/* V47: 47 botoes + 4 jogadores + 4 Colors; nenhuma aba.
 * Todos os fundos sao retangulos GE (sem imagens adicionais nem BGM externa).
 * O preview mostra retratos ja existentes somente nos slots 0..2. */
static void menu47_label(int x,int y,const char *t,int step,uint32_t color){
    if(ttt_skin_ready)skin_text_compact(x,y,t,step,step+1,9,color);
    else txt(x,y,t,1,color);
}
static void render_menu47(uint32_t bits){
    int current=(int)(bits&63u),color=(int)((bits>>6)&3u);
    int player=(int)((bits>>8)&3u),focus=(int)((bits>>10)&1u);
    int sound=(int)((bits>>15)&1u),i;
    char slot[4]="01", cp[4]="J1", col[4]="C1";
    uint32_t selected=(bits>>(11+player))&1u;
    /* Nenhuma textura grande de fundo: apenas azul escuro uniforme. */
    sceGuDisable(GU_TEXTURE_2D);
    sceGuDisable(GU_BLEND);
    box(0,0,480,272,C_OUTER);
    box(7,7,466,257,C_DARK);
    box(10,51,286,181,C_BLACK);
    box(303,51,166,181,C_BLACK);
    box(10,239,459,1,C_BLUE);
    for(i=0;i<4;i++){
        int x=236+i*56;
        box(x,12,52,25,i==player?C_GOLD:C_LIGHT);
        box(x+2,14,48,21,i==player?C_BLUE:C_DARK);
    }
    for(i=0;i<47;i++){
        int x=14+(i%8)*35,y=56+(i/8)*29;
        int active=i==current;
        uint32_t border=active?(focus?C_WHITE:C_GOLD):(i<3?C_LIGHT:C_BLUE);
        box(x,y,31,26,border);
        box(x+2,y+2,27,22,active?C_BLUE:C_OUTER);
        if(i<3 && portrait_present[i]){
            portrait(i,x+3,y+3,25,20);
            box(x+3,y+17,25,6,C_BLACK);
        }
    }
    /* Color 1..4 right panel, visibly selectable. */
    for(i=0;i<4;i++){
        int x=315+i*37;
        box(x,194,33,29,(i==color)?(focus?C_GOLD:C_WHITE):C_BLUE);
        box(x+2,196,29,25,(i==color)?C_BLUE:C_DARK);
    }
    if(current<3 && portrait_present[current])portrait(current,340,97,82,76);
    /* Text glyphs are sampled from the packed 128 KiB atlas when available. */
    if(ttt_skin_ready)skin_bind_font();
    menu47_label(17,17,"TTT EXPANSION",7,C_WHITE);
    menu47_label(18,34,"47 SLOTS",6,C_SOFT);
    for(i=0;i<4;i++){cp[1]=(char)('1'+i);menu47_label(254+i*56,20,cp,7,i==player?C_GOLD:C_WHITE);}
    for(i=0;i<47;i++){
        int x=14+(i%8)*35,y=56+(i/8)*29;
        slot[0]=(char)('0'+((i+1)/10));slot[1]=(char)('0'+((i+1)%10));
        menu47_label(x+8,y+8,slot,6,(i==current)?C_GOLD:(i<3?C_WHITE:C_SOFT));
    }
    menu47_label(314,61,"PERSONAGEM",6,C_SOFT);
    if(current==0)menu47_label(313,76,"GOKU INICIO",6,C_GOLD);
    else if(current==1)menu47_label(313,76,"GOKU MEIO",6,C_GOLD);
    else if(current==2)menu47_label(313,76,"GOKU MEIO SSJ",6,C_GOLD);
    else {menu47_label(313,76,"SLOT VAZIO",6,C_SOFT);}
    menu47_label(313,85,current<3?"ID 00-02":"SEM ID",5,C_SOFT);
    menu47_label(313,180,selected?"CONFIRMADO":(current<3?"PRONTO":"EM ESPERA"),5,selected?C_GOLD:C_WHITE);
    for(i=0;i<4;i++){col[1]=(char)('1'+i);menu47_label(324+i*37,204,col,7,(i==color)?C_GOLD:C_WHITE);}
    menu47_label(15,246,"L/R JOGADOR  TRIANGULO COLORS  X OK  O VOLTAR",5,C_WHITE);
    menu47_label(15,257,sound?"SELECT MUSICA ON   QUADRADO LIMPAR":"SELECT MUSICA OFF  QUADRADO LIMPAR",5,C_SOFT);
    sceGuDisable(GU_BLEND);
    sceGuDisable(GU_TEXTURE_2D);
}

static void render_menu(uint32_t bits){render_menu47(bits);}

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
    ttt_text_load_external();
    portrait_present[0]=load_portrait(0);
    portrait_present[1]=load_portrait(1);
    portrait_present[2]=load_portrait(2);
    skin_load_all();
    gu_initialized=1;
    return 0;
}
void ttt_gpu_set_menu(uint32_t snapshot) {menu_snapshot=snapshot;}
void ttt_gpu_set_dlc(unsigned mission, uint32_t completed){
    dlc_mission_index=mission<TTT_DLC_MISSIONS?mission:0u;
    dlc_mission_completed=ttt_dlc_progress_sanitize(completed);
}
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
    skin_free(&ttt_skin_bg,&skin_bg_block);
    skin_free(&ttt_skin_font,&skin_font_block);
    ttt_skin_ready=0;ttt_skin_mask=0;
}
