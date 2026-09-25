#pragma once
class CVector;
#define SA

class CCurves
{
public:
#ifdef SA
	static float DistForLineToCrossOtherLine(float LineBaseX, float LineBaseY, float LineDirX, float LineDirY, float OtherLineBaseX, float OtherLineBaseY,
	                                         float OtherLineDirX, float OtherLineDirY);
	static float CalcSpeedVariationInBend(CVector *startCoors, CVector *endCoors, float StartDirX, float StartDirY, float EndDirX, float EndDirY);
	static float CalcSpeedScaleFactor(CVector *startCoors, CVector *endCoors, float StartDirX, float StartDirY, float EndDirX, float EndDirY);
	static void CalcCurvePoint(CVector *startCoors, CVector *endCoors, CVector *startDir, CVector *endDir, float Time, int32 TraverselTimeInMillis,
	                           CVector *resultCoor, CVector *resultSpeed);
	static float CalcCorrectedDist(float Current, float Total, float SpeedVariation, float *pInterPol);
#else
	static float CalcSpeedScaleFactor(CVector *, CVector *, float, float, float, float);
	static void CalcCurvePoint(CVector *, CVector *, CVector *, CVector *, float, int32, CVector *, CVector *);
#endif
};
