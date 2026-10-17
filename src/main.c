/* TTT Expansion V0.4: GPU smoke test. EXPLICITLY EXPERIMENTAL. */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include <stddef.h>
#include "toggle.h"
#include "framecheck.h"
#include "gpu_probe.h"

PSP_MODULE_INFO("TTTExpGpuTest", PSP_MODULE_USER, 1, 5);
PSP_NO_CREATE_MAIN_THREAD();
#define LOG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt"
static volatile int g_running;
static SceUID g_worker=-1;
static void log_line(const char *s) {
    int n=0; SceUID fd;
    while(s[n])++n;
    fd=sceIoOpen(LOG_PATH, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND,0777);
    if(fd<0)return;
    sceIoWrite(fd,s,n); sceIoClose(fd);
}
static void put(char *d,int *p,int cap,const char *s) {
    while(*s && *p<cap-1)d[(*p)++]=*s++;
    d[*p]=0;
}
static void hex8(char *d,int *p,int cap,uint32_t v) {
    const char*h="0123456789ABCDEF";int j;
    put(d,p,cap,"0x");
    for(j=7;j>=0;--j)if(*p<cap-1)d[(*p)++]=h[(v>>(4*j))&15];
    d[*p]=0;
}
static void trace(const char*who,int rc,void *fb,int stride,int fmt) {
    char s[190];int p=0;s[0]=0;
    put(s,&p,sizeof(s),"[TTT V0.4] "); put(s,&p,sizeof(s),who);
    put(s,&p,sizeof(s)," rc="); hex8(s,&p,sizeof(s),(uint32_t)rc);
    put(s,&p,sizeof(s)," fb="); hex8(s,&p,sizeof(s),(uint32_t)(uintptr_t)fb);
    put(s,&p,sizeof(s)," stride=");hex8(s,&p,sizeof(s),(uint32_t)stride);
    put(s,&p,sizeof(s)," fmt=");hex8(s,&p,sizeof(s),(uint32_t)fmt);
    put(s,&p,sizeof(s),"\n");log_line(s);
}
static int diagnostic_thread(SceSize args, void *argp) {
    TTTToggle toggle; unsigned frame=0,probe_count=0;
    int gpu_ready=0;
    (void)args;(void)argp;
    ttt_toggle_init(&toggle);
    log_line("\n[TTT V0.4] module/thread OK. L+R+QUADRADO alterna menu.\n");
    log_line("[TTT V0.4] GPU probe: SOMENTE ao abrir, sem escrita CPU VRAM.\n");
    while(g_running) {
        SceCtrlData pad; void *fb=NULL;int stride=0,fmt=-1,rc,ge_rc,changed=0;
        if(sceDisplayWaitVblankStart()<0){sceKernelDelayThread(16000);continue;}
        if(sceCtrlPeekBufferPositive(&pad,1)>0){
            changed=ttt_toggle_frame(&toggle,pad.Buttons);
            if(changed){
                probe_count=0;
                log_line(toggle.open ? "[TTT V0.4] MENU ABERTO\n":"[TTT V0.4] MENU FECHADO\n");
            }
        }
        if(!toggle.open){frame++;continue;}
        if(!gpu_ready) {
            ge_rc=ttt_gpu_init();
            trace("GPU INIT",ge_rc,0,0,0);
            if(ge_rc<0){toggle.open=0;continue;}
            gpu_ready=1;
        }
        /* Draw 20 times/second at most, and stop after 20 seconds.
           If it interferes with the game, disable the plugin in PPSSPP. */
        if((frame%3u)==0u && probe_count<400u) {
            rc=sceDisplayGetFrameBuf(&fb,&stride,&fmt,PSP_DISPLAY_SETBUF_NEXTHSYNC);
            ge_rc=(rc<0) ? rc : ttt_gpu_frame(fb,stride,fmt);
            if(probe_count<4u || probe_count%60u==0u || ge_rc<0)
                trace("GPU DRAW",ge_rc,fb,stride,fmt);
            probe_count++;
            if(ge_rc<0) {log_line("[TTT V0.4] GPU ERRO, desabilitando desenho.\n");toggle.open=0;}
            if(probe_count==400u)log_line("[TTT V0.4] Limite de 20s atingido; feche e reabra para novo teste.\n");
        }
        frame++;
    }
    ttt_gpu_shutdown();
    sceKernelExitDeleteThread(0);
    return 0;
}
int module_start(SceSize args,void *argp){
    (void)args;(void)argp;
    log_line("\n[TTT V0.4] module_start OK - ULUS10537\n");
    g_running=1;
    g_worker=sceKernelCreateThread("TTTExpGpuWorker",diagnostic_thread,0x20,0x4000,0,NULL);
    if(g_worker<0){g_running=0;log_line("[TTT V0.4] ERROR: thread create\n");return 0;}
    if(sceKernelStartThread(g_worker,0,NULL)<0){g_running=0;log_line("[TTT V0.4] ERROR: thread start\n");}
    return 0;
}
int module_stop(SceSize args,void *argp){(void)args;(void)argp;g_running=0;return 0;}
