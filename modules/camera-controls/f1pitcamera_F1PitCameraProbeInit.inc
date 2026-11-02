
    static const unsigned long sites[]={0x36d84,0x36e7c,0x36ea5,0x36ec5,0x36f8d};
    static const unsigned long calls[]={0x36cf3,0x36d20,0x36d7b,0x36da7};
    static const unsigned long checks[]={0x36d8a,0x36e72,0x36e9b,0x36ebf,0x36f87};
    static const unsigned int lengths[]={6,10,10,6,6};
    static const unsigned char expected[5][10]={
        {0xf6,0x46,0x23,0x20,0x74,0x24},
        {0xf6,0x46,0x23,0x20,0x0f,0x85,0xc1,0,0,0},
        {0xf6,0x46,0x23,0x20,0x0f,0x85,0x98,0,0,0},
        {0xf6,0x46,0x23,0x20,0x75,0x78},
        {0xf6,0x46,0x23,0x20,0x75,0xf3}};
    void (__cdecl *wrappers[5])(void)={F1PitCamAutoCheck,F1PitCamTVCheck,F1PitCamRearCheck,F1PitCamOtherCheck,F1PitCamApplyCheck};
    unsigned int i;unsigned char *p,*original=IDAtoFlat(0x36f3f);FILE *f;
    pitCamReady=0;pitCamRecords=0;pitCamPrevious[0]=0;
    f=F1_PIT_CAMERA_TRACE?((FILE *)0) /* obsolete diagnostic disabled in T4 */:NULL;if(f){fputs("PIT CAMERA P4: conditional bit20 camera checks; pit=flagsAD:04 for player AND AI, no retirement; max 5000 records\n",f);fclose(f);}
    if(original[0]!=0x60)return 0;
    for(i=0;i<4;i++){p=IDAtoFlat(calls[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=original)return 0;}
    if(IDAtoFlat(0x36f40)[0]!=0xa0 || IDAtoFlat(0x36f61)[0]!=0xf6 || IDAtoFlat(0x380fb)[0]!=0xa1 || IDAtoFlat(0x36c3d)[0]!=0xf6 || IDAtoFlat(0x36c0e)[0]!=0xf6)return 0;
    pitCamView=IDACodeReftoDataRef(0x36f41);pitCamRequests=IDACodeReftoDataRef(0x36f63);
    pitCamMode=(unsigned long *)IDACodeReftoDataRef(0x380fc);
    pitCamKeys1=IDACodeReftoDataRef(0x36c3f);pitCamKeys2=IDACodeReftoDataRef(0x36c10);
    if(!pitCamView || !pitCamRequests || !pitCamMode || !pitCamKeys1 || !pitCamKeys2)return 0;
    /* Validate every original TEST before patching any of them. */
    for(i=0;i<sizeof(sites)/sizeof(sites[0]);i++) {
        p=IDAtoFlat(sites[i]);
        if(memcmp(p,"\xf6\x46\x23\x40",4))return 0;
    }
    for(i=0;i<5;i++)if(memcmp(IDAtoFlat(checks[i]),expected[i],lengths[i]))return 0;
    f1PitCamAutoTarget=IDAtoFlat(0x36db4);
    f1PitCamRejectTarget=IDAtoFlat(0x36f3d);
    f1PitCamApplyTarget=IDAtoFlat(0x36f80);
    for(i=0;i<5;i++) {
        p=IDAtoFlat(checks[i]);memset(p,0x90,lengths[i]);p[0]=0xe8;
        *(long *)(p+1)=(unsigned char *)wrappers[i]-(p+5);
    }
    for(i=0;i<sizeof(sites)/sizeof(sites[0]);i++) {
        p=IDAtoFlat(sites[i]);p[3]=0; /* TEST zero: pit-only camera branch not taken. */
    }
    fpF1PitCameraChangeOriginal=(void (*)(void))original;
    for(i=0;i<4;i++){p=IDAtoFlat(calls[i]);*(long *)(p+1)=(unsigned char *)F1PitCameraChangeWrapped-(p+5);}
    pitCamReady=1;PitCamTrace("INIT",0,1);
    return 1;

