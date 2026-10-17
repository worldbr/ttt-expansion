#include "input_hook.h"
int ttt_input_hook_plan(uint32_t caller,uint32_t replacement,uint32_t observed,uint32_t*out) {
    if(!out)return -14;
    if((caller&3u)||(replacement&3u))return -11;
    if((caller & UINT32_C(0xF0000000))!=(replacement & UINT32_C(0xF0000000)))return -10;
    if(observed!=TTT_INPUT_ORIGINAL_JAL)return -12;
    *out=UINT32_C(0x0C000000)|((replacement>>2)&UINT32_C(0x03FFFFFF));
    return 0;
}
