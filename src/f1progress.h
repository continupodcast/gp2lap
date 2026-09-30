/* GP2 fixed-point segment coordinate, including small boundary overlap.
   Decode explicitly, independent of compiler signed-short conversions. */
#ifndef F1PROGRESS_H
#define F1PROGRESS_H
static int F1SegmentFactor(unsigned int raw,double *fraction)
{
 long value=(long)(raw&65535U);
 if(value>=32768)value-=65536;
 /* Logs show -176..-15 and 16393..16592 at segment transitions.
    Allow 1/32 segment overlap, reject unrelated/corrupt coordinates. */
 if(value < -512 || value > 16896)return 0;
 *fraction=value/16384.0;return 1;
}
static double F1SegmentDistance(const double *distances,int segments,int segment,double fraction)
{
 double distance=distances[segment]+fraction*(distances[segment+1]-distances[segment]);
 /* Keep native lap identity at the finish line. Do not wrap a small
    overshoot into the opposite end of the same race lap. */
 if(distance<0)distance=0;
 if(distance>=distances[segments])distance=distances[segments]-0.000001;
 return distance;
}
#endif
