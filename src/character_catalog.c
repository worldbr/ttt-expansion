#include "character_catalog.h"
#include <pspiofilemgr.h>
#include <stdint.h>
#define TTT_NAMES "ms0:/PSP/PLUGINS/TTT_EXPANSION/characters.txt"
#define NSLOTS 47
#define MAX_NAME 26
static char g_names[NSLOTS][MAX_NAME];
static char g_file[4096];
static int g_loaded=0;
static void copy_clean(char *dst,const char *src,int n){
    int i=0,last=-1;
    while(n>0 && (*src==' '||*src=='\t')){src++;n--;}
    while(n-- > 0 && *src && i<MAX_NAME-1){
        unsigned char c=(unsigned char)*src++;
        /* O atlas PSP 5x7 tem suporte ASCII, nao acentos UTF-8. */
        if(c>='a' && c<='z')c=(unsigned char)(c-'a'+'A');
        if(c>=32u && c<=126u){dst[i++]=(char)c; if(c!=' ')last=i;}
    }
    dst[last<0?0:last]=0;
}
void ttt_catalog_reload(void){
    int i,n;SceUID fd;
    static const char *first[]={"GOKU (INICIO)","GOKU (MEIO)","GOKU (MEIO) SSJ"};
    for(i=0;i<NSLOTS;i++){
        int j=0;
        const char *base=(i<3)?first[i]:"PERSONAGEM";
        while(base[j] && j<MAX_NAME-1){g_names[i][j]=base[j];j++;}
        if(i>=3 && j<MAX_NAME-4){g_names[i][j++]=' ';g_names[i][j++]=(char)('0'+(i+1)/10);g_names[i][j++]=(char)('0'+(i+1)%10);}
        g_names[i][j]=0;
    }
    g_loaded=1;
    fd=sceIoOpen(TTT_NAMES,PSP_O_RDONLY,0);
    if(fd<0)return; // nomes padrao sao fallback seguro
    n=sceIoRead(fd,g_file,sizeof(g_file)-1);
    sceIoClose(fd);
    if(n<=0)return;
    g_file[n]=0;
    {
        int cursor=0;
        while(cursor<n){
            int start=cursor,end,k,slot;
            while(cursor<n && g_file[cursor]!='\n' && g_file[cursor]!='\r')cursor++;
            end=cursor;
            while(cursor<n && (g_file[cursor]=='\n' || g_file[cursor]=='\r'))cursor++;
            if(end-start<4 || g_file[start]=='#')continue;
            k=start;
            if(g_file[k]<'0'||g_file[k]>'9'||g_file[k+1]<'0'||g_file[k+1]>'9')continue;
            slot=(g_file[k]-'0')*10+g_file[k+1]-'0';
            k+=2;
            while(k<end && (g_file[k]==' '||g_file[k]=='\t'))k++;
            if(k>=end || g_file[k]!='=' || slot<1 || slot>NSLOTS)continue;
            k++;
            if(k<end)copy_clean(g_names[slot-1],g_file+k,end-k);
        }
    }
}
const char *ttt_catalog_name(unsigned slot){
    if(!g_loaded)ttt_catalog_reload();
    return slot<NSLOTS?g_names[slot]:"PERSONAGEM";
}
