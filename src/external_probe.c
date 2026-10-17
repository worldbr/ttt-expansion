/* V1.0A: establish that an actual external CHR PCK1 can be read from ms0.
 * This does NOT replace the game's PACKFILE loader (a separate later task).
 * Run only in plugin worker thread when user explicitly confirms ID00.
 */
#include "external_probe.h"
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspiofilemgr.h>
#endif
uint32_t ttt_probe_hash_update(uint32_t v,const unsigned char *data,uint32_t n) {
    uint32_t i;
    if(!data && n)return v;
    for(i=0;i<n;i++){v^=(uint32_t)data[i];v*=UINT32_C(16777619);}
    return v;
}
int ttt_probe_validate_size(uint32_t expected,uint32_t actual) {
    if(!expected || expected>UINT32_C(0x02000000))return -10;
    if(!actual || actual>UINT32_C(0x02000000))return -11;
    if(expected!=actual)return -12;
    return 0;
}
int ttt_external_probe_file(const char *path,uint32_t expected,TTTExternalProbe *out){
    if(!out||!path)return -20;
    out->result=-20;out->expected_size=expected;
    out->actual_size=0;out->bytes_read=0;out->signature=0;out->fnv1a=0;
#ifdef TTT_BRIDGE_PSP
    {
        SceUID fd=sceIoOpen(path,PSP_O_RDONLY,0);
        unsigned char buf[4096];
        uint32_t hash=UINT32_C(2166136261);
        SceOff size;
        int rc;
        if(fd<0){out->result=-1;return out->result;}
        size=sceIoLseek(fd,0,PSP_SEEK_END);
        if(size<=0||size>UINT32_C(0x02000000)){
            sceIoClose(fd);out->result=-2;return out->result;
        }
        out->actual_size=(uint32_t)size;
        rc=ttt_probe_validate_size(expected,(uint32_t)size);
        if(rc<0){sceIoClose(fd);out->result=rc;return rc;}
        if(sceIoLseek(fd,0,PSP_SEEK_SET)!=0){sceIoClose(fd);out->result=-3;return -3;}
        while(out->bytes_read<out->actual_size){
            unsigned remaining=out->actual_size-out->bytes_read;
            int want=(int)(remaining>sizeof(buf)?sizeof(buf):remaining);
            int got=sceIoRead(fd,buf,want);
            if(got!=want){sceIoClose(fd);out->result=-4;return -4;}
            if(out->bytes_read==0){
                out->signature=(uint32_t)buf[0]|((uint32_t)buf[1]<<8)|
                               ((uint32_t)buf[2]<<16)|((uint32_t)buf[3]<<24);
            }
            hash=ttt_probe_hash_update(hash,buf,(uint32_t)got);
            out->bytes_read+=(uint32_t)got;
        }
        sceIoClose(fd);
        out->fnv1a=hash;out->result=0;return 0;
    }
#else
    out->result=-90;return -90;
#endif
}
