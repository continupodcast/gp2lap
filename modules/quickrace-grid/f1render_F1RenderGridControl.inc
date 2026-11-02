
    int x,y,w,h,base,edge,fill,accent,thumb,offset;long range;
    if(!dst || !pal || !rect)return;
    x=rect[0];y=rect[1];w=rect[2]-x+1;h=rect[3]-y+1;
    if(x<0 || y<0 || w<1 || h<1 || x+w>640 || y+h>480)return;
    palette(pal);base=nearest(pal,42,45,55);edge=nearest(pal,190,190,190);accent=nearest(pal,230,28,45);
    if(f1_theme==F1_THEME_90S){palette90(pal);base=c90navy;edge=c90gold;accent=c90yellow;}
    fill=focused?accent:base;clipRight=x+w;
    if(kind<3 && f1_theme==F1_THEME_90S){
        bar(dst,x,y,w,h,fill);bar(dst,x,y,w,1,edge);bar(dst,x,y+h-1,w,1,edge);
        tc90(dst,x+2,y,w-4,h,kind==0?"PRINT SETUP":kind==1?"PRINT":"OK",F90_HEAD,focused?c90ink:-1,NULL);
        clipRight=640;return;
    }
    if(kind<3)F1GridButton(dst,x,y,w,h,kind==0?"PRINT SETUP":kind==1?"PRINT":"OK",fill,edge,-1);
    else if(kind<7) {
        F1GridButton(dst,x,y,w,h,"",fill,edge,-1);
        F1GridArrow(dst,x+(w-15)/2,y+(h-(kind==3 || kind==6?11:5))/2,kind<5,kind==3 || kind==6,edge);
    } else {
        /* Native rail has a narrow hit rectangle inside a wider painted track. */
        if(x==479 && w==10){x=475;y=150;w=20;h=240;clipRight=x+w;}
        bar(dst,x,y,w,h,base);range=extent-320;if(range<1)range=1;
        if(scroll<0)scroll=0;
        if(scroll>range)scroll=range;
        thumb=extent>0?(int)((long)h*320/extent):h;if(thumb<8)thumb=8;if(thumb>h)thumb=h;
        offset=(int)(scroll*(h-thumb)/range);bar(dst,x+1,y+offset,w>2?w-2:w,thumb,accent);
    }
    clipRight=640;

