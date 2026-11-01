#ifndef F1TYREAGE_H
#define F1TYREAGE_H
#include <string.h>
typedef struct {int seen,lap,stops,age;} F1TyreAge;
static F1TyreAge tyreAge[41];
static void F1TyreAgeReset(void){memset(tyreAge,0,sizeof(tyreAge));}
/* Use the reconciled PITS total, including native accelerated-time stops. */
static int F1TyreAgeTick(int id,int lap,int stops)
{
 F1TyreAge *s;int completed,previous;
 if(id<1 || id>40 || lap<0)return -1;s=&tyreAge[id];
 if(s->seen && (lap<s->lap || stops<s->stops))memset(s,0,sizeof(*s));
 completed=lap>1?lap-1:0;
 previous=s->lap>1?s->lap-1:0;
 if(!s->seen)s->age=completed;
 else if(completed>previous)s->age+=completed-previous;
 if(s->seen && stops>s->stops)s->age=0;
 s->seen=1;s->lap=lap;s->stops=stops;
 return s->age;
}
#endif
