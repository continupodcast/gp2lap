
    static const unsigned long checks[]={0x6e6a1,0x6e83c,0x6e8b4,0x6e971,0x6e9e9};
    static const unsigned long textCalls[]={0x3487f,0x34ab1,0x6e65b,0x6e665,0x6e7c3,0x6e7ed,0x6e805,0x6e826,0x6e830};
    static const unsigned long pixelCalls[]={0x71358,0x6d7a8};
    unsigned int i;unsigned char *p,*pixels=IDAtoFlat(0x7120c),*visibility=IDAtoFlat(0x71343),*generic=IDAtoFlat(0x6e1c2);
    unsigned char *pause=IDACodeReftoDataRef(0x3487b);
    if(IDAtoFlat(0x3487a)[0]!=0xb8 || !pause || memcmp(pause,"\x13\x07\x06\x02?PAUSED",11))return 0;
    if(memcmp(visibility,"\xf6\x05",2) || memcmp(generic,"\xff\x35",2))return 0;
    for(i=0;i<sizeof(checks)/sizeof(checks[0]);i++) {
        p=IDAtoFlat(checks[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=visibility)return 0;
        if(!(p[5]==0x75 || (p[5]==0x0f && p[6]==0x85)))return 0;
    }
    for(i=0;i<sizeof(textCalls)/sizeof(textCalls[0]);i++) {
        p=IDAtoFlat(textCalls[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=generic)return 0;
    }
    if(memcmp(pixels,"\x80\x3d",2))return 0;
    for(i=0;i<2;i++){p=IDAtoFlat(pixelCalls[i]);if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=pixels)return 0;}
    captionView=IDACodeReftoDataRef(0x6e6dd);
    if(!captionView)return 0;
    pauseText=(unsigned long)pause;
    f1NativeSuppressPixels=0;fpF1NativePixelsOriginal=(void (*)(void))pixels;
    for(i=0;i<2;i++){p=IDAtoFlat(pixelCalls[i]);*(long *)(p+1)=(unsigned char *)F1NativePixelsWrapped-(p+5);}
    fpF1NativeVisibilityOriginal=(void (*)(void))visibility;
    fpF1NativeTextOriginal=(void (*)(void))generic;
    for(i=0;i<sizeof(checks)/sizeof(checks[0]);i++) {
        p=IDAtoFlat(checks[i]);captionReturns[i]=(unsigned long)(p+5);
        *(long *)(p+1)=(unsigned char *)F1NativeVisibilityWrapped-(p+5);
    }
    for(i=0;i<sizeof(textCalls)/sizeof(textCalls[0]);i++) {
        p=IDAtoFlat(textCalls[i]);*(long *)(p+1)=(unsigned char *)F1NativeTextWrapped-(p+5);
    }
    return 1;

