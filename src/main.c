/*
 * TTT Expansion diagnostic PRX 0.3.4 - PPSSPP, ULUS10537.
 * Objective: determine why a working PRX/control thread shows no overlay.
 * Safe: no patches to EBOOT, IDs, gameplay, packfile or ISO.
 * This is a framebuffer experiment; not a reliable rendering hook yet.
 */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include <stddef.h>
#include "overlay.h"
#include "toggle.h"
#include "framecheck.h"

PSP_MODULE_INFO("TTTExpDiag", PSP_MODULE_USER, 1, 4);
PSP_NO_CREATE_MAIN_THREAD();

#define LOG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt"
static volatile int g_running;
static SceUID g_worker = -1;
static void log_line(const char *s) {
    int n=0; SceUID fd;
    while(s[n]) ++n;
    fd=sceIoOpen(LOG_PATH,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND,0777);
    if(fd<0) return;
    sceIoWrite(fd,s,n);
    sceIoClose(fd);
}

/* Use tiny formatting helpers to avoid stdio & runtime differences. */
static void put_str(char *dst,int *pos,int cap,const char *s) {
    while(*s && *pos<cap-1) dst[(*pos)++]=*s++;
    dst[*pos]=0;
}
static void put_hex(char *dst,int *pos,int cap,uint32_t value) {
    static const char h[]="0123456789ABCDEF";
    int i;
    put_str(dst,pos,cap,"0x");
    for(i=7;i>=0;i--) {
        if(*pos<cap-1)dst[(*pos)++]=h[(value>>(i*4))&15];
    }
    dst[*pos]=0;
}
static void put_dec(char *dst,int *pos,int cap,int value) {
    unsigned int u;
    char digits[16]; int k=0;
    if(value<0){put_str(dst,pos,cap,"-"); u=(unsigned int)(-(value+1))+1u;}
    else u=(unsigned int)value;
    do{digits[k++]=(char)('0'+u%10);u/=10;}while(u && k<16);
    while(k>0 && *pos<cap-1)dst[(*pos)++]=digits[--k];
    dst[*pos]=0;
}
/* Known marker pixel: x=12,y=10 (gold, upper bar drawn in overlay.c). */
static int marker_present(void *ptr,int stride,int fmt) {
    uintptr_t alias;
    if(ttt_vram_check((uintptr_t)ptr,stride,fmt)!=0) return -1;
    alias = ttt_vram_uncached((uintptr_t)ptr);
    if(!alias) return -1;
    if(fmt==3) {
        volatile uint32_t *p=(volatile uint32_t *)alias;
        return p[10*stride+12]==UINT32_C(0xff00ccff) ? 1:0;
    } else {
        /* overlay.c pixel16 encodings for gold (255,204,0). */
        uint16_t gold = (fmt==0) ? (uint16_t)0x067f :
                        (fmt==1) ? (uint16_t)0x833f : (uint16_t)0xf0cf;
        volatile uint16_t *p=(volatile uint16_t *)alias;
        return p[10*stride+12]==gold ? 1:0;
    }
}
static void trace_buffer(const char *which,int status,void *fb,int stride,int fmt,
                         int check,int wrote,int before,int after,int open) {
    char s[288];int p=0;s[0]=0;
    put_str(s,&p,sizeof s,"[TTT V0.3.4] ");put_str(s,&p,sizeof s,which);
    put_str(s,&p,sizeof s," rc=");put_hex(s,&p,sizeof s,(uint32_t)status);
    put_str(s,&p,sizeof s," fb=");put_hex(s,&p,sizeof s,(uint32_t)(uintptr_t)fb);
    put_str(s,&p,sizeof s," uncached=");put_hex(s,&p,sizeof s,(uint32_t)ttt_vram_uncached((uintptr_t)fb));
    put_str(s,&p,sizeof s," stride=");put_dec(s,&p,sizeof s,stride);
    put_str(s,&p,sizeof s," fmt=");put_dec(s,&p,sizeof s,fmt);
    put_str(s,&p,sizeof s," check=");put_dec(s,&p,sizeof s,check);
    put_str(s,&p,sizeof s," drawn=");put_dec(s,&p,sizeof s,wrote);
    put_str(s,&p,sizeof s," before=");put_dec(s,&p,sizeof s,before);
    put_str(s,&p,sizeof s," after=");put_dec(s,&p,sizeof s,after);
    put_str(s,&p,sizeof s," open=");put_dec(s,&p,sizeof s,open);
    put_str(s,&p,sizeof s,"\n");log_line(s);
}
static int draw_to_buffer(int status,void *fb,int stride,int fmt,int open,
                          int *before,int *after) {
    uintptr_t alias;
    *before=marker_present(fb,stride,fmt);
    *after=-1;
    if(status<0 || ttt_vram_check((uintptr_t)fb,stride,fmt)!=0)return 0;
    alias=ttt_vram_uncached((uintptr_t)fb);
    if(!alias)return 0;
    ttt_draw_overlay((void *)alias,stride,fmt,open);
    *after=marker_present(fb,stride,fmt);
    return 1;
}
static int diagnostic_thread(SceSize args,void *argp) {
    TTTToggle toggle;unsigned frame=0;
    (void)args;(void)argp;
    ttt_toggle_init(&toggle);
    log_line("[TTT V0.3.4] Thread iniciado. L+R+Quadrado abre/fecha.\n");
    log_line("[TTT V0.3.4] Diagnostico framebuffer: CUR=sync0, NEXT=sync1, check=0 OK, alias=0x440xxxxx.\n");
    while(g_running) {
        SceCtrlData pad;void *cur=0,*next=0;
        int cw=0,cf=-1,nw=0,nf=-1;
        int b0=-1,a0=-1,b1=-1,a1=-1;
        int rc0,rc1,wrote0,wrote1=0,changed=0;
        if(sceDisplayWaitVblankStart()<0){sceKernelDelayThread(16000);continue;}
        if(sceCtrlPeekBufferPositive(&pad,1)>0){
            changed=ttt_toggle_frame(&toggle,pad.Buttons);
            if(changed) log_line(toggle.open ? "[TTT V0.3.4] MENU ABERTO\n":"[TTT V0.3.4] MENU FECHADO\n");
        }
        rc0=sceDisplayGetFrameBuf(&cur,&cw,&cf,PSP_DISPLAY_SETBUF_NEXTHSYNC);
        rc1=sceDisplayGetFrameBuf(&next,&nw,&nf,PSP_DISPLAY_SETBUF_NEXTVSYNC);
        wrote0=draw_to_buffer(rc0,cur,cw,cf,toggle.open,&b0,&a0);
        if(next!=cur || nw!=cw || nf!=cf)
            wrote1=draw_to_buffer(rc1,next,nw,nf,toggle.open,&b1,&a1);
        else { wrote1=wrote0; b1=b0; a1=a0; }
        /* Sample early, on each toggle, and ~4 seconds later. Avoid log spam. */
        if(frame==0 || changed || frame%240u==0u) {
            trace_buffer("CUR",rc0,cur,cw,cf,ttt_vram_check((uintptr_t)cur,cw,cf),wrote0,b0,a0,toggle.open);
            trace_buffer("NEXT",rc1,next,nw,nf,ttt_vram_check((uintptr_t)next,nw,nf),wrote1,b1,a1,toggle.open);
        }
        frame++;
    }
    log_line("[TTT V0.3.4] Thread encerrado.\n");
    sceKernelExitDeleteThread(0);
    return 0;
}
int module_start(SceSize args,void *argp) {
    (void)args;(void)argp;
    log_line("\n[TTT V0.3.4] module_start OK - ULUS10537\n");
    g_running=1;
    g_worker=sceKernelCreateThread("TTTExpDiagWorker",diagnostic_thread,0x20,0x3000,0,NULL);
    if(g_worker<0){g_running=0;log_line("[TTT V0.3.4] ERRO criando thread\n");return 0;}
    if(sceKernelStartThread(g_worker,0,NULL)<0){g_running=0;log_line("[TTT V0.3.4] ERRO iniciando thread\n");}
    return 0;
}
int module_stop(SceSize args,void *argp){(void)args;(void)argp;g_running=0;return 0;}
