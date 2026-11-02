
 unsigned long *on;unsigned char *pc,*src,*flag;FILE *f;
 on=GetCfgULong("QGridEnabled");if(!on || !*on)return;
 f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f){fputs("GP2Lap F1 HUD 0.8: Quickrace custom grid\n",f);fclose(f);}
 qGridPreferenceCount=QGridParse(GetCfgString("QGridOrder"),qGridPreference);
 if(qGridPreferenceCount<1){LogLine("- QuickGrid: invalid QGridOrder; disabled\n");return;}
 pc=IDAtoFlat(0x6AEF6UL);src=IDAtoFlat(0x2C1A3UL);flag=IDAtoFlat(0x6AE32UL);
 /* Rel32 verified against actual runtime native target; relocated data references
    obtained from their operands, never guessed from flat data addresses. */
 if(!pc || !src || !flag || pc[0]!=0xE8 ||
    (unsigned long)(pc+5)+*(long *)(pc+1)!=(unsigned long)IDAtoFlat(0x2C19DUL) ||
    src[0]!=0xBA || flag[0]!=0xC6 || flag[1]!=0x05 || flag[6]!=0x80){
  LogLine("- QuickGrid: signatures differ; disabled\n");
  f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f){fputs("INIT signature mismatch; no patch\n",f);fclose(f);}return;
 }
 qGridSource=(unsigned char *)IDACodeReftoDataRef(0x2C1A4UL);
 qGridQuickFlag=(unsigned char *)IDACodeReftoDataRef(0x6AE34UL);
 if(!qGridSource || !qGridQuickFlag || !pCarIDs)return;
 QGridFinalize=(unsigned long)IDAtoFlat(0x2C19DUL);
 *(long *)(pc+1)=(long)((unsigned long)QuickGridHook-(unsigned long)(pc+5));
 LogLine("- QuickGrid Q1: Quickrace-only pre-placement order enabled\n");
 f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;if(f){QGridDump(f,"INIT preference=",qGridPreference,qGridPreferenceCount);fclose(f);}

