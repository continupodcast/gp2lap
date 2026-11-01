#ifndef F1PLAYERSHUFFLE_H
#define F1PLAYERSHUFFLE_H
/* Independent UI generator: never consume the NPC allocation random stream. */
static unsigned long F1PlayerShuffleNext(unsigned long *state)
{
 *state=(*state*1664525UL+1013904223UL)&0xffffffffUL;return *state;
}
static void F1PlayerShuffle(int *order,int count,unsigned long *state)
{
 int i,j,tmp;for(i=0;i<count;i++)order[i]=i;
 for(i=count-1;i>0;i--){j=(int)((F1PlayerShuffleNext(state)>>8)%(i+1));tmp=order[i];order[i]=order[j];order[j]=tmp;}
}
#endif
