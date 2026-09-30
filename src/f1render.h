#ifndef F1RENDER_H
#define F1RENDER_H
#define F1_MAX_ROWS 26
#define F1_PANEL_X 8
#define F1_PANEL_Y 8
#define F1_PANEL_W 104
#define F1_HEADER_H 34
#define F1_ROW_H 13
/* -1 gap means unavailable. Palette entries are VGA 6-bit RGB. */
typedef struct { int id, pos, lap, out, pit, focused; long gap; int gain, stops; long leaderGap; int lapsBehind, fastest, positionChange; } F1Row;
int F1Window(const F1Row *rows, int count, int focusId, int *start);
void F1Gap(char *label, long milliseconds);
void F1Render(unsigned char *screen, const unsigned char *palette,
              const F1Row *rows, int count, int lap, int total);
#define F1_GAIN_UNKNOWN 1000
#define F1_DRIVER_W 200
#define F1_DRIVER_H 38
#define F1_DRIVER_X ((640-F1_DRIVER_W)/2)
#define F1_DRIVER_Y F1_PANEL_Y
void F1RenderRace(unsigned char *screen, const unsigned char *palette,
    const F1Row *rows, int count, int lap, int total, int mode);
void F1RenderDriver(unsigned char *screen, const unsigned char *palette, int id, int position);
int F1DriverTop(int cockpit,int qualy,int rows);
int F1DriverLeft(int cockpit);
void F1RenderDriverAtXY(unsigned char *screen,const unsigned char *palette,int id,int position,int x,int y);
/* In TV view GP2 only shows the first 388 lines of its image buffer (the same
   limit that places the modern qualy card at 302..386). Anything drawn lower is
   never displayed, so the 90s bottom-centre slot ends 8 pixels above it. */
#define F1_TV_VISIBLE_LINES 388
#define F1_TV_SLOT_BOTTOM (F1_TV_VISIBLE_LINES-8)
/* 90s theme: in TV view the driver plate shares the bottom-centre slot with
   DIFFERENCE; key 5 cycles plate / DIFFERENCE / off. */
#define F1_P90_TV_X ((640-F1_DRIVER_W)/2)
#define F1_P90_TV_Y (F1_TV_SLOT_BOTTOM-F1_DRIVER_H)
#define F1_B90_H 30
void F1Render90Difference(unsigned char *screen,const unsigned char *palette,const F1Row *rows,int count,int focus);
void F1Render90Fastest(unsigned char *screen,const unsigned char *palette,int id,long ms,int y);
void F1RenderDriverAt(unsigned char *screen,const unsigned char *palette,int id,int position,int y);
void F1RenderNotice(unsigned char *screen,const unsigned char *palette,const char *message);
void F1RenderMicro(unsigned char *screen,const unsigned char *palette,const unsigned char *colors,int cockpit);
#define F1_MAP_X 492
#define F1_MAP_Y 8
#define F1_MAP_SIZE 140
typedef struct { double x,y; } F1MapPoint;
typedef struct { int id,pos,out; double x,y,angle; } F1MapCar;
void F1RenderMap(unsigned char *dst,const unsigned char *pal,const F1MapPoint *track,int count,
    const F1MapPoint *pits,int pitcount,const F1MapCar *cars,int n,int focus);
void F1RenderFullMap(unsigned char *dst,const unsigned char *pal,const F1MapPoint *track,int count,
    const F1MapPoint *pits,int pitcount,const F1MapCar *cars,int n,int focus);
#endif
