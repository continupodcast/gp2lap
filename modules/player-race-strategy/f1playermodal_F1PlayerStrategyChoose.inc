
 unsigned char *saved,*screen,pal[768],savedPalette[768];unsigned int savedMask;
 static unsigned long menuSerial;unsigned long menuRandom;
 F1StrategyOption options[10],original;int order[10];
 int i,j,total,sel=0,dirty=1,finish=0,pending=0,key,result=0;char part[20];KeyEvent event;
 if(!playerModalPit)return F1PlayerEditStrategy(plans,n,id,laps,choice);
 if(!pUseSVGA || !*pUseSVGA || n<1 || n>10)return 0;
 F1PlayerPaletteBuild(pal);
 saved=(unsigned char *)malloc(307200);screen=(unsigned char *)malloc(307200);
 if(!saved || !screen){free(saved);free(screen);return 0;}
 if(!F1StrategyScreen(saved,0)){free(saved);free(screen);return 0;}
 menuRandom=(unsigned long)time(NULL) ^ (++menuSerial*2654435761UL) ^ ((unsigned long)id*97UL);
 F1PlayerShuffle(order,n,&menuRandom);
 for(i=0;i<n;i++){
  const AutoPlan *p=&plans[order[i]];options[i].sequence[0]=0;options[i].pits[0]=0;total=0;
  for(j=0;j<p->count;j++){sprintf(part,"%s%c%u",j?" / ":"","HMS"[p->compound[j]],(unsigned)p->length[j]);strcat(options[i].sequence,part);}
  for(j=0;j<p->count-1;j++){total+=p->length[j];sprintf(part,"%s%d",j?" / ":"",total);strcat(options[i].pits,part);}
  if(p->count==1)strcpy(options[i].pits,"NONE");
  options[i].count=p->count;memcpy(options[i].compound,p->compound,4);memcpy(options[i].length,p->length,4);
  options[i].stops=p->count-1;options[i].fuel=(int)((p->length[0]+1)*1.0395305+0.999999);
 }
 memset(&original,0,sizeof(original));
 if(!playerModalPit && pNumCars && pCarStructs && pCarSetups){
  for(i=0;i<*pNumCars;i++)if((pCarStructs[i].id&63)==id){
   GP2Car *car=&pCarStructs[i];int stops=car->numPitStops,previous=0,pit[3],lap,valid=1,c;
   pit[0]=car->pitStop1;pit[1]=car->pitStop2;pit[2]=car->pitStop3;
   c=pCarSetups[id-1].compound;
   if(stops>3 || c>2)break;
   original.stops=stops;original.fuel=(car->fuelLoadLaps+32)/64;
   for(j=0;j<=stops;j++){
    lap=j<stops?pit[j]:laps;
    if(lap<=previous || lap>laps){valid=0;break;}
    original.compound[j]=(unsigned char)c;original.length[j]=(unsigned char)(lap-previous);
    sprintf(part,"%s%c%d",j?" / ":"","HMS"[c],lap-previous);strcat(original.sequence,part);
    if(j<stops){sprintf(part,"%s%d",j?" / ":"",lap);strcat(original.pits,part);}
    previous=lap;
   }
   if(valid){original.count=stops+1;if(!stops)strcpy(original.pits,"NONE");}
   break;
  }
 }
 /* Discard the OK that opened the race, and require a fresh confirmation. */
 while(KeyEventDequeue(&event));
 savedMask=F1PlayerPaletteCapture(savedPalette);F1StrategyMenuActive=1;
 while(!finish){
  if(dirty){
   if(playerModalPit)F1RenderPlayerPitStrategy(screen,pal,options,n,sel,id,playerModalRecommended,playerModalRequired);
   else F1RenderPlayerStrategy(screen,pal,options,n,sel,id,laps,&original);
   if(!F1StrategyScreen(screen,1))break;
   F1PlayerPalettePresent(pal,255);dirty=0;
  }
  if(KeyEventDequeue(&event)){
   key=event.scancode;
   if(pending){if(key==(pending|0x80)){finish=1;result=1;}continue;}
   if(key==0x48 || key==0xc8){if(key==0x48){sel=(sel+n)%(n+1);dirty=1;}}
   else if(key==0x50){sel=(sel+1)%(n+1);dirty=1;}
   else if(key==0x1c){pending=0x1c;*choice=sel?order[sel-1]:-1;}
   else if(key==1){pending=1;*choice=-1;}
  }else delay(10);
 }
 F1StrategyMenuActive=0;
 if(!F1StrategyScreen(saved,1))result=0;
 F1PlayerPalettePresent(savedPalette,savedMask);
 free(saved);free(screen);return result;

