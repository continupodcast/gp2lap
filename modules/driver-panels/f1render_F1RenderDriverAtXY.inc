
    int di,team,xx,yy,k,lx,j,best,a,dx,dy;
    char label[20];const F1Driver *d;
    if(!dst || !pal || id<1 || id>40 || y<0 || y>480-F1_DRIVER_H || x<0 || x>640-F1_DRIVER_W) return;
    di=driver(id);if(di<0) return;d=&f1_drivers[di];team=d->team;
    if(f1_theme==F1_THEME_90S) { F1RenderDriver90(dst,pal,id,position,x,y); return; }
    palette(pal);clipRight=x+F1_DRIVER_W;
    background(dst,x,y,F1_DRIVER_W,F1_DRIVER_H);
    for(yy=0;yy<F1_DRIVER_H;yy++) for(xx=0;xx<F1_DRIVER_W;xx++) {
        k=xx+yy/2;
        if(xx>=145) dst[(y+yy)*640+x+xx]=platecol[team][2];
        else if((k>=96 && k<108) || (k>=117 && k<126))
            dst[(y+yy)*640+x+xx]=platecol[team][1];
    }
    if(position>0) sprintf(label,"%d",position);else strcpy(label,"-");
    centeredNumber(dst,x+2,y,30,38,label,2,-1);
    clipRight=x+143;
    text(dst,x+38,y+2,d->first,5,-1,0);
    driverSurname(dst,x+38,y+10,d->last,teamcol[team]);
    text(dst,x+38,y+26,f1_team_names[team],5,-1,0);
    lx=x+38+textwidth(f1_team_names[team],5)+3;
    if(lx+8<=x+143 && !f1_team_logo_enabled[team]) for(yy=0;yy<6;yy++) for(xx=0;xx<8;xx++) dst[(y+27+yy)*640+lx+xx]=teamcol[team];
    else if(lx+8<=x+143) for(yy=0;yy<6;yy++) for(xx=0;xx<8;xx++) {
        best=-1;a=0;
        for(dy=0;dy<2;dy++) for(dx=0;dx<2;dx++) {
            j=(yy*2+dy)*16+xx*2+dx;
            if(TEAM_LOGO(team)[j][3]>a) {a=TEAM_LOGO(team)[j][3];best=j;}
        }
        if(a>=128) dst[(y+27+yy)*640+lx+xx]=logocol[team][best];
    }
    sprintf(label,"%d",d->number);
    centeredNumber(dst,x+145,y,55,38,label,2,teamcol[team]);

