
        label[0]=r->compound>=0 && r->compound<4?"HMSS"[r->compound]:'?';label[1]=0;
        clipRight=F1_PANEL_X+F1_Q_TOTAL_W;
        text(dst,F1_PANEL_X+F1_Q_W+4,y+1,label,1,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,0);
        text(dst,F1_PANEL_X+F1_Q_W+5,y+1,label,1,r->compound>=0 && r->compound<4?tyrecol[r->compound]:-1,0);
        