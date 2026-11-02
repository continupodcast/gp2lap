
    unsigned long index;
    if(camNotice[0] && (!pCurTime || *pCurTime<camNoticeAt || *pCurTime-camNoticeAt>=2500UL)){
        camNotice[0]=0;activepage&=~PAGE_F1CAMNOTICE;
    }
    if(!EnhancedCameras || !camReady || !pCurTime)return;
    index=(*camMode==0xc0)?*camIndex:0;
    camTestEnabled=0;
    if(index==9 && camDynamicOn){
        camTestExpected=(unsigned long)(camTable+9*16);camPitch=-1820;
        /* Default pose is safe even while waiting for valid car telemetry. */
        camOffsetX=0;camOffsetY=(short)-camChaseRadius;camOffsetZ=200;camOrbitYaw=0;
        CamOrbitCompute(*pCurTime);camTestEnabled=1;
    }else {camFollowValid=0;}
    if(index==7 && camTargetOn){
        camTestExpected=(unsigned long)(camTable+7*16);
        CamTargetCompute(*pCurTime);camTestEnabled=1;
    }else {CamTargetReset();}
    if(index==8 && cam360On){
        camTestExpected=(unsigned long)(camTable+8*16);
        Cam360Compute(*pCurTime);camTestEnabled=1;
    }

