/* Sector cards now use the normal upper-centre scene composition.
   Keep hook entry points for the existing patch, without cockpit VRAM writes. */
void F1VideoReset(void) {}
void F1CockpitBefore(void) {}
void F1CockpitAfter(void) {}
void (*fpF1CockpitBefore)(void)=F1CockpitBefore;
void (*fpF1CockpitAfter)(void)=F1CockpitAfter;
void (*fpF1VideoReset)(void)=F1VideoReset;
