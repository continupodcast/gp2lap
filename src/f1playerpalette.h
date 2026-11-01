#ifndef F1PLAYERPALETTE_H
#define F1PLAYERPALETTE_H
/* A startup fade may leave every native DAC entry black. The modal suspends
   that frame, so it needs its own known 6-bit palette until it returns. */
static void F1PlayerPaletteBuild(unsigned char *pal)
{
 static const unsigned char base[9][3]={{0,0,0},{4,5,7},{8,10,13},{60,61,62},{39,42,47},{56,8,12},{21,24,34},{31,41,51},{8,10,19}};
 static const unsigned char level[6]={0,12,24,36,48,63};int i,r,g,b,index;
 memset(pal,0,768);memcpy(pal,base,sizeof(base));index=16;
 for(r=0;r<6;r++)for(g=0;g<6;g++)for(b=0;b<6;b++){
  pal[index*3]=level[r];pal[index*3+1]=level[g];pal[index*3+2]=level[b];index++;
 }
 for(i=232;i<256;i++)pal[i*3]=pal[i*3+1]=pal[i*3+2]=(unsigned char)((i-232)*63/23);
}
static unsigned int F1PlayerPaletteCapture(unsigned char *pal)
{
 int i;unsigned int mask=inp(0x3c6);outp(0x3c7,0);
 for(i=0;i<768;i++)pal[i]=(unsigned char)inp(0x3c9);
 return mask;
}
static void F1PlayerPalettePresent(const unsigned char *pal,unsigned int mask)
{
 int i;outp(0x3c8,0);for(i=0;i<768;i++)outp(0x3c9,pal[i]);outp(0x3c6,mask);
}
#endif
