#ifndef F1TOWER_H
#define F1TOWER_H
void F1Update(void);
void F1Reset(void);
void F1DiagBoot(void);
void F1LogKey(unsigned int scan);
void F1DrawDiagnostic(void);
void F1Toggle(void);
void F1ToggleCard(void);
void F1ToggleDriver(void);
void F1ToggleMap(void);
void F1MapZoom(void);
int F1SuppressNativeFastest(void);
void F1Tab(unsigned int scan);
void F1RaceSession(int fresh);
void F1ControlInit(void);
void F1ControlKey(unsigned int scan);
void F1ControlCancel(void);
#endif
