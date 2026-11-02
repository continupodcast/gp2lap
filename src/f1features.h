#ifndef F1FEATURES_H
#define F1FEATURES_H
#include "f1modules.h"
extern unsigned long StrategyPlayerEnabled,StrategyPlayerPitChoice;
extern unsigned long StrategyDistributionEnabled,StrategyDiagnostics;
extern unsigned long StrategyAutoEnabled,StrategyAutoSeed,StrategyRepairTestEnabled,StrategyTestEnabled,TyreTestFuelLaps,PerCarTyreCompounds,TyreDiagnostics,TyreTestCompound,EnhancedCameras;
#define F1_TYRE_EXTRA ((F1_MODULE_TYRE_COMPOUND_HUD && PerCarTyreCompounds)?15:0)
#endif
