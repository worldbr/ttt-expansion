#include "dlc_story.h"
/* Desafios para treinamento: nao sao batalhas automaticas ou roteiros
 * modificados do modo Historia original. Os mapas originais sao mantidos. */
static const TTTDlcMission missions[TTT_DLC_MISSIONS]={
    {"Treino de Kame",    "Saga Saiyajin", "Pratique movimentos com Goku (Inicio).",0u,0u},
    {"Forca Saiyajin",    "Saga Saiyajin", "Treine a defesa e os ataques de ki.",0u,1u},
    {"Rumo a Namek",     "Saga Namek",    "Prepare Goku (Meio) para o confronto.",1u,0u},
    {"Limite de Goku",   "Saga Namek",    "Venca um desafio de treino com Color 2.",1u,1u},
    {"O Despertar",      "Saga Freeza",   "Treine com Goku (Meio) SSJ.",2u,0u},
    {"Lenda Dourada",    "Saga Freeza",   "Use o traje Color 4 do Super Saiyajin.",2u,3u}
};
const TTTDlcMission *ttt_dlc_mission(unsigned index){return index<TTT_DLC_MISSIONS?&missions[index]:0;}
uint32_t ttt_dlc_progress_sanitize(uint32_t flags){return flags&((1u<<TTT_DLC_MISSIONS)-1u);}
uint32_t ttt_dlc_progress_mark(uint32_t flags,unsigned index){
    return index<TTT_DLC_MISSIONS?ttt_dlc_progress_sanitize(flags|(1u<<index)):ttt_dlc_progress_sanitize(flags);
}
int ttt_dlc_mission_choices(unsigned index,unsigned *id,unsigned *color){
    const TTTDlcMission *m=ttt_dlc_mission(index);
    if(!m||!id||!color)return -1;
    *id=m->character;*color=m->color;
    return 0;
}
