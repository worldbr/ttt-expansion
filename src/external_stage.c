#include "external_stage.h"
#include "external_probe.h"
#include <stdlib.h>
#include <stdint.h>
#ifdef TTT_BRIDGE_PSP
#include <pspiofilemgr.h>
#include <pspsysmem.h>
#include <pspmodulemgr.h>
#else
#include <stdio.h>
#endif

static unsigned char *g_staged;
static uint32_t g_staged_size;
#ifdef TTT_BRIDGE_PSP
static SceUID g_stage_block=-1;
#endif

void ttt_external_stage_release(void) {
#ifdef TTT_BRIDGE_PSP
    if(g_stage_block>=0)sceKernelFreePartitionMemory(g_stage_block);
    g_stage_block=-1;
#else
    if(g_staged)free(g_staged);
#endif
    g_staged=0;g_staged_size=0;
}
const unsigned char *ttt_external_stage_data(void) {return g_staged;}
uint32_t ttt_external_stage_size(void) {return g_staged_size;}

#ifdef TTT_BRIDGE_PSP
typedef SceUID TTTHandle;
static TTTHandle open_file(const char *p){return sceIoOpen(p,PSP_O_RDONLY,0);}
static int open_failed(TTTHandle h){return h<0;}
static long long file_size(TTTHandle h){return (long long)sceIoLseek(h,0,PSP_SEEK_END);}
static int file_rewind(TTTHandle h){return sceIoLseek(h,0,PSP_SEEK_SET)==0;}
static int file_read(TTTHandle h,void *buf,int n){return sceIoRead(h,buf,n);}
static void file_close(TTTHandle h){sceIoClose(h);}
#else
/* Portable host test path only. PSP builds use sceIo*, not stdio. */
typedef FILE *TTTHandle;
static TTTHandle open_file(const char *p){return fopen(p,"rb");}
static int open_failed(TTTHandle h){return h==NULL;}
static long long file_size(TTTHandle h){long x;if(fseek(h,0,SEEK_END)!=0)return -1;x=ftell(h);return (long long)x;}
static int file_rewind(TTTHandle h){return fseek(h,0,SEEK_SET)==0;}
static int file_read(TTTHandle h,void *buf,int n){return (int)fread(buf,1,(size_t)n,h);}
static void file_close(TTTHandle h){fclose(h);}
#endif

int ttt_external_stage_load(const char *path, uint32_t reference, TTTExternalStage *out) {
    TTTHandle fd;
    unsigned char *ptr;
#ifdef TTT_BRIDGE_PSP
    SceUID block=-1;
#endif
    long long length;
    uint32_t total=0,hash=UINT32_C(2166136261);
    int rc=0;
    if(!out)return -20;
    out->result=-20;out->reference_size=reference;
    out->actual_size=0;out->bytes_read=0;out->first4=0;
    out->fnv1a=0;out->address=0;out->retained=0;
    out->allocator=0;out->alloc_error=0;out->block_uid=-1;
    /* A failed or repeated load cannot accidentally reuse stale bytes. */
    ttt_external_stage_release();
    if(!path){out->result=-20;return -20;}
    fd=open_file(path);
    if(open_failed(fd)){out->result=-1;return -1;}
    length=file_size(fd);
    if(length<0){rc=-2;goto finish_fd;}
    if(length<4096 || length>(long long)TTT_STAGE_MAX_SIZE){rc=-11;goto finish_fd;}
    out->actual_size=(uint32_t)length;
    if(!file_rewind(fd)){rc=-3;goto finish_fd;}
    /* V1.0C used malloc(), whose PRX libc heap may be much smaller
     * than the free user partition. Allocate directly from user memory.
     * Do NOT allocate kernel memory and do NOT patch game pointers. */
#ifdef TTT_BRIDGE_PSP
    out->allocator=1;
    block=sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER,
                                      "TTT_ExternalCHR",PSP_SMEM_High,
                                      (SceSize)out->actual_size,NULL);
    if(block<0){out->alloc_error=(int32_t)block;rc=-30;goto finish_fd;}
    ptr=(unsigned char *)sceKernelGetBlockHeadAddr(block);
    if(!ptr){out->alloc_error=-31;rc=-31;goto finish_ptr;}
#else
    out->allocator=2;
    ptr=(unsigned char *)malloc((size_t)out->actual_size);
    if(!ptr){out->alloc_error=-30;rc=-30;goto finish_fd;}
#endif
    while(total<out->actual_size){
        uint32_t remain=out->actual_size-total;
        int want=(int)(remain>4096u?4096u:remain);
        int got=file_read(fd,ptr+total,want);
        if(got<=0||got>want){rc=-4;goto finish_ptr;}
        hash=ttt_probe_hash_update(hash,ptr+total,(uint32_t)got);
        total+=(uint32_t)got;
        out->bytes_read=total;
    }
    out->first4=(uint32_t)ptr[0]|((uint32_t)ptr[1]<<8)|
        ((uint32_t)ptr[2]<<16)|((uint32_t)ptr[3]<<24);
    out->fnv1a=hash;
    g_staged=ptr;g_staged_size=out->actual_size;
#ifdef TTT_BRIDGE_PSP
    g_stage_block=block;
    out->block_uid=(int32_t)block;
#endif
    out->address=(uint32_t)(uintptr_t)g_staged;
    out->retained=1;
    rc=0;
    goto finish_fd;
finish_ptr:
#ifdef TTT_BRIDGE_PSP
    if(block>=0)sceKernelFreePartitionMemory(block);
#else
    free(ptr);
#endif
finish_fd:
    file_close(fd);
    out->result=rc;
    return rc;
}
