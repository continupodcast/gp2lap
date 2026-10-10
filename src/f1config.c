#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "f1config.h"
#include "f1defaults.h"

F1Driver f1_drivers[F1_DRIVER_COUNT];
char f1_team_names[F1_TEAM_COUNT][F1_TEAM_NAME_LEN];
unsigned char f1_colors[F1_TEAM_COUNT][3];
unsigned char f1_team_logo_enabled[F1_TEAM_COUNT];
char f1_team_marks[F1_TEAM_COUNT][F1_TEAM_MARK_LEN];
static int configured;
int f1_gap_start=50;
unsigned long f1_gap_update=4000,f1_position_time=1000;
int f1_hide_camera_caption=0, f1_fuel_enabled;
int f1_allow_external_pit_camera=0;
int f1_hide_riding_caption,f1_hide_viewing_caption,f1_hide_winner_caption;
int f1_hide_pit_caption,f1_hide_pause_caption;
unsigned long f1_fuel_duration=3000;
unsigned char f1_fuel_targets[41];
int f1_micro_enabled=1, f1_hide_retirement_caption=0;
int f1_theme=F1_THEME_MODERN;
unsigned char f1_custom_logos[F1_TEAM_COUNT][192][4];
unsigned char f1_custom_logo_loaded[F1_TEAM_COUNT];
static char logo_files[F1_TEAM_COUNT][128];

