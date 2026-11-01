/* Menu47 V0.1.2 - efeitos PCM curtos, SEM MUSICA.
 * Audio so reserva um canal durante cada efeito; nunca transmite silencio.
 * PSP: 44100Hz estereo, blocos de 512 samples.
 */
#include "menu_audio.h"
#include <pspuser.h>
#include <pspaudio.h>
#include <stdint.h>
#define SAMPLES 512
static int16_t sample_buffer[SAMPLES*2] __attribute__((aligned(64)));
static volatile int audio_run=0,audio_open=0,audio_event=0,audio_last_error=0;
static SceUID audio_thread=-1;
static int triangle(unsigned phase){unsigned p=(phase*2048u)/44100u;return (p<1024u)?(int)p:(int)(2048u-p);}
static int audio_task(SceSize args,void *argp){
    (void)args;(void)argp;
    while(audio_run){
        int kind=audio_event,channel,frames,done=0;
        unsigned phase=0,freq;
        if(!kind){sceKernelDelayThread(5000);continue;}
        audio_event=0;
        /* Never take an existing game's reserved audio channel. */
        channel=sceAudioChReserve(-1,SAMPLES,PSP_AUDIO_FORMAT_STEREO);
        if(channel<0){audio_last_error=channel;sceKernelDelayThread(9000);continue;}
        audio_last_error=0;
        frames=(kind==TTT_SND_CONFIRM)?7600:((kind==TTT_SND_ERROR)?6500:((kind==TTT_SND_CLOSE)?4700:4000));
        freq=(kind==TTT_SND_MOVE)?830u:((kind==TTT_SND_CONFIRM)?1030u:((kind==TTT_SND_ERROR)?250u:510u));
        while(done<frames && audio_run){
            int i;
            for(i=0;i<SAMPLES;i++){
                int remaining=frames-done-i;
                int gain=(remaining>0)?remaining:0;
                int sample;
                if(gain>2000)gain=2000;
                if(kind==TTT_SND_CONFIRM && (done+i)>2500)freq=1320u;
                phase+=freq;if(phase>=44100u)phase-=44100u;
                sample=((triangle(phase)-512)*24*gain)/2000;
                sample_buffer[2*i]=sample_buffer[2*i+1]=(int16_t)sample;
            }
            if(sceAudioOutputBlocking(channel,0x7000,sample_buffer)<0){audio_last_error=-90;break;}
            done+=SAMPLES;
        }
        sceAudioChRelease(channel);
    }
    sceKernelExitDeleteThread(0);
    return 0;
}
int ttt_audio_init(void){
    if(audio_thread>=0)return 0;
    audio_run=1;audio_open=0;audio_event=0;
    audio_thread=sceKernelCreateThread("TTTMenu47SFX",audio_task,0x24,0x1800,0,NULL);
    if(audio_thread<0){audio_run=0;return -1;}
    if(sceKernelStartThread(audio_thread,0,NULL)<0){audio_run=0;audio_thread=-1;return -2;}
    return 0;
}
void ttt_audio_open(int opened){audio_open=opened?1:0; /* efeito de fechar termina mesmo apos fechar */ }
void ttt_audio_play(int kind){if((audio_open||kind==TTT_SND_CLOSE)&&kind>=1&&kind<=4)audio_event=kind;}
int ttt_audio_status(void){return audio_last_error;}
void ttt_audio_shutdown(void){audio_open=0;audio_run=0;audio_event=0;audio_thread=-1;}
