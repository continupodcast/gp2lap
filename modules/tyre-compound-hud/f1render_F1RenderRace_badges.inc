
        label[0]=row->compound>=0 && row->compound<4?"HMSS"[row->compound]:'?';label[1]=0;
        text(dst,F1_PANEL_X+F1_PANEL_W+4,y+1,label,1,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,row->out);
        text(dst,F1_PANEL_X+F1_PANEL_W+5,y+1,label,1,row->compound>=0 && row->compound<4?tyrecol[row->compound]:-1,row->out);
        