static void copytext(char *dst,const char *src,int n)
{
    int i=0; while(src[i] && i<n-1) { dst[i]=src[i]; i++; } dst[i]=0;
}
static void defaults(void)
{
    int i; memcpy(f1_drivers,f1_default_drivers,sizeof(f1_drivers));
    f1_gap_start=50;f1_gap_update=4000;f1_position_time=1000;
    memcpy(f1_team_names,f1_default_team_names,sizeof(f1_team_names));
    memcpy(f1_colors,f1_default_colors,sizeof(f1_colors));
    strcpy(f1_team_names[13],"TEAM 14");
    memset(f1_colors[13],255,3);
    memset(logo_files,0,sizeof(logo_files));
    f1_hide_camera_caption=0; f1_fuel_enabled=0; f1_allow_external_pit_camera=0;
    f1_hide_riding_caption=f1_hide_viewing_caption=f1_hide_winner_caption=0;
    f1_hide_pit_caption=f1_hide_pause_caption=0; f1_fuel_duration=3000;
    f1_micro_enabled=1;f1_hide_retirement_caption=0;f1_theme=F1_THEME_MODERN;
    memset(f1_fuel_targets,0,sizeof(f1_fuel_targets));
    memset(f1_custom_logo_loaded,0,sizeof(f1_custom_logo_loaded));
    for(i=0;i<F1_TEAM_COUNT;i++) { f1_team_logo_enabled[i]=1; f1_team_marks[i][0]=f1_team_names[i][0]; f1_team_marks[i][1]=f1_team_names[i][1]?f1_team_names[i][1]:' '; f1_team_marks[i][2]=0; }
    f1_team_logo_enabled[13]=0;
}
void F1ConfigEnsure(void)
{
    if(!configured) { defaults(); configured=1; }
}
static char *trim(char *s)
{
    char *e; while(*s && isspace((unsigned char)*s)) s++;
    e=s+strlen(s); while(e>s && isspace((unsigned char)e[-1])) *--e=0;
    return s;
}
static int same(const char *a,const char *b)
{
    while(*a && *b) { if(tolower((unsigned char)*a++)!=tolower((unsigned char)*b++)) return 0; } return !*a && !*b;
}
static void stringvalue(char *dst,const char *src,int n)
{
    int i=0; src=trim((char *)src); if(*src=='\"') src++;
    while(*src && *src!='\"' && *src!=';' && i<n-1) dst[i++]=*src++;
    while(i && isspace((unsigned char)dst[i-1])) i--;
    dst[i]=0;
}
static int hexcolor(const char *s,unsigned char *rgb)
{
    char v[16]; char *e; long n; stringvalue(v,s,sizeof(v)); if(v[0]=='#') memmove(v,v+1,strlen(v));
    if(strlen(v)!=6) return 0;
    n=strtol(v,&e,16);
    if(*e) return 0;
    rgb[0]=(unsigned char)(n>>16); rgb[1]=(unsigned char)(n>>8); rgb[2]=(unsigned char)n; return 1;
}
static int valid(void)
{
    int i,j; if((f1_drivers[26].id==0)!=(f1_drivers[27].id==0)) return 0; for(i=0;i<F1_DRIVER_COUNT;i++) { if(i>=26 && !f1_drivers[i].id) continue; if(f1_drivers[i].id<1 || f1_drivers[i].id>40 || f1_drivers[i].team<0 || f1_drivers[i].team>=F1_TEAM_COUNT) return 0; for(j=0;j<i;j++) if(f1_drivers[j].id==f1_drivers[i].id) return 0; } return 1;
}
/* Fixed-size RGBA sprites, resolved beside the selected CFG, never per frame. */
static void loadlogos(const char *cfgname)
{
    int i; size_t prefix; const char *p; char path[512],magic[4]; FILE *f;
    prefix=0;
    for(p=cfgname;*p;p++) if(*p=='/' || *p=='\\' || *p==':') prefix=(size_t)(p-cfgname)+1;
    for(i=0;i<F1_TEAM_COUNT;i++) {
        if(!logo_files[i][0] || !f1_team_logo_enabled[i]) continue;
        f1_team_logo_enabled[i]=0;
        if(prefix+strlen(logo_files[i])>=sizeof(path)) continue;
        memcpy(path,cfgname,prefix); strcpy(path+prefix,logo_files[i]);
#ifdef __WATCOMC__
        { char *q; for(q=path;*q;q++) if(*q=='/') *q='\\'; }
#endif
        f=fopen(path,"rb"); if(!f) continue;
        if(fread(magic,1,4,f)==4 && !memcmp(magic,"F1L1",4) &&
           fread(f1_custom_logos[i],1,768,f)==768 && fgetc(f)==EOF) {
            f1_custom_logo_loaded[i]=1; f1_team_logo_enabled[i]=1;
        }
        fclose(f);
    }
}
int F1ConfigLoad(const char *cfgname)
{
    FILE *f; char line[256],*p,*eq,*key,*value,section[32]; int enabled=0,dirty=0,slot;
    defaults(); configured=1; if(!cfgname || !(f=fopen(cfgname,"rt"))) return 0; section[0]=0;
    while(fgets(line,sizeof(line),f)) {
        p=trim(line); if(!*p || *p==';') continue;
        if(*p=='[') { char *q=strchr(p,']'); if(q) { *q=0; copytext(section,p+1,sizeof(section)); } continue; }
        eq=strchr(p,'='); if(!eq) continue; *eq=0; key=trim(p); value=trim(eq+1);
        if(same(section,"F1 Race Tower")) {
            char *end;long v=strtol(value,&end,10);end=trim(end);
            if(end==value || (*end && *end!=';')) {fclose(f);defaults();return -1;}
            if(same(key,"GapStartProgress") && v>=0 && v<=100) { /* Legacy key: individual first-lap completion now applies. */ }
            else if(same(key,"GapUpdateTime")) { /* Legacy setting ignored: publication now follows track points. */ }
            else if(same(key,"PositionChangeTime") && v>=100 && v<=5000)f1_position_time=v;
            else {fclose(f);defaults();return -1;}
            continue;
        }
        if(same(section,"F1 Advanced")) {
            char *end;long v=strtol(value,&end,10);end=trim(end);
            if((*end && *end!=';') || end==value) {fclose(f);defaults();return -1;}
            if(same(key,"MicrosectorsEnabled")) f1_micro_enabled=v!=0;
            continue;
        }
        if(same(section,"F1 Controls")) {
            char raw[128],*end,*q;long v;
            if(same(key,"AllowExternalPitCamera")) f1_allow_external_pit_camera=atoi(value)!=0;
            else if(same(key,"HideCameraCaption")) f1_hide_camera_caption=atoi(value)!=0;
            else if(same(key,"HideRidingCaption")) f1_hide_riding_caption=atoi(value)!=0;
            else if(same(key,"HideViewingCaption")) f1_hide_viewing_caption=atoi(value)!=0;
            else if(same(key,"HideRaceWinnerCaption")) f1_hide_winner_caption=atoi(value)!=0;
            else if(same(key,"HidePitCaption")) f1_hide_pit_caption=atoi(value)!=0;
            else if(same(key,"HidePauseCaption")) f1_hide_pause_caption=atoi(value)!=0;
            else if(same(key,"HideRetirementCaption")) f1_hide_retirement_caption=atoi(value)!=0;
            else if(same(key,"FuelDrainEnabled")) f1_fuel_enabled=atoi(value)!=0;
            else if(same(key,"HudTheme")) f1_theme=atoi(value)==F1_THEME_90S?F1_THEME_90S:F1_THEME_MODERN;
            else if(same(key,"FuelDrainMilliseconds")) {
                v=strtol(value,&end,10);end=trim(end);
                if((*end && *end!=';') || v<100 || v>30000) { fclose(f);defaults();return -1; }
                f1_fuel_duration=(unsigned long)v;
            } else if(same(key,"FuelDrainCarIds")) {
                stringvalue(raw,value,sizeof(raw));q=raw;
                memset(f1_fuel_targets,0,sizeof(f1_fuel_targets));
                while(*q) {
                    v=strtol(q,&end,10);
                    if(end==q || v<1 || v>40) { fclose(f);defaults();return -1; }
                    f1_fuel_targets[v]=1;q=trim(end);
                    if(!*q) break;
                    if(*q!=',' || !*trim(q+1)) { fclose(f);defaults();return -1; }
                    q=trim(q+1);
                }
            }
            continue;
        }
        if(same(section,"F1 HUD") && same(key,"F1UseCustomData")) { enabled=atoi(value)!=0; continue; }
        if(!enabled) continue;
        if(same(section,"F1 Teams") && sscanf(key,"Team%d",&slot)==1 && slot>=1 && slot<=F1_TEAM_COUNT) {
            char *field=key+4; while(isdigit((unsigned char)*field)) field++; slot--;
            if(same(field,"Name")) { stringvalue(f1_team_names[slot],value,F1_TEAM_NAME_LEN); dirty=1; }
            else if(same(field,"Color")) { if(!hexcolor(value,f1_colors[slot])) { fclose(f); defaults(); return -1; } dirty=1; }
            else if(same(field,"Mark")) { stringvalue(f1_team_marks[slot],value,F1_TEAM_MARK_LEN); dirty=1; }
            else if(same(field,"Logo")) { f1_team_logo_enabled[slot]=(unsigned char)(atoi(value)!=0); dirty=1; }
            else if(same(field,"LogoFile")) { stringvalue(logo_files[slot],value,sizeof(logo_files[slot])); dirty=1; }
        } else if(same(section,"F1 Drivers") && sscanf(key,"Driver%d",&slot)==1 && slot>=1 && slot<=F1_DRIVER_COUNT) {
            char *field=key+6; while(isdigit((unsigned char)*field)) field++; slot--;
            if(same(field,"CarId")) { f1_drivers[slot].id=atoi(value); dirty=1; }
            else if(same(field,"First")) { stringvalue(f1_drivers[slot].first,value,F1_NAME_LEN); dirty=1; }
            else if(same(field,"Last")) { stringvalue(f1_drivers[slot].last,value,F1_NAME_LEN); dirty=1; }
            else if(same(field,"Abbr")) { stringvalue(f1_drivers[slot].abbr,value,F1_ABBR_LEN); dirty=1; }
            else if(same(field,"Number")) { f1_drivers[slot].number=atoi(value); dirty=1; }
            else if(same(field,"Team")) { f1_drivers[slot].team=atoi(value)-1; dirty=1; }
        }
    }
    fclose(f); if(!enabled) return 0;
    if(!dirty) return 1;
    if(!valid()) { defaults(); return -1; }
    loadlogos(cfgname);
    if(!f1_custom_logo_loaded[13]) f1_team_logo_enabled[13]=0;
    return 1;
}
