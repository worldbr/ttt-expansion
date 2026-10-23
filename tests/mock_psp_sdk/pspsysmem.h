#ifndef MOCK_PSPSYSMEM_H
#define MOCK_PSPSYSMEM_H
#include <stddef.h>
typedef int SceUID;
typedef unsigned int SceSize;
#define PSP_SMEM_Low 0
SceUID sceKernelAllocPartitionMemory(SceUID partitionid,const char *name,int type,SceSize size,void *addr);
void *sceKernelGetBlockHeadAddr(SceUID blockid);
int sceKernelFreePartitionMemory(SceUID blockid);
#endif
