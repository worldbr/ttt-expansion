/* Sintese PCM original em 44100 Hz; nao usa musica/efeitos protegidos. */
#include "menu_audio.h"
#include <pspuser.h>
#include <pspaudio.h>
#include <stdint.h>
#define SAMPLES 512
static int16_t sample_buffer[SAMPLES*2] __attribute__((aligned(64)));
static volatile int audio_run=0,audio_open=0,audio_music=1,audio_event=0;
static SceUID audio_thread=-1;
static unsigned phase_music,phase_fx,frames_music;
static int remaining_fx,fx_kind;
/* Triangulo em -1024..1024. Sem ponto flutuante ou biblioteca math. */
static int tri(unsigned phase){unsigned p=(phase*2048u)/44100u;return (p<1024u)?(int)p:(int)(2048u-p);}
static int audio_task(SceSize args,void *argp){
    int channel=-1; (void)args;(void)argp;
    while(audio_run){
        int i;
        if(!audio_open){
            if(channel>=0){sceAudioChRelease(channel);channel=-1;}
            sceKernelDelayThread(20000);continue;
        }
        if(channel<0){
            channel=sceAudioChReserve(-1,SAMPLES,PSP_AUDIO_FORMAT_STEREO);
            if(channel<0){sceKernelDelayThread(100000);continue;}
        }
        if(audio_event){fx_kind=audio_event;audio_event=0;remaining_fx=(fx_kind==2)?5500:3600;phase_fx=0;}
        for(i=0;i<SAMPLES;i++){
            int value=0, fx=0;
            if(audio_music){
                static const unsigned notes[4]={220,262,330,247};
                unsigned note=notes[(frames_music/22050u)%4u];
                phase_music+=note;if(phase_music>=44100u)phase_music-=44100u;
                value=(tri(phase_music)-512)*2; /* fundo baixo */
            }
            if(remaining_fx>0){
                unsigned f=(fx_kind==1)?660u:(fx_kind==2)?880u:(fx_kind==3)?370u:170u;
                phase_fx+=f;if(phase_fx>=44100u)phase_fx-=44100u;
                fx=((tri(phase_fx)-512)*remaining_fx)/1800;
                remaining_fx--;
            }
            value+=fx;
            if(value>14000)value=14000;if(value< -14000)value=-14000;
            sample_buffer[i*2]=sample_buffer[i*2+1]=(int16_t)value;
            frames_music++;
        }
        /* Evita ocupar o thread que processa controle e desenho. */
        if(sceAudioOutputBlocking(channel,0x2800,sample_buffer)<0)sceKernelDelayThread(12000);
    }
    if(channel>=0)sceAudioChRelease(channel);
    sceKernelExitDeleteThread(0);return 0;
}
int ttt_audio_init(void){
    if(audio_thread>=0)return 0;
    audio_run=1;audio_open=0;audio_music=1;
    audio_thread=sceKernelCreateThread("TTTMenu47Audio",audio_task,0x24,0x1000,0,NULL);
    if(audio_thread<0){audio_run=0;return -1;}
    if(sceKernelStartThread(audio_thread,0,NULL)<0){audio_run=0;audio_thread=-1;return -2;}
    return 0;
}
void ttt_audio_open(int opened){audio_open=opened?1:0;if(!opened)audio_event=0;}
void ttt_audio_music(int enabled){audio_music=enabled?1:0;}
void ttt_audio_play(int kind){if(audio_open && kind>=1&&kind<=4)audio_event=kind;}
void ttt_audio_shutdown(void){audio_open=0;audio_run=0;audio_thread=-1;}
