/* Host simulation of PSP file IO + partition memory, using the REAL loader
 * in gpu_probe.c; link with --gc-sections to omit drawing code not tested. */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pspge.h>
#include <pspgu.h>
#include <pspiofilemgr.h>
#include <pspsysmem.h>
#include "gpu_probe.h"
#include "ui_text.h"
static FILE *files[16];
static void *blocks[16];
static const char *assetroot;
static int fail_alloc;
static size_t allowed_alloc_bytes=131072u; /* simulate PSP resource pressure */
static unsigned allocations;
SceUID sceIoOpen(const char *path,int flags,int mode){
    int i; char local[1024];const char *prefix="ms0:/PSP/PLUGINS/TTT_EXPANSION/";
    (void)flags;(void)mode;
    if(strncmp(path,prefix,strlen(prefix))!=0)return -1;
    snprintf(local,sizeof(local),"%s/%s",assetroot,path+strlen(prefix));
    for(i=1;i<16;i++)if(!files[i])break;
    if(i==16)return -1;
    files[i]=fopen(local,"rb");return files[i]?i:-2;
}
int sceIoLseek(SceUID fd,int offset,int whence){
    long p;
    if(fd<1||fd>=16||!files[fd]||fseek(files[fd],offset,whence))return -1;
    p=ftell(files[fd]);return (int)p;
}
int sceIoRead(SceUID fd,void *dst,int len){
    if(fd<1||fd>=16||!files[fd])return -1;
    return (int)fread(dst,1,(size_t)len,files[fd]);
}
int sceIoClose(SceUID fd){
    if(fd<1||fd>=16||!files[fd])return -1;
    fclose(files[fd]);files[fd]=NULL;return 0;
}
SceUID sceKernelAllocPartitionMemory(SceUID partitionid,const char *name,int type,SceSize size,void *addr){
    int i;(void)name;(void)type;(void)addr;
    assert(partitionid==2);
    if(fail_alloc || size>allowed_alloc_bytes)return -8;
    ++allocations;
    for(i=1;i<16;i++)if(!blocks[i])break;
    if(i==16)return -9;
    blocks[i]=malloc(size);return blocks[i]?i:-10;
}
void *sceKernelGetBlockHeadAddr(SceUID id){return (id>0&&id<16)?blocks[id]:NULL;}
int sceKernelFreePartitionMemory(SceUID id){
    if(id<=0||id>=16||!blocks[id])return -1;
    free(blocks[id]);blocks[id]=NULL;return 0;
}
int sceGeSaveContext(PspGeContext *ctx){(void)ctx;return 0;}
int sceGeRestoreContext(PspGeContext *ctx){(void)ctx;return 0;}
int sceGuInit(void){return 0;}
void sceKernelDcacheWritebackRange(const void *ptr,unsigned int size){(void)ptr;(void)size;}
int main(int argc,char **argv){
    TTTSkinStats stats;
    assert(argc==4);
    assetroot=argv[1];fail_alloc=atoi(argv[2]);
    assert(ttt_gpu_init()==0);
    stats=ttt_gpu_skin_stats();
    if(!strcmp(assetroot,"TTT_EXPANSION")){
        assert(ttt_text_stats(0).overrides==TTT_TEXT_COUNT);
        assert(ttt_text_stats(1).overrides==TTT_TEXT_COUNT);
        assert(ttt_text_stats(2).overrides==TTT_TEXT_COUNT);
        assert(!strcmp(ttt_text(0,TTT_TEXT_TAB_CHARACTERS),"PERSONAGENS"));
    }else{
        assert(ttt_text_stats(0).overrides==0);
    }
    assert(stats.mask==(unsigned)atoi(argv[3]));
    if(stats.mask==3){
        assert(stats.bg.stage==0&&stats.font.stage==0);
        assert(stats.bg.read_bytes==131072&&stats.font.read_bytes==49152);
        assert(stats.font.alloc_rc==0); /* same user block */
        assert(allocations==1);
        assert(stats.bg.file_size==131072&&stats.font.file_size==65536);
    }else if(fail_alloc){assert(stats.bg.stage==3&&stats.font.stage==6);}
    else{assert(stats.bg.stage==1&&stats.font.stage==6);}
    ttt_gpu_shutdown();
    for(int i=1;i<16;i++) assert(!blocks[i]&&!files[i]);
    puts("SKIN LOADER MOCK PASS");
    return 0;
}
