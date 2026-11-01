#ifndef F1PLAYERUI_H
#define F1PLAYERUI_H
typedef struct {char sequence[48],pits[24];int stops,fuel,count;unsigned char compound[4],length[4];} F1StrategyOption;
void F1RenderPlayerStrategy(unsigned char *,const unsigned char *,const F1StrategyOption *,int,int,int,int,const F1StrategyOption *);
void F1RenderPlayerPitStrategy(unsigned char *,const unsigned char *,const F1StrategyOption *,int,int,int,int,int);
void F1RenderStrategyEditor(unsigned char *,const unsigned char *,const F1StrategyOption *,int,int,int,int,int,int,int,const char *);
void F1StrategyCircuitSet(const char *,const double *,int);
#endif
