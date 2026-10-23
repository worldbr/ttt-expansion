#ifndef TTT_MENU_AUDIO_H
#define TTT_MENU_AUDIO_H
/* Thread exclusiva de audio, sem chamadas no hook de GPU. */
enum {TTT_SND_MOVE=1,TTT_SND_CONFIRM=2,TTT_SND_CLOSE=3,TTT_SND_ERROR=4};
int ttt_audio_init(void);
void ttt_audio_open(int opened);
void ttt_audio_music(int enabled);
void ttt_audio_play(int kind);
void ttt_audio_shutdown(void);
#endif
