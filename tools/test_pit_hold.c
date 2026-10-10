#include <assert.h>
#include <string.h>
#include "f1pitdisplay.h"
int main(void) {
    F1PitDisplayReset();
    F1PitDisplayTick(34,1,1000);F1PitDisplayTick(26,1,2000);
    F1PitDisplayTick(34,1,25300);F1PitDisplayTick(34,0,25400);
    assert(pitClocks[34].active==2 && pitClocks[34].elapsed==24400);
    F1PitDisplayTick(34,0,30399);assert(pitClocks[34].active==2 && pitClocks[34].elapsed==24400);
    F1PitDisplayTick(26,1,30399);assert(pitClocks[26].active==1 && pitClocks[26].elapsed==28399);
    F1PitDisplayTick(34,0,30400);assert(!pitClocks[34].active);
    F1PitDisplayTick(34,1,30000);F1PitDisplayTick(34,0,31000);F1PitDisplayTick(34,1,32000);
    assert(pitClocks[34].active==1 && pitClocks[34].elapsed==0);
    F1PitDisplayTick(34,0,33000);F1PitDisplayTick(34,0,100);assert(!pitClocks[34].active);
    F1PitDisplayReset();assert(!pitClocks[26].active);
    return 0;
}
