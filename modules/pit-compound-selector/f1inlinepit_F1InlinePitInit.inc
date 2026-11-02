
 unsigned char *call,*original,*p,*controlCall,*cursor1,*cursor2,*cursorOriginal,*navigation;
 if(inlinePitReady)return 1;
 if(!StrategyPlayerEnabled || !StrategyPlayerPitChoice || !PerCarTyreCompounds)return 0;
 navigation=IDAtoFlat(0x35581);
 if(!navigation || navigation[0]!=0xe8 || navigation+5+*(long *)(navigation+1)!=IDAtoFlat(0x34d14) ||
  memcmp(IDAtoFlat(0x34d14),"\x60\xf7\x05",3))return 0;
 cursor1=IDAtoFlat(0x34e2a);cursor2=IDAtoFlat(0x34e9e);cursorOriginal=IDAtoFlat(0x34c43);
 if(!cursor1 || !cursor2 || !cursorOriginal || cursor1[0]!=0xe8 || cursor2[0]!=0xe8 ||
  cursor1+5+*(long *)(cursor1+1)!=cursorOriginal || cursor2+5+*(long *)(cursor2+1)!=cursorOriginal)return 0;
 controlCall=IDAtoFlat(0x3564e);
 if(!controlCall || controlCall[0]!=0xe8 || controlCall+5+*(long *)(controlCall+1)!=IDAtoFlat(0x353cf) ||
  memcmp(IDAtoFlat(0x34bb4),"\x8b\x1c\x9d",3) || memcmp(IDAtoFlat(0x34eaa),"\xa1",1) ||
  memcmp(IDAtoFlat(0x34e52),"\x89\x15",2) || memcmp(IDAtoFlat(0x353d4),"\x80\x3d",2))return 0;
 call=IDAtoFlat(0x6cee5);original=IDAtoFlat(0x6d1b3);p=IDAtoFlat(0x7213b);
 if(!call || !original || !p || call[0]!=0xe8 || call+5+*(long *)(call+1)!=original ||
  memcmp(original,"\xc6\x05",2) || memcmp(p,"\x80\x3d",2))return 0;
 /* Validate every opcode supplying shared pointers before installing a hook. */
 if(memcmp(IDAtoFlat(0x6cedb),"\xa1",1) || memcmp(IDAtoFlat(0x6cecb),"\xf7\x05",2) ||
  memcmp(IDAtoFlat(0x7247e),"\x8b\x34\x95",3) || memcmp(IDAtoFlat(0x72485),"\x8b\x2c\x95",3) ||
  memcmp(IDAtoFlat(0x72494),"\x03\x35",2) || memcmp(IDAtoFlat(0x724ef),"\xf7\x25",2) ||
  memcmp(IDAtoFlat(0x72470),"\x0f\xb6\x80",3) || memcmp(IDAtoFlat(0x354f9),"\xc6\x05",2))return 0;
 inlinePitTextMode=IDACodeReftoDataRef(0x7213d);
 inlinePitMenu=(unsigned long *)IDACodeReftoDataRef(0x6cedc);
 inlinePitRequests=(unsigned long *)IDACodeReftoDataRef(0x6cecd);
 inlinePitCoords=IDACodeReftoDataRef(0x6d1b5);
 inlinePitFonts=(unsigned long *)IDACodeReftoDataRef(0x72481);
 inlinePitHeights=(unsigned long *)IDACodeReftoDataRef(0x72488);
 inlinePitImageBase=(unsigned char **)IDACodeReftoDataRef(0x72496);
 inlinePitStride=(unsigned long *)IDACodeReftoDataRef(0x724f1);
 inlinePitMap=IDACodeReftoDataRef(0x72473);
 inlinePitDirty=IDACodeReftoDataRef(0x354fb);
 if(!inlinePitTextMode || !inlinePitMenu || !inlinePitRequests || !inlinePitCoords || !inlinePitFonts || !inlinePitHeights ||
  !inlinePitImageBase || !inlinePitStride || !inlinePitMap || !inlinePitDirty)return 0;
 if(IDAtoFlat(0x353f9)[0]!=0xb8 || IDAtoFlat(0x354e8)[0]!=0xb8)return 0;
 inlinePitPromptRef=(unsigned long *)IDAtoFlat(0x353fa);inlinePitClearRef=(unsigned long *)IDAtoFlat(0x354e9);
 inlinePitOriginalPrompt=*inlinePitPromptRef;inlinePitOriginalClear=*inlinePitClearRef;
 inlinePitDescriptors=(unsigned char **)IDACodeReftoDataRef(0x34bb7);
 inlinePitRow=(unsigned long *)IDACodeReftoDataRef(0x34eab);
 inlinePitColumn=(unsigned long *)IDACodeReftoDataRef(0x34e54);
 inlinePitAction=IDACodeReftoDataRef(0x353d6);
 if(!inlinePitDescriptors || !inlinePitRow || !inlinePitColumn || !inlinePitAction)return 0;
 inlinePitOriginalDescriptor=inlinePitDescriptors[6];
 if(!inlinePitOriginalDescriptor || inlinePitOriginalDescriptor[0]!=1 || inlinePitOriginalDescriptor[1]!=2)return 0;
 fpF1InlinePitNavigationOriginal=(void (__cdecl *)(void))IDAtoFlat(0x34d14);
 *(long *)(navigation+1)=(unsigned char *)F1InlinePitNavigationWrapped-(navigation+5);
 fpF1InlinePitCursorOriginal=(void (__cdecl *)(void))cursorOriginal;
 *(long *)(cursor1+1)=(unsigned char *)F1InlinePitCursorWrapped-(cursor1+5);
 *(long *)(cursor2+1)=(unsigned char *)F1InlinePitCursorWrapped-(cursor2+5);
 fpF1InlinePitControlOriginal=(void (__cdecl *)(void))IDAtoFlat(0x353cf);
 fpF1InlinePitPollOriginal=(void (__cdecl *)(void))IDAtoFlat(0x34e70);
 *(long *)(controlCall+1)=(unsigned char *)F1InlinePitControlWrapped-(controlCall+5);
 fpF1InlinePitOriginal=(void (__cdecl *)(void))original;inlinePitChar=(void (*)(unsigned int))p;
 *(long *)(call+1)=(unsigned char *)F1InlinePitWrapped-(call+5);inlinePitReady=1;return 1;

