/* Runtime-editable labels for TTT Expansion. ASCII font, UTF-8 Latin accents
 * transliterated during load. No heap, no GPU calls, no ISO editing. */
#include "ui_text.h"
#include <stddef.h>
#if defined(TTT_BRIDGE_PSP)
#include <pspiofilemgr.h>
#endif
static char labels[TTT_UI_TEXT_LANGS][TTT_TEXT_COUNT][TTT_UI_TEXT_MAX];
static TTTTextFileStats stats[TTT_UI_TEXT_LANGS];
static const char *const keynames[TTT_TEXT_COUNT]={
    "TITLE",
    "TAB_CHARACTERS",
    "TAB_MISSIONS",
    "TAB_MAPS",
    "TAB_OPTIONS",
    "COMING_SOON",
    "EXTRA_CHARACTERS",
    "PREVIEW",
    "FUTURE",
    "STATUS_PREVIEW",
    "STATUS_CONFIRMED",
    "STATUS_ACTIVE",
    "CHAR0_NAME",
    "CHAR1_NAME",
    "CHAR2_NAME",
    "CHAR2_SUBTITLE",
    "CHAR0_CARD_TOP",
    "CHAR0_CARD_BOTTOM",
    "CHAR1_CARD_TOP",
    "CHAR1_CARD_BOTTOM",
    "CHAR2_CARD_TOP",
    "CHAR2_CARD_BOTTOM",
    "ID_LABEL",
    "PLAYER_LABEL",
    "COLOR_LABEL",
    "FOCUS_CHARACTERS",
    "FOCUS_COLORS",
    "MISSIONS_SOON_TITLE",
    "MISSIONS_SOON_DETAIL",
    "MAPS_SOON_TITLE",
    "MAPS_SOON_DETAIL",
    "OPTIONS_SOON_TITLE",
    "OPTIONS_SOON_DETAIL",
    "FOOTER_MOVE",
    "FOOTER_CONFIRM",
    "FOOTER_CLOSE",
    "TEXTURE_ERROR",
    "MISSION_RESERVED",
};
static const char *const defaults[TTT_UI_TEXT_LANGS][TTT_TEXT_COUNT]={
    {
        "TTT EXPANSION",
        "PERSONAGENS",
        "MISSOES",
        "MAPAS",
        "OPCOES",
        "EM BREVE",
        "PERSONAGENS EXTRAS",
        "PREVIA",
        "FUTURO",
        "PREVIA",
        "CONFIRMADO",
        "ATIVO",
        "Goku (Inicio)",
        "Goku (Meio)",
        "Goku (Meio)",
        "Super Saiyajin",
        "Goku",
        "(Inicio)",
        "Goku",
        "(Meio)",
        "Goku (Meio)",
        "Super Saiyajin",
        "ID",
        "JOGADOR",
        "TRAJE",
        "PERSONAGENS",
        "TRAJES",
        "MISSOES - EM BREVE",
        "Novos desafios serao adicionados.",
        "MAPAS - EM BREVE",
        "Cenarios adicionais estao em desenvolvimento.",
        "OPCOES - EM BREVE",
        "Mais configuracoes serao adicionadas.",
        "ESQ/DIR: MOVER   CIMA/BAIXO: FOCO   L/R: JOGADOR",
        "X: CONFIRMAR   TRIANGULO: ABA   SELECT: IDIOMA",
        "QUADRADO: LIMPAR   CIRCULO: FECHAR",
        "TEXTURAS NAO CARREGADAS - VER DIAGNOSTICO",
        "DESAFIOS TEMPORARIAMENTE DESATIVADOS",
    },
    {
        "TTT EXPANSION",
        "CHARACTERS",
        "MISSIONS",
        "MAPS",
        "OPTIONS",
        "COMING SOON",
        "EXTRA CHARACTERS",
        "PREVIEW",
        "FUTURE",
        "PREVIEW",
        "CONFIRMED",
        "ACTIVE",
        "Goku (Early)",
        "Goku (Mid)",
        "Goku (Mid)",
        "Super Saiyan",
        "Goku",
        "(Early)",
        "Goku",
        "(Mid)",
        "Goku (Mid)",
        "Super Saiyan",
        "ID",
        "PLAYER",
        "COSTUME",
        "CHARACTERS",
        "COSTUMES",
        "MISSIONS - COMING SOON",
        "New challenges will be added.",
        "MAPS - COMING SOON",
        "Additional maps are in development.",
        "OPTIONS - COMING SOON",
        "More options will be added.",
        "LEFT/RIGHT: MOVE  UP/DOWN: FOCUS  L/R: PLAYER",
        "X: CONFIRM   TRIANGLE: TAB   SELECT: LANGUAGE",
        "SQUARE: CLEAR  CIRCLE: CLOSE",
        "TEXTURES NOT LOADED - CHECK DIAGNOSTIC",
        "TRAINING CHALLENGES TEMPORARILY DISABLED",
    },
    {
        "TTT EXPANSION",
        "PERSONAJES",
        "MISIONES",
        "MAPAS",
        "OPCIONES",
        "PROXIMAMENTE",
        "PERSONAJES EXTRA",
        "VISTA PREVIA",
        "FUTURO",
        "PREVIA",
        "CONFIRMADO",
        "ACTIVO",
        "Goku (Inicio)",
        "Goku (Medio)",
        "Goku (Medio)",
        "Super Saiyajin",
        "Goku",
        "(Inicio)",
        "Goku",
        "(Medio)",
        "Goku (Medio)",
        "Super Saiyajin",
        "ID",
        "JUGADOR",
        "TRAJE",
        "PERSONAJES",
        "TRAJES",
        "MISIONES - PROXIMAMENTE",
        "Se agregaran nuevos desafios.",
        "MAPAS - PROXIMAMENTE",
        "Mas escenarios en desarrollo.",
        "OPCIONES - PROXIMAMENTE",
        "Mas configuraciones en desarrollo.",
        "IZQ/DER: MOVER  ARR/ABA: FOCO  L/R: JUGADOR",
        "X: CONFIRMAR  TRIANGULO: PESTANA  SELECT: IDIOMA",
        "CUADRADO: LIMPIAR  CIRCULO: CERRAR",
        "TEXTURAS NO CARGADAS - VER DIAGNOSTICO",
        "DESAFIOS TEMPORALMENTE DESACTIVADOS",
    },
};
static void copy_text(char *out,const char *in){
    unsigned i=0;while(in[i] && i<TTT_UI_TEXT_MAX-1u){out[i]=in[i];i++;}out[i]=0;
}
static unsigned sz(const char*s){unsigned i=0;while(s[i])i++;return i;}
static char fold_accent(unsigned char c){
    /* UTF-8 C3 + continuation: Latin-1 common accented letters. */
    switch(c){
        case 0x80:case 0x81:case 0x82:case 0x83:case 0x84:case 0x85:return 'A';
        case 0x87:return 'C';
        case 0x88:case 0x89:case 0x8A:case 0x8B:return 'E';
        case 0x8C:case 0x8D:case 0x8E:case 0x8F:return 'I';
        case 0x91:return 'N';
        case 0x92:case 0x93:case 0x94:case 0x95:case 0x96:return 'O';
        case 0x99:case 0x9A:case 0x9B:case 0x9C:return 'U';
        case 0x9D:return 'Y';
        case 0x9F:return 's';
        case 0xA0:case 0xA1:case 0xA2:case 0xA3:case 0xA4:case 0xA5:return 'a';
        case 0xA7:return 'c';
        case 0xA8:case 0xA9:case 0xAA:case 0xAB:return 'e';
        case 0xAC:case 0xAD:case 0xAE:case 0xAF:return 'i';
        case 0xB1:return 'n';
        case 0xB2:case 0xB3:case 0xB4:case 0xB5:case 0xB6:return 'o';
        case 0xB9:case 0xBA:case 0xBB:case 0xBC:return 'u';
        case 0xBD:case 0xBF:return 'y';
        default:return '?';
    }
}
static void normalize(char *out,const char *in){
    unsigned i=0,j=0;unsigned char c;
    while((c=(unsigned char)in[i])!=0 && j<TTT_UI_TEXT_MAX-1u){
        if(c<32u){i++;continue;}
        if(c<128u){out[j++]=(char)c;++i;continue;}
        if(c==0xC3u && in[i+1]){out[j++]=fold_accent((unsigned char)in[i+1]);i+=2;continue;}
        if(c==0xE2u && (unsigned char)in[i+1]==0x80u && in[i+2]){
            unsigned char q=(unsigned char)in[i+2];
            out[j++]=(q==0x98u||q==0x99u)?'\'':(q==0x9Cu||q==0x9Du)?'"':'-';i+=3;continue;
        }
        out[j++]='?'; ++i;
        while(((unsigned char)in[i]&0xC0u)==0x80u && in[i])i++;
    }
    out[j]=0;
}
void ttt_text_defaults(void){
    unsigned l,k;for(l=0;l<TTT_UI_TEXT_LANGS;l++){
        stats[l].open_rc=0;stats[l].bytes_read=0;stats[l].overrides=0;stats[l].invalid_lines=0;
        for(k=0;k<TTT_TEXT_COUNT;k++)copy_text(labels[l][k],defaults[l][k]);
    }
}
const char*ttt_text(unsigned lang,TTTTextKey key){
    if(lang>=TTT_UI_TEXT_LANGS)lang=0;
    if((unsigned)key>=TTT_TEXT_COUNT)return "";
    return labels[lang][(unsigned)key][0]?labels[lang][(unsigned)key]:defaults[lang][(unsigned)key];
}
TTTTextFileStats ttt_text_stats(unsigned lang){
    TTTTextFileStats empty={0,0,0,0};return lang<TTT_UI_TEXT_LANGS?stats[lang]:empty;
}
int ttt_text_parse_line(unsigned lang,const char *line){
    unsigned k,i=0,j=0,end;const char *equal;
    char key[64],clean[TTT_UI_TEXT_MAX];
    if(lang>=TTT_UI_TEXT_LANGS||!line)return -1;
    while(*line==' '||*line=='\t')line++;
    if(!*line||*line=='#'||*line==';')return 0;
    equal=line;while(*equal && *equal!='=' && *equal!='\r' && *equal!='\n')equal++;
    if(*equal!='='){stats[lang].invalid_lines++;return -2;}
    while(line+i<equal && i<sizeof(key)-1u){key[i]=line[i];i++;}
    while(i && (key[i-1]==' '||key[i-1]=='\t'))i--;
    key[i]=0;
    equal++;while(*equal==' '||*equal=='\t')equal++;
    end=sz(equal);
    while(end && (equal[end-1]==' '||equal[end-1]=='\t'||equal[end-1]=='\r'||equal[end-1]=='\n'))end--;
    if(!end)return 0; /* empty value keeps default */
    /* Trim to bounded buffer without trusting remote or local input sizes. */
    char val[TTT_UI_TEXT_MAX*4];
    for(j=0;j<end && j<sizeof(val)-1u;j++){val[j]=equal[j];}
    val[j]=0;
    normalize(clean,val);
    for(k=0;k<TTT_TEXT_COUNT;k++){
        const char *name=keynames[k];unsigned n=0;
        while(name[n] && name[n]==key[n])n++;
        if(!name[n]&&!key[n]){
            if(clean[0]){copy_text(labels[lang][k],clean);stats[lang].overrides++;}
            return 1;
        }
    }
    stats[lang].invalid_lines++;
    return -3;
}
void ttt_text_load_external(void){
    static const char *const paths[3]={
        "ms0:/PSP/PLUGINS/TTT_EXPANSION/lang/pt.txt",
        "ms0:/PSP/PLUGINS/TTT_EXPANSION/lang/en.txt",
        "ms0:/PSP/PLUGINS/TTT_EXPANSION/lang/es.txt"
    };
    unsigned lang;
    ttt_text_defaults();
#if defined(TTT_BRIDGE_PSP)
    for(lang=0;lang<TTT_UI_TEXT_LANGS;lang++){
        SceUID f=sceIoOpen(paths[lang],PSP_O_RDONLY,0);
        char line[512],block[512];unsigned pos=0;int too_long=0,n;
        if(f<0){stats[lang].open_rc=(int)f;continue;}
        while(stats[lang].bytes_read<TTT_UI_TEXT_LINES_MAX){
            unsigned x;
            int request=TTT_UI_TEXT_LINES_MAX-stats[lang].bytes_read;
            if(request>(int)sizeof(block))request=sizeof(block);
            n=sceIoRead(f,block,request);
            if(n<=0)break;
            stats[lang].bytes_read+=n;
            for(x=0;x<(unsigned)n;x++){
                char ch=block[x];
                if(ch=='\n'){
                    if(!too_long){line[pos]=0;ttt_text_parse_line(lang,line);}
                    else stats[lang].invalid_lines++;
                    pos=0;too_long=0;
                }else if(pos<sizeof(line)-1u){line[pos++]=ch;}
                else too_long=1;
            }
        }
        if(pos && !too_long){line[pos]=0;ttt_text_parse_line(lang,line);}
        sceIoClose(f);
    }
#else
    (void)paths;(void)lang;
#endif
}
