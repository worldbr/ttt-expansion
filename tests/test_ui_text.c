#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ui_text.h"
int main(void){
    ttt_text_defaults();
    assert(TTT_TEXT_COUNT>=35);
    assert(strcmp(ttt_text(0,TTT_TEXT_TAB_CHARACTERS),"PERSONAGENS")==0);
    assert(strcmp(ttt_text(0,TTT_TEXT_TAB_MISSIONS),"MISSOES")==0);
    assert(strcmp(ttt_text(0,TTT_TEXT_COMING_SOON),"EM BREVE")==0);
    assert(strcmp(ttt_text(2,TTT_TEXT_TAB_OPTIONS),"OPCIONES")==0);
    assert(ttt_text_parse_line(0,"# comentario")==0);
    assert(ttt_text_parse_line(0,"CHAR2_SUBTITLE=Super Saiyajin Azul")==1);
    assert(strcmp(ttt_text(0,TTT_TEXT_CHAR2_SUBTITLE),"Super Saiyajin Azul")==0);
    assert(ttt_text_parse_line(0,"TAB_MISSIONS = Miss\303\265es")==1);
    assert(strcmp(ttt_text(0,TTT_TEXT_TAB_MISSIONS),"Missoes")==0);
    assert(ttt_text_parse_line(0,"FOOTER_MOVE= ")==0);
    assert(ttt_text_parse_line(0,"UNKNOWN_KEY=foo")==-3);
    assert(ttt_text_parse_line(3,"TITLE=invalid")==-1);
    assert(ttt_text_parse_line(1,"FOOTER_CLOSE=Circle: Close\r")==1);
    assert(strcmp(ttt_text(1,TTT_TEXT_FOOTER_CLOSE),"Circle: Close")==0);
    assert(ttt_text_stats(0).overrides==2);
    assert(ttt_text_stats(0).invalid_lines==1);
    assert(ttt_text_parse_line(0,"FOOTER_CLOSE=abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz0123456789abcdefghijklmnop")==1);
    assert(strlen(ttt_text(0,TTT_TEXT_FOOTER_CLOSE))<=TTT_UI_TEXT_MAX-1u);
    ttt_text_defaults();
    assert(!strcmp(ttt_text(0,TTT_TEXT_CHAR2_SUBTITLE),"Super Saiyajin"));
    puts("OK: external UI labels, PT/EN/ES, UTF-8 normalization, validation, fallback");
    return 0;
}
