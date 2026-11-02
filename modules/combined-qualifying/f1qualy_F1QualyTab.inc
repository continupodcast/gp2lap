
    if(scan==15 && active && pSessionMode && (*pSessionMode==0x40 || *pSessionMode==0x44)) {
        FILE *f;int n,i;const F1QRow *rows;
        F1QCombinedMode=!F1QCombinedMode;
        f=((FILE *)0) /* obsolete diagnostic disabled in T4 */;
        if(f) {
            fprintf(f,"v0.75 TAB mode=%02X combined=%d clock=%lu fri_ptr=%d sat_ptr=%d\n",(unsigned)*pSessionMode,F1QCombinedMode,pCurTime?*pCurTime:0,GP2_QualTimesSes1!=NULL,GP2_QualTimesSes2!=NULL);
            rows=F1QRows(&n);
            for(i=0;i<n;i++)if(rows[i].id>=1 && rows[i].id<=40)fprintf(f,"CarId=%d current=%ld friday_raw=%lu saturday_raw=%lu\n",rows[i].id,rows[i].best,GP2_QualTimesSes1?GP2_QualTimesSes1->car[rows[i].id-1]:0,GP2_QualTimesSes2?GP2_QualTimesSes2->car[rows[i].id-1]:0);
            fclose(f);
        }
    }

