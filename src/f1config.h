#ifndef F1CONFIG_H
#define F1CONFIG_H
extern int f1_gap_start;
extern unsigned long f1_gap_update,f1_position_time;

#define F1_DRIVER_COUNT 28
#define F1_TEAM_COUNT 14
#define F1_ABBR_LEN 8
#define F1_NAME_LEN 24
#define F1_TEAM_NAME_LEN 24
#define F1_TEAM_MARK_LEN 3

typedef struct { int id, number, team; char abbr[F1_ABBR_LEN], last[F1_NAME_LEN], first[F1_NAME_LEN]; } F1Driver;

extern F1Driver f1_drivers[F1_DRIVER_COUNT];
extern char f1_team_names[F1_TEAM_COUNT][F1_TEAM_NAME_LEN];
extern unsigned char f1_colors[F1_TEAM_COUNT][3];
extern unsigned char f1_team_logo_enabled[F1_TEAM_COUNT];
extern char f1_team_marks[F1_TEAM_COUNT][F1_TEAM_MARK_LEN];
extern unsigned char f1_custom_logos[F1_TEAM_COUNT][192][4];
extern unsigned char f1_custom_logo_loaded[F1_TEAM_COUNT];
extern int f1_hide_camera_caption, f1_fuel_enabled;
extern unsigned long f1_fuel_duration;
extern unsigned char f1_fuel_targets[41];
extern int f1_micro_enabled, f1_hide_retirement_caption;
/* 0 = modern F1 graphics, 1 = late-1990s broadcast graphics. */
#define F1_THEME_MODERN 0
#define F1_THEME_90S 1
extern int f1_theme;

/* Returns 1 for custom data, 0 for compiled defaults, -1 for invalid custom data. */
int F1ConfigLoad(const char *cfgname);
void F1ConfigEnsure(void);

#endif
