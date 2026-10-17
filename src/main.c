/* TTT Expansion V1.0C: patch verified JAL call sites, not HLE import stub. */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include <stddef.h>
#include "toggle.h"
#include "gpu_probe.h"
#include "frame_hook.h"
#include "menu_state.h"
#include "input_hook.h"
#include "battle_bridge.h"
#include "resource_hook.h"
#include "external_probe.h"
#include "external_stage.h"
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
    append(s,&pos,sizeof(s),"[TTT V1.0C] STUB ");
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
    append(s,&p,sizeof(s),"[TTT V1.0C] CALLS ");
    append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," 08A63A48=");hex(s,&p,sizeof(s),c.site_a);
    append(s,&p,sizeof(s)," 08A63B10=");hex(s,&p,sizeof(s),c.site_b);
    append(s,&p,sizeof(s)," 08A6408C=");hex(s,&p,sizeof(s),c.site_c);
    append(s,&p,sizeof(s)," interceptor=");hex(s,&p,sizeof(s),c.replacement);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void stats_log(void) {
    TTTFrameHookStats st=ttt_frame_hook_stats();char s[240];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT V1.0C] HOOK installed=");hex(s,&p,sizeof(s),(uint32_t)st.installed);
    append(s,&p,sizeof(s)," calls=");hex(s,&p,sizeof(s),st.calls);
    append(s,&p,sizeof(s)," draws=");hex(s,&p,sizeof(s),st.draws);
    append(s,&p,sizeof(s)," buffer=");hex(s,&p,sizeof(s),st.last_buffer);
    append(s,&p,sizeof(s)," gpu_rc=");hex(s,&p,sizeof(s),(uint32_t)st.last_draw_result);
    append(s,&p,sizeof(s)," enabled=");hex(s,&p,sizeof(s),(uint32_t)st.draw_enabled);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void gpu_guard_log(const char *phase) {
    TTTGeGuardStats g=ttt_gpu_guard_stats();
    char s[290];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT V1.0C] GE GUARD ");append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," init_save=");hex(s,&p,sizeof(s),(uint32_t)g.init_save);
    append(s,&p,sizeof(s)," init_restore=");hex(s,&p,sizeof(s),(uint32_t)g.init_restore);
    append(s,&p,sizeof(s)," frame_save=");hex(s,&p,sizeof(s),(uint32_t)g.frame_save);
    append(s,&p,sizeof(s)," frame_restore=");hex(s,&p,sizeof(s),(uint32_t)g.frame_restore);
    append(s,&p,sizeof(s)," attempts=");hex(s,&p,sizeof(s),g.attempts);
    append(s,&p,sizeof(s)," restores=");hex(s,&p,sizeof(s),g.restores);
    append(s,&p,sizeof(s)," failed=");hex(s,&p,sizeof(s),(uint32_t)g.failed);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void input_log(const char *phase) {
    TTTInputHookStats st=ttt_input_hook_stats();char s[300];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT V1.0C] INPUT ");append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," installed=");hex(s,&p,sizeof(s),(uint32_t)st.installed);
    append(s,&p,sizeof(s)," site=");hex(s,&p,sizeof(s),st.site_opcode);
    append(s,&p,sizeof(s)," target=");hex(s,&p,sizeof(s),st.target);
    append(s,&p,sizeof(s)," calls=");hex(s,&p,sizeof(s),st.calls);
    append(s,&p,sizeof(s)," muted=");hex(s,&p,sizeof(s),st.muted);
    append(s,&p,sizeof(s)," raw=");hex(s,&p,sizeof(s),st.last_raw);
    append(s,&p,sizeof(s)," forwarded=");hex(s,&p,sizeof(s),st.last_sent);
    append(s,&p,sizeof(s)," quarantine=");hex(s,&p,sizeof(s),(uint32_t)st.quarantine);
    append(s,&p,sizeof(s)," open=");hex(s,&p,sizeof(s),(uint32_t)st.open);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void bridge_log(const char *phase){
    TTTBridgeStats st=ttt_bridge_stats();char b[300];int p=0;b[0]=0;
    append(b,&p,sizeof(b),"[TTT V1.0C] BRIDGE ");append(b,&p,sizeof(b),phase);
    append(b,&p,sizeof(b)," installed=");hex(b,&p,sizeof(b),(uint32_t)st.installed);
    append(b,&p,sizeof(b)," error=");hex(b,&p,sizeof(b),(uint32_t)st.last_error);
    append(b,&p,sizeof(b)," A00=");hex(b,&p,sizeof(b),st.a);
    append(b,&p,sizeof(b)," B00=");hex(b,&p,sizeof(b),st.b);
    append(b,&p,sizeof(b)," Videl2=");hex(b,&p,sizeof(b),st.videl_2);
    append(b,&p,sizeof(b)," End=");hex(b,&p,sizeof(b),st.videl_end);
    append(b,&p,sizeof(b)," limit=");hex(b,&p,sizeof(b),st.limit);
    append(b,&p,sizeof(b),"\n");log_line(b);
}
static void resource_log(const char *phase){
    TTTResourceHookStats st=ttt_res_hook_stats();
    char b[400];int p=0;b[0]=0;
    append(b,&p,sizeof(b),"[TTT V1.0C] RESOURCE ");append(b,&p,sizeof(b),phase);
    append(b,&p,sizeof(b)," installed=");hex(b,&p,sizeof(b),(uint32_t)st.installed);
    append(b,&p,sizeof(b)," error=");hex(b,&p,sizeof(b),(uint32_t)st.last_error);
    append(b,&p,sizeof(b)," checked=");hex(b,&p,sizeof(b),st.calls_checked);
    append(b,&p,sizeof(b)," unchecked=");hex(b,&p,sizeof(b),st.calls_unchecked);
    append(b,&p,sizeof(b)," redirects=");hex(b,&p,sizeof(b),st.redirects);
    append(b,&p,sizeof(b)," last_id=");hex(b,&p,sizeof(b),st.last_id);
    append(b,&p,sizeof(b)," last_offset=");hex(b,&p,sizeof(b),st.last_variant);
    append(b,&p,sizeof(b)," old=");hex(b,&p,sizeof(b),st.last_original);
    append(b,&p,sizeof(b)," new=");hex(b,&p,sizeof(b),st.last_result);
    append(b,&p,sizeof(b)," size_1194=");hex(b,&p,sizeof(b),st.resource_size);
    append(b,&p,sizeof(b)," size_goku=");hex(b,&p,sizeof(b),st.original_size);
    append(b,&p,sizeof(b)," record_id=");hex(b,&p,sizeof(b),st.resource_id);
    append(b,&p,sizeof(b)," pack_magic=");hex(b,&p,sizeof(b),st.pack_magic);
    append(b,&p,sizeof(b)," count=");hex(b,&p,sizeof(b),st.pack_count);
    append(b,&p,sizeof(b),"\n");log_line(b);
    /* Unlike last_id/old/new, this snapshot is not overwritten by normal
     * resource requests and tells us exactly what the redirect returned. */
    if(st.match_count){
        char m[260];int q=0;m[0]=0;
        append(m,&q,sizeof(m),"[TTT V1.0C] RESOURCE MATCH count=");
        hex(m,&q,sizeof(m),st.match_count);
        append(m,&q,sizeof(m)," id=");hex(m,&q,sizeof(m),st.match_id);
        append(m,&q,sizeof(m)," offset=");hex(m,&q,sizeof(m),st.match_offset);
        append(m,&q,sizeof(m)," old=");hex(m,&q,sizeof(m),st.match_original);
        append(m,&q,sizeof(m)," new=");hex(m,&q,sizeof(m),st.match_redirected);
        append(m,&q,sizeof(m)," path=");hex(m,&q,sizeof(m),st.match_path);
        append(m,&q,sizeof(m),"\n");log_line(m);
    }
}
static int diagnostic_thread(SceSize args,void *argp) {
    TTTToggle toggle;TTTMenuState menu;unsigned frames=0;int gpu_ready=0,armed=0;
    (void)args;(void)argp;
    ttt_toggle_init(&toggle);
    ttt_menu_init(&menu);
    log_line("[TTT V1.0C] Thread iniciada. X em Goku Early 1P arma ID00 e CHR1194 Color1; QUADRADO desfaz.\n");
    /* Wait until game starts drawing and the loader finishes linking imports. */
    sceKernelDelayThread(1500000);
    probe_log("APOS_START");
    callsites_log("APOS_START");
    bridge_log("APOS_START");resource_log("APOS_START");
    {
        int rc=ttt_input_hook_install();
        if(rc<0) {
            char s[120];int p=0;s[0]=0;
            append(s,&p,sizeof(s),"[TTT V1.0C] INPUT INSTALAR ERRO=");
            hex(s,&p,sizeof(s),(uint32_t)rc);
            append(s,&p,sizeof(s)," (controle do jogo NAO bloqueado)\n");log_line(s);
        } else log_line("[TTT V1.0C] INPUT HOOK INSTALADO.\n");
        input_log("APOS_INSTALAR");
    }
    while(g_running) {
        SceCtrlData pad;
        if(sceDisplayWaitVblankStart()<0){sceKernelDelayThread(16000);continue;}
        if(sceCtrlPeekBufferPositive(&pad,1)>0) {
        int toggled=ttt_toggle_frame(&toggle,pad.Buttons);
        if(toggled){
            ttt_input_hook_menu(toggle.open);
            ttt_menu_rebase(&menu,pad.Buttons);
            if(toggle.open) {
                int rc;
                log_line("[TTT V1.0C] MENU ABERTO. Preparando GE.\n");
                probe_log("ANTES_GPU");
                if(!gpu_ready) {
                    rc=ttt_gpu_init();
                    if(rc<0){log_line("[TTT V1.0C] GPU INIT ERRO.\n");toggle.open=0;ttt_input_hook_menu(0);}
                    else gpu_ready=1;
                    probe_log("DEPOIS_GPU");gpu_guard_log("INIT");
                }
                if(gpu_ready) {
                    {
                        char b[96];int q=0;b[0]=0;
                        append(b,&q,sizeof(b),"[TTT V1.0C] PORTRAITS bits=");
                        hex(b,&q,sizeof(b),(uint32_t)ttt_gpu_portrait_status());
                        append(b,&q,sizeof(b)," (1=EARLY 2=MID)\n");
                        log_line(b);
                    }
                    ttt_frame_hook_gpu_ready(1);
                    ttt_gpu_set_menu(ttt_menu_snapshot(&menu)|((uint32_t)ttt_bridge_is_active()<<25));
                    probe_log("ANTES_HOOK");
                    callsites_log("ANTES_HOOK");
                    rc=ttt_frame_hook_install();
                    if(rc<0) {
                        char s[80];int p=0;s[0]=0;
                        append(s,&p,sizeof(s),"[TTT V1.0C] HOOK INSTALAR ERRO=");hex(s,&p,sizeof(s),(uint32_t)rc);
                        append(s,&p,sizeof(s),"\n");log_line(s);
                        probe_log("FALHA_HOOK");
                        callsites_log("FALHA_HOOK");
                        toggle.open=0;
                        ttt_input_hook_menu(0);
                    } else {log_line("[TTT V1.0C] HOOK INSTALADO.\n");callsites_log("INSTALADO");armed=1;}
                }
            } else {
                ttt_frame_hook_remove();armed=0;gpu_guard_log("CLOSE");
                log_line("[TTT V1.0C] MENU FECHADO. HOOK RESTAURADO.\n");callsites_log("RESTAURADO");
            }
            stats_log();
            input_log("TOGGLE");
        } else if(toggle.open) {
            int ev=ttt_menu_buttons(&menu,pad.Buttons);
            ttt_gpu_set_menu(ttt_menu_snapshot(&menu)|((uint32_t)ttt_bridge_is_active()<<25));
            if(ev==TTT_EVENT_CONFIRM) {
                char line[230];int p=0;line[0]=0;
                append(line,&p,sizeof(line),"[TTT V1.0C] CONFIRMACAO personagem=");
                append(line,&p,sizeof(line),menu.character?"GOKU_MID":"GOKU_EARLY");
                append(line,&p,sizeof(line)," ID=");
                append(line,&p,sizeof(line),menu.character?"01":"00");
                append(line,&p,sizeof(line)," JOGADOR=");
                append(line,&p,sizeof(line),menu.player==0?"1P":menu.player==1?"2P":menu.player==2?"3P":"4P");
                append(line,&p,sizeof(line)," color=");
                append(line,&p,sizeof(line),menu.color==0?"1":menu.color==1?"2":menu.color==2?"3":"4");
                if(menu.tab==0 && menu.player==0 && menu.character==0 && menu.color==0) {
                    /* External loader milestone C: retain the complete CHR
                     * in the plugin's own RAM. No game loader injection yet. */
                    {
                        TTTExternalStage ex;
                        int erc=ttt_external_stage_load(TTT_EXTERNAL_EARLY_COLOR1,TTT_EXTERNAL_TEST_SIZE,&ex);
                        char e[360];int ep=0;e[0]=0;
                        append(e,&ep,sizeof(e),"[TTT V1.0C] EXTERNAL STAGE rc=");
                        hex(e,&ep,sizeof(e),(uint32_t)erc);
                        append(e,&ep,sizeof(e)," expected=");hex(e,&ep,sizeof(e),ex.reference_size);
                        append(e,&ep,sizeof(e)," size=");hex(e,&ep,sizeof(e),ex.actual_size);
                        append(e,&ep,sizeof(e)," read=");hex(e,&ep,sizeof(e),ex.bytes_read);
                        append(e,&ep,sizeof(e)," first4=");hex(e,&ep,sizeof(e),ex.first4);
                        append(e,&ep,sizeof(e)," fnv1a=");hex(e,&ep,sizeof(e),ex.fnv1a);
                        append(e,&ep,sizeof(e)," address=");hex(e,&ep,sizeof(e),ex.address);
                        append(e,&ep,sizeof(e)," retained=");hex(e,&ep,sizeof(e),ex.retained);
                        append(e,&ep,sizeof(e)," (SOMENTE RAM DO PLUGIN; BATALHA USA ISO 1194)\n");
                        log_line(e);
                    }
                    int rc=ttt_res_hook_install();
                    if(rc==0){
                        rc=ttt_bridge_install();
                        if(rc<0)ttt_res_hook_remove();
                    }
                    append(line,&p,sizeof(line),rc==0?" BRIDGE ID00 + CHR1194 ISO (Videl forma 2)":" BRIDGE/RECURSO ERRO=");
                    if(rc<0)hex(line,&p,sizeof(line),(uint32_t)rc);
                }else append(line,&p,sizeof(line)," PREVIA - TESTE CHR SOMENTE EARLY 1P COLOR1");
                append(line,&p,sizeof(line),"\n");log_line(line);
                bridge_log("CONFIRMAR");resource_log("CONFIRMAR");
                ttt_gpu_set_menu(ttt_menu_snapshot(&menu)|((uint32_t)ttt_bridge_is_active()<<25));
            } else if(ev==TTT_EVENT_BRIDGE_RESET) {
                ttt_external_stage_release();
                ttt_res_hook_remove();
                int rc=ttt_bridge_remove();
                log_line(rc==0?"[TTT V1.0C] BRIDGE DESARMADA PELO QUADRADO.\n":"[TTT V1.0C] AVISO BRIDGE RESTAURACAO INCOMPLETA.\n");
                bridge_log("DESARMAR");resource_log("DESARMAR");
                ttt_gpu_set_menu(ttt_menu_snapshot(&menu)|((uint32_t)ttt_bridge_is_active()<<25));
            } else if(ev==TTT_EVENT_CLOSE) {
                toggle.open=0;
                ttt_input_hook_menu(0);
                ttt_frame_hook_remove();armed=0;gpu_guard_log("CLOSE");
                log_line("[TTT V1.0C] MENU FECHADO CIRCULO. HOOK RESTAURADO.\n");
                callsites_log("RESTAURADO");
            }
        } else {
            ttt_menu_rebase(&menu,pad.Buttons);
        }
        }
        if((++frames%120u)==0u){
            if(armed){stats_log();gpu_guard_log("STATUS");input_log("STATUS");}
            if(ttt_res_hook_stats().installed)resource_log("STATUS");
        }
    }
    ttt_external_stage_release();
    ttt_res_hook_remove();
    ttt_bridge_remove();
    ttt_input_hook_remove();
    ttt_frame_hook_remove();
    ttt_gpu_shutdown();
    sceKernelExitDeleteThread(0);
    return 0;
}
int module_start(SceSize args,void *argp) {
    (void)args;(void)argp;
    log_line("\n[TTT V1.0C] module_start OK - ULUS10537\n");
    g_running=1;
    g_worker=sceKernelCreateThread("TTTExpMenuWorker",diagnostic_thread,0x20,0x4000,0,NULL);
    if(g_worker<0){g_running=0;log_line("[TTT V1.0C] ERROR: thread create\n");return 0;}
    if(sceKernelStartThread(g_worker,0,NULL)<0){g_running=0;log_line("[TTT V1.0C] ERROR: thread start\n");}
    return 0;
}
int module_stop(SceSize args,void *argp) {
    (void)args;(void)argp;
    g_running=0;
    /* Worker releases staged data after seeing g_running=0. */
    ttt_res_hook_remove();
    ttt_bridge_remove();
    ttt_input_hook_remove();
    ttt_frame_hook_remove();
    return 0;
}
