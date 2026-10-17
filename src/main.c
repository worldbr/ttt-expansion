/* TTT Expansion V0.5.3: patch verified JAL call sites, not HLE import stub. */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include <stddef.h>
#include "toggle.h"
#include "gpu_probe.h"
#include "frame_hook.h"
PSP_MODULE_INFO("TTTExpCallHook", PSP_MODULE_USER, 1, 10);
PSP_NO_CREATE_MAIN_THREAD();
#define LOG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt"
static volatile int g_running;
static SceUID g_worker=-1;
static void log_line(const char *s) {
    int n=0; SceUID fd;
    while(s[n])++n;
    fd=sceIoOpen(LOG_PATH,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND,0777);
    if(fd<0)return;
    sceIoWrite(fd,s,n); sceIoClose(fd);
}
static void append(char *d,int *p,int cap,const char *s) {
    while(*s && *p<cap-1)d[(*p)++]=*s++;
    d[*p]=0;
}
static void hex(char *d,int *p,int cap,uint32_t v) {
    const char*h="0123456789ABCDEF";int j;
    append(d,p,cap,"0x");
    for(j=7;j>=0;--j)if(*p<cap-1)d[(*p)++]=h[(v>>(4*j))&15];
    d[*p]=0;
}
static void probe_log(const char *phase) {
    TTTStubProbe x=ttt_frame_hook_probe();
    char s[270];int pos=0;s[0]=0;
    append(s,&pos,sizeof(s),"[TTT V0.5.3] STUB ");
    append(s,&pos,sizeof(s),phase);
    append(s,&pos,sizeof(s)," 08A66A80=");hex(s,&pos,sizeof(s),x.at_80);
    append(s,&pos,sizeof(s)," 08A66A84=");hex(s,&pos,sizeof(s),x.at_84);
    append(s,&pos,sizeof(s)," 08A66A88=");hex(s,&pos,sizeof(s),x.at_88);
    append(s,&pos,sizeof(s)," 08A66A8C=");hex(s,&pos,sizeof(s),x.at_8c);
    append(s,&pos,sizeof(s),"\n");log_line(s);
}
static void callsites_log(const char *phase) {
    TTTCallsProbe c=ttt_frame_hook_probe_calls();
    char s[280];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT V0.5.3] CALLS ");
    append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," 08A63A48=");hex(s,&p,sizeof(s),c.site_a);
    append(s,&p,sizeof(s)," 08A63B10=");hex(s,&p,sizeof(s),c.site_b);
    append(s,&p,sizeof(s)," 08A6408C=");hex(s,&p,sizeof(s),c.site_c);
    append(s,&p,sizeof(s)," interceptor=");hex(s,&p,sizeof(s),c.replacement);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void stats_log(void) {
    TTTFrameHookStats st=ttt_frame_hook_stats();char s[240];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT V0.5.3] HOOK installed=");hex(s,&p,sizeof(s),(uint32_t)st.installed);
    append(s,&p,sizeof(s)," calls=");hex(s,&p,sizeof(s),st.calls);
    append(s,&p,sizeof(s)," draws=");hex(s,&p,sizeof(s),st.draws);
    append(s,&p,sizeof(s)," buffer=");hex(s,&p,sizeof(s),st.last_buffer);
    append(s,&p,sizeof(s)," gpu_rc=");hex(s,&p,sizeof(s),(uint32_t)st.last_draw_result);
    append(s,&p,sizeof(s)," enabled=");hex(s,&p,sizeof(s),(uint32_t)st.draw_enabled);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static int diagnostic_thread(SceSize args,void *argp) {
    TTTToggle toggle;unsigned frames=0;int gpu_ready=0,armed=0;
    (void)args;(void)argp;
    ttt_toggle_init(&toggle);
    log_line("[TTT V0.5.3] Thread iniciada. L+R+QUADRADO instala/remove hook.\n");
    /* Wait until game starts drawing and the loader finishes linking imports. */
    sceKernelDelayThread(1500000);
    probe_log("APOS_START");
    callsites_log("APOS_START");
    while(g_running) {
        SceCtrlData pad;
        if(sceDisplayWaitVblankStart()<0){sceKernelDelayThread(16000);continue;}
        if(sceCtrlPeekBufferPositive(&pad,1)>0 && ttt_toggle_frame(&toggle,pad.Buttons)) {
            if(toggle.open) {
                int rc;
                log_line("[TTT V0.5.3] MENU ABERTO. Preparando GE.\n");
                probe_log("ANTES_GPU");
                if(!gpu_ready) {
                    rc=ttt_gpu_init();
                    if(rc<0){log_line("[TTT V0.5.3] GPU INIT ERRO.\n");toggle.open=0;}
                    else gpu_ready=1;
                    probe_log("DEPOIS_GPU");
                }
                if(gpu_ready) {
                    ttt_frame_hook_gpu_ready(1);
                    probe_log("ANTES_HOOK");
                    callsites_log("ANTES_HOOK");
                    rc=ttt_frame_hook_install();
                    if(rc<0) {
                        char s[80];int p=0;s[0]=0;
                        append(s,&p,sizeof(s),"[TTT V0.5.3] HOOK INSTALAR ERRO=");hex(s,&p,sizeof(s),(uint32_t)rc);
                        append(s,&p,sizeof(s),"\n");log_line(s);
                        probe_log("FALHA_HOOK");
                        callsites_log("FALHA_HOOK");
                        toggle.open=0;
                    } else {log_line("[TTT V0.5.3] HOOK INSTALADO.\n");callsites_log("INSTALADO");armed=1;}
                }
            } else {
                ttt_frame_hook_remove();armed=0;
                log_line("[TTT V0.5.3] MENU FECHADO. HOOK RESTAURADO.\n");callsites_log("RESTAURADO");
            }
            stats_log();
        }
        if((++frames%120u)==0u && armed)stats_log();
    }
    ttt_frame_hook_remove();
    ttt_gpu_shutdown();
    sceKernelExitDeleteThread(0);
    return 0;
}
int module_start(SceSize args,void *argp) {
    (void)args;(void)argp;
    log_line("\n[TTT V0.5.3] module_start OK - ULUS10537\n");
    g_running=1;
    g_worker=sceKernelCreateThread("TTTExpHookWorker",diagnostic_thread,0x20,0x4000,0,NULL);
    if(g_worker<0){g_running=0;log_line("[TTT V0.5.3] ERROR: thread create\n");return 0;}
    if(sceKernelStartThread(g_worker,0,NULL)<0){g_running=0;log_line("[TTT V0.5.3] ERROR: thread start\n");}
    return 0;
}
int module_stop(SceSize args,void *argp) {
    (void)args;(void)argp;
    g_running=0;
    ttt_frame_hook_remove();
    return 0;
}
