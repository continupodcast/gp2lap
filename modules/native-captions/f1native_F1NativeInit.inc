
    static const unsigned long calls[]={0x6e234,0x6e768,0x6e8a2,0x6e95f,0x6e9d7,0x6eab9};
    unsigned int i;unsigned char *p,*original=IDAtoFlat(0x6d792);
    /* All-or-nothing validation before any instruction is modified. */
    if(memcmp(original,"\x60\xb0\x20\xaa\xb0\x00\xaa",7)) return 0;
    for(i=0;i<sizeof(calls)/sizeof(calls[0]);i++) {
        p=IDAtoFlat(calls[i]);
        if(p[0]!=0xe8 || p+5+*(long *)(p+1)!=original) return 0;
    }
    fontTable=(unsigned char **)IDACodeReftoDataRef(0x7bc6b);
    fontState=IDACodeReftoDataRef(0x7bc2f);
    coords=IDACodeReftoDataRef(0x6e1e3);
    center=IDACodeReftoDataRef(0x7118d);
    viewOffset=IDACodeReftoDataRef(0x711a2);
    height=(unsigned long *)IDACodeReftoDataRef(0x7d7d9);
    if(!fontTable || !fontState || !coords || !center || !viewOffset || !height) return 0;
    fpF1NativeOriginal=(void (*)(void))original;
    for(i=0;i<sizeof(calls)/sizeof(calls[0]);i++) {
        p=IDAtoFlat(calls[i]);*(long *)(p+1)=(unsigned char *)F1NativeCaptionWrapped-(p+5);
    }
    return 1;

