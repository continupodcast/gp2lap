
        label[0]=row->compound>=0 && row->compound<4?"HMSS"[row->compound]:'?';label[1]=0;
        t90(dst,x+T90_W+4,vc90(y,T90_BOXH,F90_SMALL),label,F90_SMALL,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,NULL,dim,8);
        t90(dst,x+T90_W+5,vc90(y,T90_BOXH,F90_SMALL),label,F90_SMALL,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,NULL,dim,8);
