/* TTT Expansion V2.0.0: patch verified JAL call sites, not HLE import stub. */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include <stddef.h>
#include "toggle.h"
#include "gpu_probe.h"
#include "ui_text.h"
#include "frame_hook.h"
#include "menu_state.h"
#include "menu_audio.h"
#include "input_hook.h"
#include "battle_bridge.h"
#include "resource_hook.h"
#include "slot_plan.h"
#include "actor_watch.h"
#include "actor_inject.h"
#include "dlc_story.h"
#include "dlc_progress.h"
PSP_MODULE_INFO("TTTExpCallHook", PSP_MODULE_USER, 1, 12);
PSP_NO_CREATE_MAIN_THREAD();
#define LOG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt"
/* Assinaturas exatas dos dois resolvedores patched no ELF V0.3.
 * Sem isso NUNCA injetar ID 00-02; o jogo original nao sabe carregar CHR extra. */
static int native_v03_ready(void){
    volatile const uint32_t *c=(volatile const uint32_t*)(uintptr_t)0x089C6CA0u;
    volatile const uint32_t *u=(volatile const uint32_t*)(uintptr_t)0x089C6D60u;
    volatile const uint32_t *n=(volatile const uint32_t*)(uintptr_t)0x08AF3244u;
    return *c==UINT32_C(0x2C8800A1) && *u==UINT32_C(0x3C0308A7) && *n==UINT32_C(0x3734);
}
static volatile int g_running;
static uint32_t g_mission_progress;
/* Static: avoid a 5 KB actor-watch ring on the 16 KB worker stack. */
static TTTActorWatch g_actor_watch;
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
    append(s,&pos,sizeof(s),"[TTT DLC V2.0.3] STUB ");
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
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] CALLS ");
    append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," 08A63A48=");hex(s,&p,sizeof(s),c.site_a);
    append(s,&p,sizeof(s)," 08A63B10=");hex(s,&p,sizeof(s),c.site_b);
    append(s,&p,sizeof(s)," 08A6408C=");hex(s,&p,sizeof(s),c.site_c);
    append(s,&p,sizeof(s)," interceptor=");hex(s,&p,sizeof(s),c.replacement);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void stats_log(void) {
    TTTFrameHookStats st=ttt_frame_hook_stats();char s[240];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] HOOK installed=");hex(s,&p,sizeof(s),(uint32_t)st.installed);
    append(s,&p,sizeof(s)," calls=");hex(s,&p,sizeof(s),st.calls);
    append(s,&p,sizeof(s)," draws=");hex(s,&p,sizeof(s),st.draws);
    append(s,&p,sizeof(s)," buffer=");hex(s,&p,sizeof(s),st.last_buffer);
    append(s,&p,sizeof(s)," gpu_rc=");hex(s,&p,sizeof(s),(uint32_t)st.last_draw_result);
    append(s,&p,sizeof(s)," enabled=");hex(s,&p,sizeof(s),(uint32_t)st.draw_enabled);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void gpu_guard_log(const char *phase) {
    TTTGeGuardStats g=ttt_gpu_guard_stats();
    char s[490];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] GE GUARD ");append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," init_save=");hex(s,&p,sizeof(s),(uint32_t)g.init_save);
    append(s,&p,sizeof(s)," init_restore=");hex(s,&p,sizeof(s),(uint32_t)g.init_restore);
    append(s,&p,sizeof(s)," frame_save=");hex(s,&p,sizeof(s),(uint32_t)g.frame_save);
    append(s,&p,sizeof(s)," frame_restore=");hex(s,&p,sizeof(s),(uint32_t)g.frame_restore);
    append(s,&p,sizeof(s)," attempts=");hex(s,&p,sizeof(s),g.attempts);
    append(s,&p,sizeof(s)," restores=");hex(s,&p,sizeof(s),g.restores);
    append(s,&p,sizeof(s)," failed=");hex(s,&p,sizeof(s),(uint32_t)g.failed);
    append(s,&p,sizeof(s)," primitives=");hex(s,&p,sizeof(s),g.primitives);
    append(s,&p,sizeof(s)," peak=");hex(s,&p,sizeof(s),g.peak_primitives);
    append(s,&p,sizeof(s)," skipped=");hex(s,&p,sizeof(s),g.skipped_primitives);
    append(s,&p,sizeof(s)," guard_fault=");hex(s,&p,sizeof(s),(uint32_t)g.canary_fault);
    append(s,&p,sizeof(s)," list_bytes=");hex(s,&p,sizeof(s),g.list_bytes);
    append(s,&p,sizeof(s),"\n");log_line(s);
}
static void input_log(const char *phase) {
    TTTInputHookStats st=ttt_input_hook_stats();char s[300];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] INPUT ");append(s,&p,sizeof(s),phase);
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
    append(b,&p,sizeof(b),"[TTT DLC V2.0.3] BRIDGE ");append(b,&p,sizeof(b),phase);
    append(b,&p,sizeof(b)," installed=");hex(b,&p,sizeof(b),(uint32_t)st.installed);
    append(b,&p,sizeof(b)," error=");hex(b,&p,sizeof(b),(uint32_t)st.last_error);
    append(b,&p,sizeof(b)," A00=");hex(b,&p,sizeof(b),st.a);
    append(b,&p,sizeof(b)," B00=");hex(b,&p,sizeof(b),st.b);
    append(b,&p,sizeof(b)," A01=");hex(b,&p,sizeof(b),st.a1);
    append(b,&p,sizeof(b)," B01=");hex(b,&p,sizeof(b),st.b1);
    append(b,&p,sizeof(b)," A02=");hex(b,&p,sizeof(b),st.a2);
    append(b,&p,sizeof(b)," B02=");hex(b,&p,sizeof(b),st.b2);
    append(b,&p,sizeof(b)," Goku5=");hex(b,&p,sizeof(b),st.goku_5);
    append(b,&p,sizeof(b)," GokuEnd=");hex(b,&p,sizeof(b),st.goku_end);
    append(b,&p,sizeof(b)," Videl2=");hex(b,&p,sizeof(b),st.videl_2);
    append(b,&p,sizeof(b)," VidelEnd=");hex(b,&p,sizeof(b),st.videl_end);
    append(b,&p,sizeof(b)," limit=");hex(b,&p,sizeof(b),st.limit);
    append(b,&p,sizeof(b),"\n");log_line(b);
}
static void resource_log(const char *phase){
    TTTResourceHookStats st=ttt_res_hook_stats();
    char b[400];int p=0;b[0]=0;
    append(b,&p,sizeof(b),"[TTT DLC V2.0.3] RESOURCE ");append(b,&p,sizeof(b),phase);
    append(b,&p,sizeof(b)," installed=");hex(b,&p,sizeof(b),(uint32_t)st.installed);
    append(b,&p,sizeof(b)," error=");hex(b,&p,sizeof(b),(uint32_t)st.last_error);
    append(b,&p,sizeof(b)," checked=");hex(b,&p,sizeof(b),st.calls_checked);
    append(b,&p,sizeof(b)," unchecked=");hex(b,&p,sizeof(b),st.calls_unchecked);
    append(b,&p,sizeof(b)," redirects=");hex(b,&p,sizeof(b),st.redirects);
    append(b,&p,sizeof(b)," last_id=");hex(b,&p,sizeof(b),st.last_id);
    append(b,&p,sizeof(b)," last_offset=");hex(b,&p,sizeof(b),st.last_variant);
    append(b,&p,sizeof(b)," old=");hex(b,&p,sizeof(b),st.last_original);
    append(b,&p,sizeof(b)," new=");hex(b,&p,sizeof(b),st.last_result);
    append(b,&p,sizeof(b)," size_14098=");hex(b,&p,sizeof(b),st.resource_size);
    append(b,&p,sizeof(b)," size_goku=");hex(b,&p,sizeof(b),st.original_size);
    append(b,&p,sizeof(b)," record_id=");hex(b,&p,sizeof(b),st.resource_id);
    append(b,&p,sizeof(b)," off_14098=");hex(b,&p,sizeof(b),st.resource_data_offset);
    append(b,&p,sizeof(b)," off_goku=");hex(b,&p,sizeof(b),st.original_data_offset);
    append(b,&p,sizeof(b)," pack_magic=");hex(b,&p,sizeof(b),st.pack_magic);
    append(b,&p,sizeof(b)," count=");hex(b,&p,sizeof(b),st.pack_count);
    append(b,&p,sizeof(b)," assets12=");hex(b,&p,sizeof(b),st.available_mask);
    append(b,&p,sizeof(b),"\n");log_line(b);
    /* Unlike last_id/old/new, this snapshot is not overwritten by normal
     * resource requests and tells us exactly what the redirect returned. */
    if(st.match_count){
        char m[260];int q=0;m[0]=0;
        append(m,&q,sizeof(m),"[TTT DLC V2.0.3] RESOURCE MATCH count=");
        hex(m,&q,sizeof(m),st.match_count);
        append(m,&q,sizeof(m)," id=");hex(m,&q,sizeof(m),st.match_id);
        append(m,&q,sizeof(m)," offset=");hex(m,&q,sizeof(m),st.match_offset);
        append(m,&q,sizeof(m)," old=");hex(m,&q,sizeof(m),st.match_original);
        append(m,&q,sizeof(m)," new=");hex(m,&q,sizeof(m),st.match_redirected);
        append(m,&q,sizeof(m)," path=");hex(m,&q,sizeof(m),st.match_path);
        append(m,&q,sizeof(m),"\n");log_line(m);
    }
}
/* Log exclusively in worker; Assembly never logs or allocates. */
static void inject_log(const char *phase){
    TTTActorInjectStats st=ttt_actor_inject_stats();
    char s[400];int p=0;int i;s[0]=0;
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] ACTOR INJECT ");append(s,&p,sizeof(s),phase);
    append(s,&p,sizeof(s)," installed=");hex(s,&p,sizeof(s),st.installed);
    append(s,&p,sizeof(s)," enabled=");hex(s,&p,sizeof(s),st.enabled);
    append(s,&p,sizeof(s)," mask=");hex(s,&p,sizeof(s),st.enabled_mask);
    append(s,&p,sizeof(s)," error=");hex(s,&p,sizeof(s),st.error);
    append(s,&p,sizeof(s)," hits=");hex(s,&p,sizeof(s),st.hits);
    append(s,&p,sizeof(s)," player=");hex(s,&p,sizeof(s),st.last_player);
    append(s,&p,sizeof(s)," original_id=");hex(s,&p,sizeof(s),st.last_original_id);
    append(s,&p,sizeof(s)," original_color=");hex(s,&p,sizeof(s),st.last_original_color);
    append(s,&p,sizeof(s)," injected_id=");hex(s,&p,sizeof(s),st.last_requested_id);
    append(s,&p,sizeof(s)," injected_color=");hex(s,&p,sizeof(s),st.last_requested_color);
    append(s,&p,sizeof(s)," patch=");hex(s,&p,sizeof(s),st.patch_insn);
    append(s,&p,sizeof(s)," delay=");hex(s,&p,sizeof(s),st.delay_insn);
    append(s,&p,sizeof(s),"\n");log_line(s);
    for(i=0;i<4;i++){
        char b[230];int q=0;b[0]=0;
        append(b,&q,sizeof(b),"[TTT DLC V2.0.3] ACTOR INJECT SLOT player=");
        hex(b,&q,sizeof(b),(uint32_t)(i+1));
        append(b,&q,sizeof(b)," enabled=");hex(b,&q,sizeof(b),(st.enabled_mask>>i)&1u);
        append(b,&q,sizeof(b)," hits=");hex(b,&q,sizeof(b),st.slot_hits[i]);
        append(b,&q,sizeof(b)," original_id=");hex(b,&q,sizeof(b),st.slot_original_id[i]);
        append(b,&q,sizeof(b)," original_color=");hex(b,&q,sizeof(b),st.slot_original_color[i]);
        append(b,&q,sizeof(b)," want_id=");hex(b,&q,sizeof(b),st.slot_requested_id[i]);
        append(b,&q,sizeof(b)," want_color=");hex(b,&q,sizeof(b),st.slot_requested_color[i]);
        append(b,&q,sizeof(b),"\n");log_line(b);
    }
}
static void plan_log(const TTTSlotPlan *plan,const char *phase,uint32_t frame){
    int i;TTTSlotChoice ch;
    TTTActorInjectStats is=ttt_actor_inject_stats();
    for(i=0;i<4;i++){
        char line[230];int p=0;line[0]=0;
        (void)ttt_slot_plan_get(plan,i,&ch);
        append(line,&p,sizeof(line),"[TTT DLC V2.0.3] PLAN ");append(line,&p,sizeof(line),phase);
        append(line,&p,sizeof(line)," player=");hex(line,&p,sizeof(line),(uint32_t)(i+1));
        append(line,&p,sizeof(line)," confirmed=");hex(line,&p,sizeof(line),ch.confirmed);
        append(line,&p,sizeof(line)," want_id=");hex(line,&p,sizeof(line),ch.external_id);
        append(line,&p,sizeof(line)," want_color=");hex(line,&p,sizeof(line),ch.color_index);
        append(line,&p,sizeof(line)," chr_known=");hex(line,&p,sizeof(line),ch.resource_supported);
        append(line,&p,sizeof(line)," applied=");
        hex(line,&p,sizeof(line),(ch.confirmed && ch.resource_supported && ((is.enabled_mask>>i)&1u) && is.slot_hits[i]>0)?1u:0u);
        append(line,&p,sizeof(line)," frame=");hex(line,&p,sizeof(line),frame);
        append(line,&p,sizeof(line),"\n");log_line(line);
    }
}
/* Four player slots observed by user in BATTLE memory. They are not the
 * pre-load selection state. This is deliberately a READ-ONLY probe. */
