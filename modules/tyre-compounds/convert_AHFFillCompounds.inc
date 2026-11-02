
  int i, s, k, nb, mp, base, ncars, mode, q, sh, c;
  int present[4], bstop[4], bcount[4], comp[4], stop2comp[4];
  GP2CSx *psetup;

  f1RaceCompoundPending=0;
  if (!PerCarTyreCompounds) return;            /* feature off -> stock untouched */
  if (!pCarSetups) return;
  psetup = (GP2CSx *)pPlayerSetup;

  /* True track base compound. b_TireType (0x1771C0) is the SAME byte as the player's  */
  /* setup[+8] (pPlayerSetup 0x1771B8 + 8), so our own fills overwrite it: it equals    */
  /* the genuine track base only on the FIRST session of a weekend, before any fill.    */
  /* DoTrackSetup writes it at track load; snapshot it once per track (dwTrackChecksum   */
  /* change) and reuse it for every session that weekend, surviving quali->race.        */
  if (dwTrackChecksum != s_baseCsum || s_trackBase < 0) {
    s_trackBase = (*((BYTE *)pCarSetups - 0x58)) & 3;
    s_baseCsum  = dwTrackChecksum;
  }
  base = s_trackBase;

  mode = GP2_RaceMode ? (int)(*GP2_RaceMode) : 0;

  if (mode == 0x80) {
    /* --- RACE: pit-strategy -> per-car compound --- */
    ncars = GP2_NumStarters ? (int)*GP2_NumStarters : 26;
    if (ncars < 1) ncars = 1;
    if (ncars > GP2_MAX_NUMCARS) ncars = GP2_MAX_NUMCARS;

    for (s = 0; s < 4; s++) present[s] = 0;
    for (i = 0; i < ncars; i++) present[pCarSetups[i].num_pitstops & 3]++;

    nb = 0;                                    /* distinct stop-counts present, ascending */
    for (s = 0; s < 4; s++) if (present[s]) { bstop[nb] = s; bcount[nb] = present[s]; nb++; }
    if (nb == 0) return;

    mp = 0;                                    /* most-populated bucket (ties -> fewest stops) */
    for (k = 1; k < nb; k++) if (bcount[k] > bcount[mp]) mp = k;

    for (k = 0; k < nb; k++) comp[k] = base + (k - mp);   /* fewer stops harder, anchored at base */
    if (comp[0] < 0)    { sh = -comp[0];       for (k = 0; k < nb; k++) comp[k] += sh; }  /* clamp run */
    if (comp[nb-1] > 3) { sh = comp[nb-1] - 3; for (k = 0; k < nb; k++) comp[k] -= sh; } /* into A..D */

    for (s = 0; s < 4; s++) stop2comp[s] = base;
    for (k = 0; k < nb; k++) {
      c = comp[k]; if (c < 0) c = 0; if (c > 3) c = 3;
      stop2comp[bstop[k]] = c;
    }

    if(TyreDiagnostics && TyreTestCompound>=1 && TyreTestCompound<=4)
      for(s=0;s<4;s++)stop2comp[s]=(int)TyreTestCompound-1;
    for (i = 0; i < GP2_MAX_NUMCARS; i++)
      pCarSetups[i].compound = (BYTE)stop2comp[pCarSetups[i].num_pitstops & 3];
    for(s=0;s<4;s++)f1RaceCompoundMap[s]=stop2comp[s];
    f1RaceCompoundPending=1;
    /* Leave player setup alone until the instantiated car is available. */
  }
  else if ((mode & 0x40) || mode == 0x01 || mode == 0x03) {
    /* --- QUALIFYING (0x40), or Friday(0x01)/Saturday(0x03) free practice: follow   --- */
    /* --- the qualifying tyre rule (D when SoftTyresInQuali, else the stock base).   --- */
    /* --- Warm-up (0x02) and unlimited practice (0x00) fall through unchanged below. --- */
    q = SoftTyresInQuali ? 3 : base;
    if(TyreDiagnostics && TyreTestCompound>=1 && TyreTestCompound<=4)q=(int)TyreTestCompound-1;
    for (i = 0; i < GP2_MAX_NUMCARS; i++) pCarSetups[i].compound = (BYTE)q;
    if (psetup) psetup->compound = (BYTE)q;
  }
  else {
    /* --- Unlimited practice (0x00), pre-race warm-up (0x02), anything else: leave --- */
    /* --- every car on the stock single compound (track base) -> behaves like stock. -- */
    if(TyreDiagnostics && TyreTestCompound>=1 && TyreTestCompound<=4)base=(int)TyreTestCompound-1;
    for (i = 0; i < GP2_MAX_NUMCARS; i++) pCarSetups[i].compound = (BYTE)base;
    if (psetup) psetup->compound = (BYTE)base;
  }

