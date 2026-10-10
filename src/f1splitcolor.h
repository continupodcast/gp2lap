#ifndef F1SPLITCOLOR_H
#define F1SPLITCOLOR_H
/* Compare with this driver's reference before the measurement and the
   session minimum. Equal session records share purple until beaten. */
static int F1SplitColor(long value,long personal,long overall)
{
    if(value<=0) return 0;
    if(!overall || value<=overall) return 3;
    if(!personal || value<personal) return 1;
    return 2;
}
#endif
