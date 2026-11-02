
        label[0]=r->compound>=0 && r->compound<4?"HMSS"[r->compound]:'?';label[1]=0;
        t90(dst,x+T90_QW+4,vc90(y,T90_BOXH,F90_SMALL),label,F90_SMALL,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,NULL,0,8);
        t90(dst,x+T90_QW+5,vc90(y,T90_BOXH,F90_SMALL),label,F90_SMALL,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,NULL,0,8);
