#include "dlc_progress.h"
#include "dlc_story.h"
#ifdef TTT_BRIDGE_PSP
#include <pspiofilemgr.h>
#endif
#define TTT_PROGRESS_PATH "ms0:/PSP/PLUGINS/TTT_EXPANSION/dlc_progress.bin"
#define TTT_PROGRESS_MAGIC UINT32_C(0x32434C44) /* DLC2 */
typedef struct ProgressDisk {uint32_t magic, flags, check;} ProgressDisk;
uint32_t ttt_dlc_progress_load(void){
#ifdef TTT_BRIDGE_PSP
    ProgressDisk disk;int count;
    SceUID fd=sceIoOpen(TTT_PROGRESS_PATH,PSP_O_RDONLY,0);
    if(fd<0)return 0u;
    count=sceIoRead(fd,&disk,(int)sizeof(disk));sceIoClose(fd);
    if(count!=(int)sizeof(disk)||disk.magic!=TTT_PROGRESS_MAGIC||disk.check!=(disk.magic^disk.flags^UINT32_C(0x5A13C0DE)))return 0u;
    return ttt_dlc_progress_sanitize(disk.flags);
#else
    return 0u;
#endif
}
int ttt_dlc_progress_save(uint32_t flags){
#ifdef TTT_BRIDGE_PSP
    ProgressDisk disk;int count;SceUID fd;
    disk.magic=TTT_PROGRESS_MAGIC;disk.flags=ttt_dlc_progress_sanitize(flags);
    disk.check=disk.magic^disk.flags^UINT32_C(0x5A13C0DE);
    fd=sceIoOpen(TTT_PROGRESS_PATH,PSP_O_WRONLY|PSP_O_CREAT|PSP_O_TRUNC,0777);
    if(fd<0)return -1;
    count=sceIoWrite(fd,&disk,(int)sizeof(disk));sceIoClose(fd);
    return count==(int)sizeof(disk)?0:-2;
#else
    (void)flags;return -90;
#endif
}
