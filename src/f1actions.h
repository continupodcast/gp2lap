#ifndef F1ACTIONS_H
#define F1ACTIONS_H
typedef struct {
    unsigned char targets[41];
    unsigned long start,last,duration;
    int active,held;
} F1FuelState;
void F1FuelCancel(F1FuelState *s);
int F1FuelKey(F1FuelState *s,unsigned int scan);
void F1FuelStart(F1FuelState *s,const unsigned char *targets,unsigned long now,unsigned long duration);
int F1FuelTick(F1FuelState *s,unsigned long now,int allowed);
int F1FuelTarget(const F1FuelState *s,unsigned int id);
int F1PatchCameraCaption(unsigned char *code);
int F1PatchRetirementCaption(unsigned char *code);
#endif
