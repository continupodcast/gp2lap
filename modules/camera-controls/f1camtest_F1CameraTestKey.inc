
    char message[64];int requested;
    if(scan==0xa4 || scan==0xa6 || scan==0x96){
        if(camOrbitHeld==(int)(scan&127))camOrbitHeld=0;return;
    }
    if(!EnhancedCameras || !camReady || (scan&128) || !control)return;
    requested=scan==0x24?1:scan==0x26?2:scan==0x16?3:0;
    if(requested && camOrbitHeld!=(int)scan){
        camOrbitHeld=(int)scan;
        if(F1CockpitView){CamShowNotice("External view required");CamLog("BLOCKED_COCKPIT",scan,control);return;}
        if(!camCycle){camSavedMode=*camMode;camSavedIndex=*camIndex;camSavedView=1;}
        if(requested==3){
            if(camCycle!=3){camCycle=3;camTargetExternal=0;}
            else if(!camTargetExternal)camTargetExternal=1;
            else {camCycle=0;camTargetExternal=0;}
        }else {camCycle=camCycle==requested?0:requested;camTargetExternal=0;}
        camDynamicOn=camCycle==1;cam360On=camCycle==2;camTargetOn=camCycle==3;
        camFollowValid=0;CamTargetReset();
        if(!camCycle)CamCycleOriginal();
        else {
            *camMode=0xc0;*camIndex=camCycle==1?9:camCycle==2?8:7;
            cam360Start=pCurTime?*pCurTime:0;
            CamShowNotice(camCycle==1?"Dynamic Chase Camera":camCycle==2?"360 Camera":camTargetExternal?"External Target Camera":"Target Camera");
        }
        CamTestUpdate();CamLog("CAMERA_DIRECT",scan,control);
    }else if(scan==0x13){
        CamCycleOriginal();CamTestUpdate();CamLog("CAMERA_RESTORE",scan,control);
    }else if((scan==0x17 || scan==0x25) && *camMode==0xc0 &&
             ((*camIndex==8 && cam360On) || (*camIndex==9 && camDynamicOn))){
        int *radius=(*camIndex==8)?&cam360Radius:&camChaseRadius;
        *radius+=(scan==0x25)?100:-100;
        if(*radius<300)*radius=300;
        if(*radius>2000)*radius=2000;
        sprintf(message,"%s distance: %d",*camIndex==8?"360":"Chase",*radius);CamShowNotice(message);
        CamTestUpdate();CamLog("CAMERA_DISTANCE",scan,control);
    }

