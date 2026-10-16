/*
 * TTT Expansion diagnostic PRX - user-mode PPSSPP plugin.
 * Does not patch the EBOOT, ISO, character IDs, or packfile.
 * Framebuffer overlay is best-effort, and log is diagnostic fallback.
 */
#include <pspuser.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <stdint.h>
#include "overlay.h"
#include "toggle.h"

PSP_MODULE_INFO("TTTExpDiag", PSP_MODULE_USER, 1, 3);
PSP_NO_CREATE_MAIN_THREAD();

#define LOG_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/diagnostico.txt"
static volatile int g_running = 0;
static SceUID g_worker = -1;

static void log_line(const char *s) {
    int n = 0;
    SceUID fd;
    while (s[n]) ++n;
    fd = sceIoOpen(LOG_PATH, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
    if (fd < 0) return;
    sceIoWrite(fd, s, n);
    sceIoClose(fd);
}

static int valid_vram(void *address, int stride, int pixel_format) {
    uintptr_t ptr = (uintptr_t)address;
    uint32_t addr = (uint32_t)ptr;
    uint32_t offset = addr & UINT32_C(0x001fffff);
    uint32_t bytes_per_pixel;
    if (!address || pixel_format < 0 || pixel_format > 3 || stride < 480 || stride > 2048) return 0;
    /* Accept VRAM and common cached/uncached aliases only. Reject regular RAM. */
    if ((addr & UINT32_C(0x1f000000)) != UINT32_C(0x04000000)) return 0;
    bytes_per_pixel = pixel_format == 3 ? 4u : 2u;
    return offset + (uint32_t)stride * 272u * bytes_per_pixel <= UINT32_C(0x00200000);
}

static int diagnostic_thread(SceSize args, void *argp) {
    TTTToggle toggle;
    (void)args; (void)argp;
    ttt_toggle_init(&toggle);
    log_line("[TTT V0.3] Thread iniciado. L+R+Quadrado abre/fecha.\n");
    while (g_running) {
        SceCtrlData pad;
        void *fb = 0;
        int stride = 0, fmt = -1;
        int result = sceDisplayWaitVblankStart();
        if (result < 0) { sceKernelDelayThread(16000); continue; }
        if (sceCtrlPeekBufferPositive(&pad, 1) > 0 && ttt_toggle_frame(&toggle, pad.Buttons)) {
            log_line(toggle.open ? "[TTT V0.3] MENU ABERTO\n" : "[TTT V0.3] MENU FECHADO\n");
        }
        if (sceDisplayGetFrameBuf(&fb, &stride, &fmt, PSP_DISPLAY_SETBUF_NEXTHSYNC) >= 0 &&
            valid_vram(fb, stride, fmt)) {
            ttt_draw_overlay(fb, stride, fmt, toggle.open);
        }
    }
    log_line("[TTT V0.3] Thread encerrado.\n");
    sceKernelExitDeleteThread(0);
    return 0;
}

int module_start(SceSize args, void *argp) {
    (void)args; (void)argp;
    log_line("[TTT V0.3] module_start OK - ULUS10537\n");
    g_running = 1;
    g_worker = sceKernelCreateThread("TTTExpDiagWorker", diagnostic_thread, 0x20, 0x2000, 0, NULL);
    if (g_worker < 0) { g_running = 0; log_line("[TTT V0.3] ERRO criando thread\n"); return 0; }
    if (sceKernelStartThread(g_worker, 0, NULL) < 0) {
        g_running = 0;
        log_line("[TTT V0.3] ERRO iniciando thread\n");
    }
    return 0;
}

int module_stop(SceSize args, void *argp) {
    (void)args; (void)argp;
    g_running = 0;
    return 0;
}
