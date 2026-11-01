#ifndef F1COMPOUNDSTATE_H
#define F1COMPOUNDSTATE_H
/* Active deferred player-compound resolution. No retired diagnostic output. */
int F1EffectivePlayerCompound(const int *map)
{
 int i,n,id,stops;GP2Car *c;
 if(!map || !pCarStructs || !pNumCars)return -1;
 n=*pNumCars;if(n<1 || n>28)return -1;if(n>26)n=26;
 for(i=0;i<n;i++){
  c=&pCarStructs[i];id=c->id&63;
  if(!(c->id&128) || id<1 || id>40)continue;
  stops=c->numPitStops;if(stops>3)continue;
  /* Synchronize compound only, preserving stop count and pit schedule. */
  if(pCarSetups)pCarSetups[id-1].compound=(BYTE)map[stops];
  return map[stops];
 }
 return -1;
}
#endif
