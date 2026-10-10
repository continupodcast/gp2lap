#ifndef F1ADVANCED_H
#define F1ADVANCED_H
extern int F1MicroMode;
double F1TrackFraction(unsigned long address,unsigned int factor);
double F1TrackMicroPosition(unsigned long address,unsigned int factor);
void F1AccEvent(const char *event);
void F1AdvancedReport(void);
void F1AdvancedReset(void);
void F1AdvancedSuspend(void);
void F1AdvancedUpdate(void);
void F1AdvancedCard(unsigned char *dst,const unsigned char *pal,int focus,int cockpit);
#endif
