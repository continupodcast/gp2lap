
    unsigned char *p,*target;long relative;FILE *f;
    if(!GP2_CodeStartAdr)return 0;
    p=IDAtoFlat(0x83859);target=IDAtoFlat(0x7bdff);
    if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=target || memcmp(p+5,"\x61\xc3",2))return 0;
    if(memcmp(IDAtoFlat(0x835ac),"\xa1",1) || IDAtoFlat(0x8368a)[0]!=0xbe ||
       memcmp(IDAtoFlat(0x835d9),"\x3b\x05",2) || memcmp(IDAtoFlat(0x83804),"\x81\xc6",2))return 0;
    gridProbeScroll=(long *)IDACodeReftoDataRef(0x835ad);
    gridProbeCount=(unsigned long *)IDACodeReftoDataRef(0x835db);
    gridProbeOrder=IDACodeReftoDataRef(0x8368b);
    gridProbeTimes=(unsigned long *)IDACodeReftoDataRef(0x83806);
    gridProbeCanvas=IDACodeReftoDataRef(0x7ad0b);
    if(memcmp(IDAtoFlat(0x87920),"\x80\x3d",2) || IDAtoFlat(0x8794e)[0]!=0x05 || IDAtoFlat(0x834df)[0]!=0xbe)return 0;
    gridMenuDepth=IDACodeReftoDataRef(0x87922);
    gridMenuStack=IDACodeReftoDataRef(0x8794f);
    gridMenuPage=IDACodeReftoDataRef(0x834e0);
    if(memcmp(IDAtoFlat(0x851cd),"\x8b\x1d",2) || memcmp(IDAtoFlat(0x812ac),"\x03\x1d",2) || memcmp(IDAtoFlat(0x812b2),"\x03\x05",2))return 0;
    gridSelectedObject=(unsigned char **)IDACodeReftoDataRef(0x851cf);
    gridOriginX=(long *)IDACodeReftoDataRef(0x812ae);
    gridOriginY=(long *)IDACodeReftoDataRef(0x812b4);
    if(memcmp(IDAtoFlat(0x85587),"\x0f\xb6\x0d",3))return 0;
    gridSelectedIndex=IDACodeReftoDataRef(0x8558a);
    if(IDAtoFlat(0x7f623)[0]!=0xba)return 0;
    gridProbePalette=IDACodeReftoDataRef(0x7f624);
    if(!gridProbeScroll || !gridProbeCount || !gridProbeOrder || !gridProbeTimes || !gridProbeCanvas)return 0;
    fpF1GridProbeOriginal=(void (*)(void))target;
    relative=(unsigned char *)F1GridProbeWrapped-(p+5);memcpy(p+1,&relative,4);
    f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f){fputs("GRID MENU v0.63: modern panel after native draw; original controls retained; scroll/grid snapshots\n",f);fclose(f);}
    return 1;

