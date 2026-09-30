#ifndef F1QUALY_H
#define F1QUALY_H
#define F1_Q_MAX 28
#define F1_Q_W 116
#define F1_Q_HEADER 36
#define F1_CARD_X 422
#define F1_CARD_Y_TV 302
#define F1_CARD_Y_COCKPIT 388
#define F1_CARD_W 210
#define F1_CARD_H 84
/* Time zero is unavailable. Input times retain GP2 validity bits. */
typedef struct {
    int id,active,lap,split,pit;
    unsigned long start,best,best1,best2,last,s1,s2;
} F1QCar;
typedef struct {
    int id,pos,pit,outlap;
    long best,pb[3],live[2];
    int color[2];
} F1QRow;
typedef struct {
    int id,pos,pit,outlap,post,pole,showGap,hasGap,leaderId;
    long running,last,gap,leaderBest;
    int color[3];
} F1QCard;
/* 0 empty, 1 personal improvement, 2 slower, 3 overall best */
void F1QReset(void);
void F1QTick(const F1QCar *cars,int count,unsigned long clock);
const F1QRow *F1QRows(int *count);
int F1QGetCard(int id,unsigned long clock,F1QCard *card);
long F1QGlobal(int sector);
void F1QTime(char *label,long ms,int sector);
void F1QLiveTime(char *label,long ms);
int F1QWindow(const F1QRow *rows,int count,int focus,int *start);
void F1QDelta(char *label,long ms);
void F1RenderQualy(unsigned char *dst,const unsigned char *pal,
    const F1QRow *rows,int count,long remaining,int practice,int gaps,int compact,int focus);
void F1RenderQualyCard(unsigned char *dst,const unsigned char *pal,const F1QCard *card,int cockpit);
void F1QualyUpdate(void);
void F1QualyCompose(unsigned char *dst,const unsigned char *pal,int focus,int table,int card,int cockpit);
void F1CockpitBefore(void);
void F1CockpitAfter(void);
void F1VideoReset(void);
#endif
