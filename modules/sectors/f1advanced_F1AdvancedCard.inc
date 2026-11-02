
    static const unsigned char unavailable[30]={0};
    if(!F1MicroMode) return;
    diagnostic(focus);
    if(!f1_micro_enabled) return;
    F1RenderMicro(dst,pal,ready && splitReady && focus>=1 && focus<=40?micro.cars[focus].colors:unavailable,cockpit);

