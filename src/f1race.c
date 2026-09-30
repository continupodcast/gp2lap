/* Session-local grid baseline and manual race tower pages. */
#include <string.h>
#include "f1race.h"
static int grid[41], pendingGrid, mode, tabDown;
void F1RaceReset(int fresh)
{
    memset(grid,0,sizeof(grid));pendingGrid=fresh;
}
void F1RaceStats(F1Row *rows,int count)
{
    int i,id,seen[41],positions[27],valid=1;
    if(!rows || count<1 || count>26) return;
    if(pendingGrid) {
        memset(seen,0,sizeof(seen));memset(positions,0,sizeof(positions));
        for(i=0;i<count;i++) {
            id=rows[i].id;
            if(id<1 || id>40 || rows[i].pos<1 || rows[i].pos>count) { valid=0;break; }
            if(seen[id] || positions[rows[i].pos] || rows[i].lap>1) { valid=0;break; }
            seen[id]=1;positions[rows[i].pos]=1;
        }
        if(valid) {
            for(i=0;i<count;i++) grid[rows[i].id]=rows[i].pos;
            pendingGrid=0;
        }
    }
    for(i=0;i<count;i++) {
        id=rows[i].id;
        rows[i].gain=id>=1 && id<=40 && grid[id]?grid[id]-rows[i].pos:F1_GAIN_UNKNOWN;
    }
}
void F1RaceKey(unsigned int scan,int enabled)
{
    if(scan==0x8f) tabDown=0;
    else if(scan==0x0f) {
        if(enabled && !tabDown) mode=(mode+1)%4;
        tabDown=1;
    }
}
int F1RaceMode(void) { return mode; }
