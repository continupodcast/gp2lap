/* Read-only combined qualifying table. Native session arrays are indexed by CarId. */
static long F1CombinedValid(unsigned long t) {return t && !(t&0xf0000000UL) && t<=3600000UL?(long)t:0;}
static int F1CombinedBuild(F1QRow *dst,const F1QRow *src,int n,const unsigned long *fri,const unsigned long *sat,int mode)
{
    int i,j;long a,b;F1QRow tmp;
    if(n<0 || n>F1_Q_MAX)return 0;
    for(i=0;i<n;i++) {
        dst[i]=src[i];a=b=0;
        if(src[i].id>=1 && src[i].id<=40) {
            if(fri)a=F1CombinedValid(fri[src[i].id-1]);
            if(sat && mode==0x44)b=F1CombinedValid(sat[src[i].id-1]);
        }
        /* Current-session best supplies live updates before native session arrays commit. */
        if(src[i].best>0 && (!a || src[i].best<a))a=src[i].best;
        if(b && (!a || b<a))a=b;
        dst[i].best=a;
    }
    /* Stable insertion order: timed drivers first, equal times retain native day order. */
    for(i=1;i<n;i++) {tmp=dst[i];j=i;
        while(j>0 && tmp.best && (!dst[j-1].best || tmp.best<dst[j-1].best)) {dst[j]=dst[j-1];j--;}
        dst[j]=tmp;
    }
    for(i=0;i<n;i++)dst[i].pos=dst[i].best?i+1:0;
    return n;
}
