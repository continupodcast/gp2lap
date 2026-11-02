
    unsigned char *site;
    FILE *f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
    if(f){fputs("GP2Lap v0.75 Cameras - rear follow smoothing, Ctrl+L toggle / Ctrl+R restore; 3/4 height, 6/7 zoom, 5 reset.\n",f);fclose(f);}
    if(memcmp(IDAtoFlat(0x38635),"\x8b\x2c\xbd",3) ||
       memcmp(IDAtoFlat(0x386ab),"\x8b\x2c\xbd",3) ||
       memcmp(IDAtoFlat(0x3861e),"\x80\x3d",2) ||
       *IDAtoFlat(0x38589)!=0xa1){CamLog("SIGNATURE_FAILED",0,0);return;}
    camZoom=(long *)IDACodeReftoDataRef(0x38638);
    camHeight=(long *)IDACodeReftoDataRef(0x386ae);
    camMode=IDACodeReftoDataRef(0x38620);
    camIndex=(unsigned long *)IDACodeReftoDataRef(0x3858a);
    site=IDAtoFlat(0x385b4);
    if(site[0]!=0xe8 || site+5+*(long *)(site+1)!=IDAtoFlat(0x384a1) ||
       memcmp(IDAtoFlat(0x385a9),"\x81\xc3",2)) {CamLog("HOOK_SIGNATURE_FAILED",0,0);return;}
    camTable=IDACodeReftoDataRef(0x385ab);
    camViewed=(GP2Car **)IDACodeReftoDataRef(0x3855d);
    camTestExpected=(unsigned long)(camTable+144);
    camFinalX=(long *)IDACodeReftoDataRef(0x38530);
    camFinalY=(long *)IDACodeReftoDataRef(0x38540);
    camFinalZ=(long *)IDACodeReftoDataRef(0x38550);
    camReadOriginal=(void (*)(void))IDAtoFlat(0x384a1);
    *(long *)(site+1)=(unsigned char *)F1CamReadWrapped-(site+5);
    camReady=1;CamLog("READY",0,0);

