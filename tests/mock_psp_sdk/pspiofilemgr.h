typedef int SceUID;
#define PSP_O_RDONLY 0
#define PSP_SEEK_END 2
#define PSP_SEEK_SET 0
SceUID sceIoOpen(const char*,int,int);
int sceIoLseek(SceUID,int,int);
int sceIoRead(SceUID,void*,int);
int sceIoClose(SceUID);
