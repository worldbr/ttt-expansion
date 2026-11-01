#ifndef TTT_MENU_AUDIO_H
#define TTT_MENU_AUDIO_H
/* Sons sem musica, com canal reservado somente durante o efeito. */
enum {TTT_SND_MOVE=1,TTT_SND_CONFIRM=2,TTT_SND_CLOSE=3,TTT_SND_ERROR=4};
int ttt_audio_init(void);
void ttt_audio_open(int opened);
void ttt_audio_play(int kind);
int ttt_audio_status(void);
void ttt_audio_shutdown(void);
#endif