static const uint32_t actor_id[4]={0x08B48AE8u,0x08B48BC0u,0x08B48CB8u,0x08B48D90u};
static const uint32_t actor_color[4]={0x08B48AF0u,0x08B48BC8u,0x08B48CC0u,0x08B48D98u};
static void actor_probe_log(void){
    int i;char s[440];int p=0;s[0]=0;
    append(s,&p,sizeof(s),"[TTT DLC V2.0.3] ACTOR RAW (observacao apos hook)");
    for(i=0;i<4;i++){
        append(s,&p,sizeof(s)," P");hex(s,&p,sizeof(s),(uint32_t)(i+1));
        append(s,&p,sizeof(s),"_id=");hex(s,&p,sizeof(s),*(volatile const uint32_t*)(uintptr_t)actor_id[i]);
        append(s,&p,sizeof(s),"_color=");hex(s,&p,sizeof(s),*(volatile const uint32_t*)(uintptr_t)actor_color[i]);
    }
    append(s,&p,sizeof(s),"\n");log_line(s);
}
/* Sample the four combat actor structs during the menu-to-battle transition.
 * The worker only READS actor memory, and the ring contains no pointers.
 * Observing a write at VBlank resolution does not identify the game PC that
 * performed it. Use the PPSSPP debugger write watchpoint for that. */
