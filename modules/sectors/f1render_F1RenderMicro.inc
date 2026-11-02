
    int s,i,x,y,col,step=cockpit?9:6,width=cockpit?86:62;
    int left=cockpit?F1_CARD_COMPACT_X:F1_CARD_X;
    int top=(cockpit?F1_CARD_Y_COCKPIT:F1_CARD_Y_TV)+(cockpit?43:77);
    palette(pal);
    for(s=0;s<3;s++) {
        for(i=0;i<width;i++) for(y=top;y<top+3;y++) {
            col=i<step*10 && i%step<step-1?sectorcol[colors[s*10+i/step]<=3?colors[s*10+i/step]:0]:badgeBlack;
            x=left+8+s*(cockpit?90:66)+i;dst[y*640+x]=(unsigned char)col;
        }
    }

