
 int c;GP2CSx *player;
 if(!f1RaceCompoundPending || !PerCarTyreCompounds)return;
 if(!GP2_RaceMode || *GP2_RaceMode!=0x80){f1RaceCompoundPending=0;return;}
 player=(GP2CSx *)pPlayerSetup;if(!player)return;
 c=F1EffectivePlayerCompound(f1RaceCompoundMap);
 if(c<0)return; /* Retry next update; no fabricated fallback strategy. */
 player->compound=(BYTE)c;
 f1RaceCompoundPending=0;