static void actor_watch_sample(uint32_t frame,uint32_t plan_mask,int menu_open){
    uint32_t ids[TTT_ACTOR_COUNT],colors[TTT_ACTOR_COUNT];
    uint32_t i;
    for(i=0;i<TTT_ACTOR_COUNT;i++){
        ids[i]=*(volatile const uint32_t*)(uintptr_t)actor_id[i];
        colors[i]=*(volatile const uint32_t*)(uintptr_t)actor_color[i];
    }
    (void)ttt_actor_watch_observe(&g_actor_watch,frame,ids,colors,plan_mask,menu_open);
}
static void actor_change_log(uint32_t *cursor){
    uint32_t end=g_actor_watch.events,seq,start;
    if(!end||end<=*cursor)return;
    start=*cursor+1u;
    if(end-start+1u>32u){
        char gap[155];int gp=0;gap[0]=0;
        append(gap,&gp,sizeof(gap),"[TTT DLC V2.0.3] ACTOR GAP dropped=");
        hex(gap,&gp,sizeof(gap),(end-start+1u)-32u);
        append(gap,&gp,sizeof(gap)," (aumentar amostragem do log)\n");
        log_line(gap);
        start=end-31u;
    }
    for(seq=start;seq<=end;seq++){
        TTTActorChange e;char b[410];int p=0;b[0]=0;
        if(ttt_actor_watch_get(&g_actor_watch,seq,&e)!=0)continue;
        append(b,&p,sizeof(b),"[TTT DLC V2.0.3] ACTOR CHANGE seq=");
        hex(b,&p,sizeof(b),e.seq);
        append(b,&p,sizeof(b)," frame=");hex(b,&p,sizeof(b),e.frame);
        append(b,&p,sizeof(b)," player=");hex(b,&p,sizeof(b),e.player);
        append(b,&p,sizeof(b)," initial=");hex(b,&p,sizeof(b),(uint32_t)(e.kind==TTT_ACTOR_INIT));
        append(b,&p,sizeof(b)," old_id=");hex(b,&p,sizeof(b),e.previous_id);
        append(b,&p,sizeof(b)," id=");hex(b,&p,sizeof(b),e.id);
        append(b,&p,sizeof(b)," old_color=");hex(b,&p,sizeof(b),e.previous_color);
        append(b,&p,sizeof(b)," color=");hex(b,&p,sizeof(b),e.color);
        append(b,&p,sizeof(b)," plan=");hex(b,&p,sizeof(b),e.plan_mask);
        append(b,&p,sizeof(b)," menu=");hex(b,&p,sizeof(b),e.menu_open);
        append(b,&p,sizeof(b),"\n");log_line(b);
    }
    *cursor=end;
}
static void trace_log(uint32_t *cursor){
    uint32_t end=ttt_res_trace_count(), seq;
    uint32_t start=(*cursor)+1u;
    if(!end||start>end)return;
    /* Bound logging per sample while preserving the most recent call events.
       The hook itself never performs disk I/O. */
    if(end-start+1u>12u)start=end-11u;
    for(seq=start;seq<=end;seq++){
        TTTResourceTrace e;char b[270];int p=0;b[0]=0;
        if(ttt_res_trace_get(seq,&e)!=0)continue;
        append(b,&p,sizeof(b),"[TTT DLC V2.0.3] LOAD TRACE seq=");hex(b,&p,sizeof(b),e.seq);
        append(b,&p,sizeof(b)," site=");hex(b,&p,sizeof(b),e.site);
        append(b,&p,sizeof(b)," id=");hex(b,&p,sizeof(b),e.id);
        append(b,&p,sizeof(b)," offset=");hex(b,&p,sizeof(b),e.offset);
        append(b,&p,sizeof(b)," old=");hex(b,&p,sizeof(b),e.original);
        append(b,&p,sizeof(b)," result=");hex(b,&p,sizeof(b),e.result);
        append(b,&p,sizeof(b)," redirected=");hex(b,&p,sizeof(b),e.redirected);
        append(b,&p,sizeof(b),"\n");log_line(b);
    }
    *cursor=end;
}
static int diagnostic_thread(SceSize args,void *argp) {
    TTTToggle toggle;TTTMenuState menu;TTTSlotPlan plan;unsigned frames=0;int gpu_ready=0,armed=0;uint32_t last_trace=0,last_actor=0;
    (void)args;(void)argp;
    ttt_toggle_init(&toggle);
    ttt_menu_init(&menu);
    (void)ttt_audio_init();
    ttt_slot_plan_init(&plan);
    ttt_actor_watch_init(&g_actor_watch);
    g_mission_progress=ttt_dlc_progress_load();
    ttt_gpu_set_dlc(0,g_mission_progress);
    log_line("[TTT MENU47] Inicializado. 47 slots (3 habilitados). ELF V0.3 nativo OBRIGATORIO.\n");
    /* Wait until game starts drawing and the loader finishes linking imports. */
    sceKernelDelayThread(1500000);
    probe_log("APOS_START");
    callsites_log("APOS_START");
    bridge_log("APOS_START");resource_log("APOS_START");
    {
        int rc=ttt_input_hook_install();
        if(rc<0) {
            char s[120];int p=0;s[0]=0;
            append(s,&p,sizeof(s),"[TTT DLC V2.0.3] INPUT INSTALAR ERRO=");
            hex(s,&p,sizeof(s),(uint32_t)rc);
            append(s,&p,sizeof(s)," (controle do jogo NAO bloqueado)\n");log_line(s);
        } else log_line("[TTT DLC V2.0.3] INPUT HOOK INSTALADO.\n");
        input_log("APOS_INSTALAR");
    }
    while(g_running) {
        SceCtrlData pad;
        if(sceDisplayWaitVblankStart()<0){sceKernelDelayThread(16000);continue;}
        if(sceCtrlPeekBufferPositive(&pad,1)>0) {
        int toggled=ttt_toggle_frame(&toggle,pad.Buttons);
        if(toggled){
            ttt_audio_open(toggle.open);
            if(toggle.open)ttt_audio_play(TTT_SND_CONFIRM);
            ttt_input_hook_menu(toggle.open);
            ttt_menu_rebase(&menu,pad.Buttons);
            if(toggle.open) {
                int rc;
                log_line("[TTT DLC V2.0.3] MENU ABERTO. Preparando GE.\n");
                probe_log("ANTES_GPU");
                if(!gpu_ready) {
                    rc=ttt_gpu_init();
                    if(rc<0){log_line("[TTT DLC MENU47] GPU INIT ERRO.\n");toggle.open=0;ttt_input_hook_menu(0);ttt_audio_open(0);}
                    else gpu_ready=1;
                    probe_log("DEPOIS_GPU");gpu_guard_log("INIT");
                }
                if(gpu_ready) {
                    {
                        char b[96];int q=0;b[0]=0;
                        append(b,&q,sizeof(b),"[TTT DLC V2.0.3] PORTRAITS bits=");
                        hex(b,&q,sizeof(b),(uint32_t)ttt_gpu_portrait_status());
                        append(b,&q,sizeof(b)," (1=EARLY 2=MID 4=MID_SSJ) SKIN=");
                        hex(b,&q,sizeof(b),(uint32_t)ttt_gpu_skin_status());
                        append(b,&q,sizeof(b),"\n");
                        log_line(b);
                    }
                    /* Language overrides are loaded on first GPU init.
                     * Missing files use safe built-in defaults. */
                    {
                        unsigned lang_index;
                        for(lang_index=0;lang_index<3u;lang_index++){
                            TTTTextFileStats st=ttt_text_stats(lang_index);
                            char line[240];int q=0;line[0]=0;
                            append(line,&q,sizeof(line),"[TTT DLC V2.0.3] TEXT LANG=");
                            hex(line,&q,sizeof(line),lang_index);
                            append(line,&q,sizeof(line)," open=");hex(line,&q,sizeof(line),(uint32_t)st.open_rc);
                            append(line,&q,sizeof(line)," bytes=");hex(line,&q,sizeof(line),(uint32_t)st.bytes_read);
                            append(line,&q,sizeof(line)," overrides=");hex(line,&q,sizeof(line),(uint32_t)st.overrides);
                            append(line,&q,sizeof(line)," invalid=");hex(line,&q,sizeof(line),(uint32_t)st.invalid_lines);
                            append(line,&q,sizeof(line),"\n");log_line(line);
                        }
                    }
                    /* Distinguish missing MS files from heap/partition-memory
                     * failures. Portrait bitmask alone cannot do this. */
                    {
                        TTTSkinStats tex=ttt_gpu_skin_stats();
                        char b[310];int q=0;b[0]=0;
                        append(b,&q,sizeof(b),"[TTT DLC V2.0.3] SKIN_BG stage=");hex(b,&q,sizeof(b),(uint32_t)tex.bg.stage);
                        append(b,&q,sizeof(b)," open=");hex(b,&q,sizeof(b),(uint32_t)tex.bg.open_rc);
                        append(b,&q,sizeof(b)," size=");hex(b,&q,sizeof(b),(uint32_t)tex.bg.file_size);
                        append(b,&q,sizeof(b)," alloc=");hex(b,&q,sizeof(b),(uint32_t)tex.bg.alloc_rc);
                        append(b,&q,sizeof(b)," read=");hex(b,&q,sizeof(b),(uint32_t)tex.bg.read_bytes);
                        append(b,&q,sizeof(b)," expected=0x00020000\n");log_line(b);
                        q=0;b[0]=0;
                        append(b,&q,sizeof(b),"[TTT DLC V2.0.3] SKIN_FONT stage=");hex(b,&q,sizeof(b),(uint32_t)tex.font.stage);
                        append(b,&q,sizeof(b)," open=");hex(b,&q,sizeof(b),(uint32_t)tex.font.open_rc);
                        append(b,&q,sizeof(b)," size=");hex(b,&q,sizeof(b),(uint32_t)tex.font.file_size);
                        append(b,&q,sizeof(b)," alloc=");hex(b,&q,sizeof(b),(uint32_t)tex.font.alloc_rc);
                        append(b,&q,sizeof(b)," read=");hex(b,&q,sizeof(b),(uint32_t)tex.font.read_bytes);
                        append(b,&q,sizeof(b)," expected=0x00010000\n");log_line(b);
                    }
                    ttt_frame_hook_gpu_ready(1);
                    ttt_gpu_set_menu(ttt_menu_snapshot(&menu)|((uint32_t)ttt_bridge_is_active()<<30));
                    probe_log("ANTES_HOOK");
                    callsites_log("ANTES_HOOK");
                    rc=ttt_frame_hook_install();
                    if(rc<0) {
                        char s[80];int p=0;s[0]=0;
                        append(s,&p,sizeof(s),"[TTT DLC V2.0.3] HOOK INSTALAR ERRO=");hex(s,&p,sizeof(s),(uint32_t)rc);
                        append(s,&p,sizeof(s),"\n");log_line(s);
                        probe_log("FALHA_HOOK");
                        callsites_log("FALHA_HOOK");
                        toggle.open=0;
                        ttt_input_hook_menu(0);ttt_audio_open(0);
                    } else {log_line("[TTT DLC V2.0.3] HOOK INSTALADO.\n");callsites_log("INSTALADO");armed=1;}
                }
            } else {
                ttt_frame_hook_remove();armed=0;gpu_guard_log("CLOSE");
                log_line("[TTT DLC V2.0.3] MENU FECHADO. HOOK RESTAURADO.\n");callsites_log("RESTAURADO");
            }
            if(toggled) log_line(toggle.open?"[TTT DLC V2.0.3] MENU HOTKEY ABERTO\n":"[TTT DLC V2.0.3] MENU HOTKEY FECHADO\n");
            stats_log();
            input_log("TOGGLE");
        } else if(toggle.open) {
            uint8_t old_character=menu.character,old_color=menu.color,old_player=menu.player,old_focus=menu.focus;
            int ev=ttt_menu_buttons(&menu,pad.Buttons);
            ttt_gpu_set_menu(ttt_menu_snapshot(&menu));
            if(old_character!=menu.character||old_color!=menu.color||old_player!=menu.player||old_focus!=menu.focus)
                ttt_audio_play(TTT_SND_MOVE);
            /* Usuario mudou a escolha ja confirmada: nao manter hook armado com ID antigo. */
            if((old_character!=menu.character||old_color!=menu.color) && old_player==menu.player && !menu.confirmed[menu.player]){
                ttt_actor_inject_disable_slot(menu.player);
                ttt_slot_plan_disable(&plan,menu.player);
            }
            if(ev==TTT_EVENT_SOUND_TOGGLE){
                ttt_audio_music(menu.sound);
                ttt_audio_play(TTT_SND_MOVE);
            }else if(ev==TTT_EVENT_EMPTY){
                ttt_audio_play(TTT_SND_ERROR);
                log_line("[TTT MENU47] Slot sem ID de combate habilitado. Nenhuma injecao.\n");
            }else if(ev==TTT_EVENT_CONFIRM){
                /* V0.3 usa resolvedor NATIVO do EBOOT. Nao ativar resource_hook
                 * nem battle_bridge, para evitar conflito com ELF modificado. */
                int id=(int)ttt_menu_external_id(&menu),rc;
                if(id>2){rc=-41;}else if(!native_v03_ready()){rc=-42;}else rc=ttt_actor_inject_install();
                if(rc==0)rc=ttt_actor_inject_enable_slot((int)menu.player,(uint32_t)id,(uint32_t)menu.color);
                if(rc==0){
                    (void)ttt_slot_plan_select(&plan,menu.player,id,menu.color);
                    ttt_audio_play(TTT_SND_CONFIRM);
                    log_line("[TTT MENU47] JOGADOR CONFIRMADO: ELF V0.3 requerido.\n");
                }else{
                    menu.confirmed[menu.player]=0;
                    ttt_slot_plan_disable(&plan,menu.player);
                    ttt_actor_inject_disable_slot((int)menu.player);
                    ttt_audio_play(TTT_SND_ERROR);
                    log_line("[TTT MENU47] BLOQUEADO: falha ao instalar injector.\n");
                }
                ttt_gpu_set_menu(ttt_menu_snapshot(&menu));
            }else if(ev==TTT_EVENT_BRIDGE_RESET){
                ttt_actor_inject_remove();
                ttt_slot_plan_reset(&plan);
                for(int k=0;k<4;k++)menu.confirmed[k]=0;
                ttt_audio_play(TTT_SND_CLOSE);
                log_line("[TTT MENU47] QUADRADO: selecoes limpas.\n");
                ttt_gpu_set_menu(ttt_menu_snapshot(&menu));
            }else if(ev==TTT_EVENT_CLOSE){
                toggle.open=0;
                ttt_audio_open(0);
                ttt_input_hook_menu(0);
                ttt_frame_hook_remove();armed=0;gpu_guard_log("CLOSE");
                log_line("[TTT MENU47] MENU FECHADO.\n");
            }
        } else {
            ttt_menu_rebase(&menu,pad.Buttons);
        }
        }
        /* 60 Hz read-only chronological actor sampler; no disk I/O in sample. */
        ++frames;
        actor_watch_sample(frames,ttt_slot_plan_mask(&plan),toggle.open);
        if((frames%30u)==0u)actor_change_log(&last_actor);
        if((frames%120u)==0u){
            if(toggle.open) {
                TTTFrameHookStats hs=ttt_frame_hook_stats();
                TTTGeGuardStats gs=ttt_gpu_guard_stats();
                char b[320];int q=0;b[0]=0;
                append(b,&q,sizeof(b),"[TTT DLC V2.0.3] MENU WATCHDOG open=");
                hex(b,&q,sizeof(b),(uint32_t)toggle.open);
                append(b,&q,sizeof(b)," player=");hex(b,&q,sizeof(b),(uint32_t)menu.player);
                append(b,&q,sizeof(b)," chr=");hex(b,&q,sizeof(b),(uint32_t)menu.character);
                append(b,&q,sizeof(b)," draw_enabled=");hex(b,&q,sizeof(b),(uint32_t)hs.draw_enabled);
                append(b,&q,sizeof(b)," draws=");hex(b,&q,sizeof(b),hs.draws);
                append(b,&q,sizeof(b)," gpu_rc=");hex(b,&q,sizeof(b),(uint32_t)hs.last_draw_result);
                append(b,&q,sizeof(b)," ge_failed=");hex(b,&q,sizeof(b),(uint32_t)gs.failed);
                append(b,&q,sizeof(b)," primitives=");hex(b,&q,sizeof(b),gs.primitives);
                append(b,&q,sizeof(b)," skipped=");hex(b,&q,sizeof(b),gs.skipped_primitives);
                append(b,&q,sizeof(b)," canary=");hex(b,&q,sizeof(b),(uint32_t)gs.canary_fault);
                append(b,&q,sizeof(b),"\n");log_line(b);
                if(armed && hs.installed && !hs.draw_enabled && !gs.failed) {
                    int retry_rc=ttt_frame_hook_resume_drawing();
                    char retry[110];int rp=0;retry[0]=0;
                    append(retry,&rp,sizeof(retry),"[TTT DLC V2.0.3] MENU RECOVERY RESULT rc=");
                    hex(retry,&rp,sizeof(retry),(uint32_t)retry_rc);
                    append(retry,&rp,sizeof(retry),"\n");log_line(retry);
                }
            }
            if(armed){stats_log();gpu_guard_log("STATUS");input_log("STATUS");}
            if(ttt_res_hook_stats().installed){
                if(last_trace>ttt_res_trace_count())last_trace=0;
                resource_log("STATUS");
                inject_log("STATUS");
                trace_log(&last_trace);
                if(ttt_slot_plan_mask(&plan))actor_probe_log();
            }
        }
    }
    ttt_actor_inject_remove();
    ttt_res_hook_remove();
    ttt_bridge_remove();
    ttt_input_hook_remove();
    ttt_frame_hook_remove();
    ttt_gpu_shutdown();
    ttt_audio_shutdown();
    sceKernelExitDeleteThread(0);
    return 0;
}
int module_start(SceSize args,void *argp) {
    (void)args;(void)argp;
    log_line("\n[TTT DLC V2.0.3] module_start OK - ULUS10537 | DLC Rebuild V2.0.3: ATLAS UNICO 128KB + 3 GOKUS + 6 MISSOES\n");
    g_running=1;
    g_worker=sceKernelCreateThread("TTTExpMenuWorker",diagnostic_thread,0x20,0x4000,0,NULL);
    if(g_worker<0){g_running=0;log_line("[TTT DLC V2.0.3] ERROR: thread create\n");return 0;}
    if(sceKernelStartThread(g_worker,0,NULL)<0){g_running=0;log_line("[TTT DLC V2.0.3] ERROR: thread start\n");}
    return 0;
}
int module_stop(SceSize args,void *argp) {
    (void)args;(void)argp;
    g_running=0;
    ttt_audio_shutdown();
    ttt_actor_inject_remove();
    ttt_res_hook_remove();
    ttt_bridge_remove();
    ttt_input_hook_remove();
    ttt_frame_hook_remove();
    return 0;
}
