#include <assert.h>
#include <string.h>
#include "../../src/f1render.h"
#include "../../src/f1qualy.h"
static unsigned char image[640*480],saved[640*480];
int main(void){
 memset(image,0x5a,sizeof(image));memcpy(saved,image,sizeof(image));
 /* Null telemetry and palette also prove omitted modules do not touch native data. */
 F1RenderRace(image,0,0,26,1,53,0);
 F1RenderQualy(image,0,0,26,0,0,0,0,0);
 F1RenderQualyCard(image,0,0,0);F1RenderMicro(image,0,0,0);
 F1RenderDriverAtXY(image,0,1,1,0,0);
 F1Render90Difference(image,0,0,26,1);F1RenderCurrentGap(image,0,0,26,1);
 F1Render90Fastest(image,0,1,83000,0);F1RenderModernFastest(image,0,1,83000,0);
 F1RenderMap(image,0,0,0,0,0,0,0,0);F1RenderFullMap(image,0,0,0,0,0,0,0,0);
 F1RenderPitDisplays(image,0,0,26);
 F1RenderGridMenu(image,0,0,0,0,0,0,0,0,0,0);
 F1RenderGridControl(image,0,0,0,0,0,0);
 assert(!memcmp(image,saved,sizeof(image)));return 0;
}
