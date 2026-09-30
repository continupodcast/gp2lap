#ifndef F1MICRO_H
#define F1MICRO_H
typedef struct {
    int seen,lap,haveCross,newLap; double position,cross;
    unsigned long clock; long personal[30]; unsigned char colors[30];
} F1MicroCar;
typedef struct { F1MicroCar cars[41]; long best[30]; } F1MicroState;
void F1MicroReset(F1MicroState *s);
void F1MicroSample(F1MicroState *s,int id,int lap,double position,unsigned long clock,int valid);
#endif
