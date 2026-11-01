/* Native grid menu in the established Futura / gold / navy 90s theme. */
static void F1RenderGridMenu90(unsigned char *dst,const unsigned char *pal,
    const unsigned char *ids,const unsigned long *times,int count,long scroll,
    long extent,const char *country,const char *circuit,int laps)
{
    int i,j,start,n,y,di,team;long range,pole,t;char label[128],name[F1_NAME_LEN];
    palette90(pal);clipRight=494;
    /* Whole native grid body; input objects and cursor are preserved. */
    bar(dst,140,58,356,379,c90navy);
    sprintf(label,"GRAND PRIX OF %.40s",country?country:"UNKNOWN");
    up90(label,label,sizeof(label));
    tc90(dst,148,62,319,22,label,F90_NAME,-1,NULL);
    if(laps>0)sprintf(label,"%.40s (%d Laps)",circuit?circuit:"Unknown",laps);
    else sprintf(label,"%.40s",circuit?circuit:"Unknown");
    tc90(dst,148,85,319,18,label,F90_HEAD,c90cyan,NULL);
    bar(dst,148,105,319,2,c90yellow);
    t90(dst,154,110,"STARTING GRID",F90_HEAD,c90yellow,NULL,0,280);
    t90(dst,153,128,"POS",F90_SMALL,-1,NULL,0,22);
    t90(dst,181,128,"DRIVER",F90_SMALL,-1,NULL,0,120);
    t90(dst,315,128,"TEAM",F90_SMALL,-1,NULL,0,80);
    tr90(dst,465,128,"TIME",F90_SMALL,c90yellow,0);
    range=extent-320;if(range<1)range=1;
    if(scroll<0)scroll=0;
    if(scroll>range)scroll=range;
    start=count>10?(int)(scroll*(count-10)/range):0;
    n=count-start;if(n>10)n=10;
    di=ids[0]&63;pole=di>=1 && di<=40?F1CombinedValidGrid(times[di-1]):0;
    for(i=0;i<n;i++) {
        j=start+i;y=147+i*27;di=driver(ids[j]&63);
        team=di<0?-1:f1_drivers[di].team;
        if(i&1)bar(dst,148,y-2,319,26,c90ink);
        bar(dst,150,y,24,22,c90yellow);sprintf(label,"%d",j+1);
        tc90(dst,150,y,24,22,label,F90_NAME,-1,r90box);
        clipRight=307;
        if(di>=0) {
            t90(dst,181,y,f1_drivers[di].first,F90_SMALL,-1,NULL,0,126);
            up90(name,f1_drivers[di].last,sizeof(name));
            t90(dst,181,y+10,name,F90_NAME,-1,NULL,0,126);
        } else t90(dst,181,y+5,"UNKNOWN",F90_NAME,-1,NULL,0,126);
        clipRight=398;
        t90(dst,315,vc90(y,22,F90_HEAD),team>=0?f1_team_names[team]:"UNKNOWN",F90_HEAD,c90cyan,NULL,0,83);
        di=ids[j]&63;t=di>=1 && di<=40?F1CombinedValidGrid(times[di-1]):0;
        if(!t)strcpy(label,"NO TIME");else if(j==0 || !pole)F1QTime(label,t,0);else F1QDelta(label,t-pole);
        clipRight=467;tr90(dst,465,vc90(y,22,F90_HEAD),label,F90_HEAD,c90yellow,0);
    }
    clipRight=640;
}
