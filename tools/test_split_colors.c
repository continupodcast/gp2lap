#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/f1timing.c"
#include "f1micro.h"
static F1QCar a,b;
static F1QCard card;
static void tick(unsigned long clock,int reverse) {
 F1QCar cars[2];cars[reverse?1:0]=a;cars[reverse?0:1]=b;
 F1QTick(cars,2,clock);
}
static void setup(void) {
 F1QReset();memset(&a,0,sizeof(a));memset(&b,0,sizeof(b));
 a.id=1;a.active=1;a.lap=2;a.split=2;a.start=1000;
 a.best=90000;a.best1=30000;a.best2=60000;a.last=90000;
 b=a;b.id=2;b.best=96000;b.best1=32000;b.best2=64000;b.last=96000;
 tick(1100,0);
}
static void segment(F1MicroState *s,int id,unsigned long base,int third,int fourth) {
 F1MicroSample(s,id,0,.8,base,1);F1MicroSample(s,id,0,1.2,base+100,1);
 F1MicroSample(s,id,0,1.8,base+third,1);F1MicroSample(s,id,0,2.2,base+fourth,1);
}
int main(void) {
 F1MicroState m;int reverse;
 /* Green even when slower than pole; yellow when slower than own PB. */
 setup();b.split=0;b.s1=31000;tick(32000,0);
 assert(F1QGetCard(2,32000,&card) && card.color[0]==1);
 tick(32100,0);assert(F1QGetCard(2,32100,&card) && card.color[0]==1);
 setup();b.split=0;b.s1=32500;tick(33500,0);
 assert(F1QGetCard(2,33500,&card) && card.color[0]==2);
 setup();b.split=1;b.s1=31000;b.s2=62000;tick(63000,0);
 assert(F1QGetCard(2,63000,&card) && card.color[0]==1 && card.color[1]==1);
 /* No dependence on input order; superseded session purple becomes green. */
 for(reverse=0;reverse<2;reverse++) {
  setup();a.split=0;a.s1=29000;b.split=0;b.s1=28000;tick(31000,reverse);
  assert(F1QGetCard(1,31000,&card) && card.color[0]==1);
  assert(F1QGetCard(2,31000,&card) && card.color[0]==3);
 }
 /* A displayed completed lap follows later session records, including S3. */
 setup();a.lap=3;a.start=88000;a.last=87000;a.s1=29000;a.s2=58000;
 a.best=87000;a.best1=29000;a.best2=58000;tick(88000,0);
 assert(F1QGetCard(1,88000,&card) && card.post && card.color[2]==3);
 b.lap=3;b.start=89000;b.last=84000;b.s1=28000;b.s2=56000;
 b.best=84000;b.best1=28000;b.best2=56000;tick(89000,0);
 assert(F1QGetCard(1,89000,&card) && card.post);
 assert(card.color[0]==1 && card.color[1]==1 && card.color[2]==1);
 assert(F1QGetCard(2,89000,&card) && card.color[2]==3);
 /* The first valid measured sector is purple immediately, then stable. */
 F1QReset();memset(&a,0,sizeof(a));memset(&b,0,sizeof(b));
 a.id=1;a.active=1;a.lap=2;a.start=1000;a.split=2;
 tick(1000,0);a.split=0;a.s1=30000;tick(31000,0);
 assert(F1QGetCard(1,31000,&card) && card.color[0]==3);
 tick(31100,0);assert(F1QGetCard(1,31100,&card) && card.color[0]==3);
 /* Native PB loaded after accelerated time remains a personal reference. */
 setup();b.best=95000;b.best1=31000;b.best2=63000;tick(200000,0);
 b.lap=3;b.start=200000;tick(200100,0);
 b.split=0;b.s1=31500;tick(231500,0);
 assert(F1QGetCard(2,231500,&card) && card.color[0]==2);
 /* Same rule for measured microsectors, including retroactive purple. */
 F1MicroReset(&m);segment(&m,1,0,300,500);assert(m.cars[1].colors[1]==3);
 segment(&m,2,0,400,600);assert(m.cars[2].colors[1]==1);
 segment(&m,2,1000,350,550);assert(m.cars[2].colors[1]==1);
 segment(&m,2,2000,450,650);assert(m.cars[2].colors[1]==2);
 segment(&m,2,3000,200,400);assert(m.cars[2].colors[1]==3 && m.cars[1].colors[1]==1);
 F1MicroSuspend(&m);assert(m.best[1]==250 && m.cars[2].personal[1]==250);
 puts("PASS: personal/session sector and microsector colours, stable first record, input order, retroactive purple, native PB updates and suspension.");
 return 0;
}
