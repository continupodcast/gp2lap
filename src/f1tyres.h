#ifndef F1TYRES_H
#define F1TYRES_H
/* Read-only assigned compound. Caller keeps GP2's raw ID, including player bit. */
static int F1AssignedCompound(unsigned int rawId,const GP2CSx *setups,const GP2CSx *player)
{
    unsigned int id=rawId&63;const GP2CSx *setup;
    if(id<1 || id>40)return -1;
    setup=(rawId&128)?player:(setups?setups+id-1:0);
    if(!setup || setup->compound>3)return -1;
    return (int)setup->compound;
}
#endif
