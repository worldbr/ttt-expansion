#ifndef PSPGU_MOCK
#define PSPGU_MOCK
#include <stdint.h>
#define GU_SPRITES 1
#define GU_VERTEX_16BIT 2
#define GU_TRANSFORM_2D 4
#define GU_TEXTURE_16BIT 8
#define GU_TEXTURE_2D 12
#define GU_PSM_8888 16
#define GU_PSM_4444 19
#define GU_TFX_MODULATE 20
#define GU_TFX_REPLACE 21
#define GU_TCC_RGBA 22
#define GU_LINEAR 23
#define GU_NEAREST 24
#define GU_CLAMP 25
#define GU_BLEND 26
#define GU_ADD 27
#define GU_SRC_ALPHA 28
#define GU_ONE_MINUS_SRC_ALPHA 29
#define GU_DIRECT 30
#define GU_SCISSOR_TEST 31
#define GU_DEPTH_TEST 32
#define GU_LIGHTING 33
#define GU_CULL_FACE 34
#define GU_ALPHA_TEST 35
#define GU_STENCIL_TEST 36
#define GU_COLOR_TEST 37
#define GU_COLOR_LOGIC_OP 38
#define GU_FOG 39
#define GU_FLAT 40
#define GU_SYNC_FINISH 41
#define GU_SYNC_WHAT_DONE 42
void *sceGuGetMemory(int size);
int sceGuInit(void);
int sceGuStart(int mode,void* list);
void sceGuEnable(int);
void sceGuDisable(int);
void sceGuColor(uint32_t);
void sceGuDrawArray(int,int,int,void*,void*);
void sceGuTexMode(int,int,int,int);
void sceGuTexFunc(int,int);
void sceGuTexFilter(int,int);
void sceGuTexWrap(int,int);
void sceGuTexImage(int,int,int,int,void*);
void sceGuBlendFunc(int,int,int,int,int);
void sceGuDrawBufferList(int,void*,int);
void sceGuOffset(int,int);
void sceGuViewport(int,int,int,int);
void sceGuScissor(int,int,int,int);
void sceGuDepthMask(int);
void sceGuShadeModel(int);
int sceGuFinish(void);
int sceGuSync(int,int);
#endif
