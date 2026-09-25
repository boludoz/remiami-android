#include "common.h"

#include "Collision.h"
#include "Curves.h"

#define SA
#ifdef SA

// Helper inline functions
inline float
DistToMathematicalLine2D(float lineX, float lineY, float dirX, float dirY, float pointX, float pointY)
{
	// Distance from point to line defined by (lineX, lineY) with direction (dirX, dirY)
	float dx = pointX - lineX;
	float dy = pointY - lineY;
	return Abs(dx * dirY - dy * dirX);
}

// func: sa 0x43C610
float
CCurves::DistForLineToCrossOtherLine(float LineBaseX, float LineBaseY, float LineDirX, float LineDirY, float OtherLineBaseX, float OtherLineBaseY,
                                     float OtherLineDirX, float OtherLineDirY)
{
	float Dir = LineDirX * OtherLineDirY - LineDirY * OtherLineDirX;

	if(Dir == 0.0f) {
		return -1.0f; // Lines are parallel, no intersection
	}

	float Dist = (LineBaseX - OtherLineBaseX) * OtherLineDirY - (LineBaseY - OtherLineBaseY) * OtherLineDirX;
	float DistOfCrossing = -Dist / Dir;

	return DistOfCrossing;
}

// func: sa 0x43C660
float
CCurves::CalcSpeedVariationInBend(CVector *startCoors, CVector *endCoors, float StartDirX, float StartDirY, float EndDirX, float EndDirY)
{
	float ReturnVal = 0.0f;
	float DotProduct = StartDirX * EndDirX + StartDirY * EndDirY;

	if(DotProduct <= 0.0f) {
		// If the dot product is <= 0, return a constant value (1/3)
		ReturnVal = 1.0f / 3.0f;
		return ReturnVal;
	}

	if(DotProduct > 0.7f) {
		// Calculate the distance from the start point to the mathematical line defined by the end point and direction
		float DistToLine = DistToMathematicalLine2D(endCoors->x, endCoors->y, EndDirX, EndDirY, startCoors->x, startCoors->y);

		// Calculate the straight-line distance between the start and end points
		float StraightDist = (*startCoors - *endCoors).Magnitude2D();

		// Normalize the distance to the line by the straight-line distance
		ReturnVal = (DistToLine / StraightDist) * (1.0f / 3.0f);
		return ReturnVal;
	}

	// If the dot product is <= 0.7, interpolate the return value
	ReturnVal = (1.0f - (DotProduct / 0.7f)) * (1.0f / 3.0f);

	return ReturnVal;
}

// func: sa 0x43C710
float
CCurves::CalcSpeedScaleFactor(CVector *startCoors, CVector *endCoors, float StartDirX, float StartDirY, float EndDirX, float EndDirY)
{
	float SpeedVariation = CalcSpeedVariationInBend(startCoors, endCoors, StartDirX, StartDirY, EndDirX, EndDirY);

	float DistToPoint1 = DistForLineToCrossOtherLine(startCoors->x, startCoors->y, StartDirX, StartDirY, endCoors->x, endCoors->y, EndDirX, EndDirY);

	float DistToPoint2 = DistForLineToCrossOtherLine(endCoors->x, endCoors->y, -EndDirX, -EndDirY, startCoors->x, startCoors->y, StartDirX, StartDirY);

	float StraightDist1;
	float StraightDist2;
	float BendDist;
	float BendDist_Time;
	float BendDistOneSegment;
	float TotalDist_Time;

	if(DistToPoint1 > 0.0f && DistToPoint2 > 0.0f) {
		BendDistOneSegment = Min(DistToPoint1, DistToPoint2);
		BendDistOneSegment = Min(5.0f, BendDistOneSegment);

		StraightDist1 = DistToPoint1 - BendDistOneSegment;
		StraightDist2 = DistToPoint2 - BendDistOneSegment;

		BendDist = 2.0f * BendDistOneSegment;

		TotalDist_Time = BendDist;
	} else {
		BendDist = (*startCoors - *endCoors).Magnitude2D();
		BendDist_Time = 1.0f - SpeedVariation;

		StraightDist1 = 0.0f;
		StraightDist2 = 0.0f;

		TotalDist_Time = BendDist / BendDist_Time;
	}

	return TotalDist_Time + StraightDist1 + StraightDist2;
}

// func: sa 0x43C880
float
CCurves::CalcCorrectedDist(float Current, float Total, float SpeedVariation, float *pInterPol)
{
	if(Total < 0.00001f) // Epsilon to avoid division by zero
	{
		*pInterPol = 0.5f;
		return 0.0f;
	}

	*pInterPol = 0.5f - (Cos(PI * (Current / Total)) * 0.5f);

	float AverageSpeed = Sin((Current / Total) * TWOPI);
	float CorrectedDist = AverageSpeed * (Total * (1.0f / TWOPI)) * SpeedVariation + ((1.0f - (SpeedVariation + SpeedVariation) + 1.0f) * 0.5f) * Current;

	return CorrectedDist;
}

// func: sa 0x43C900
void
CCurves::CalcCurvePoint(CVector *startCoors, CVector *endCoors, CVector *startDir, CVector *endDir, float Time, int32 TraverselTimeInMillis,
                        CVector *resultCoor, CVector *resultSpeed)
{
	float BendDist, BendDist_Time, CurrentDist_Time, Interpol, StraightDist2, StraightDist1, TotalDist_Time, OurTime;
	float BendDistOneSegment;
	CVector CoorsOnLine1, CoorsOnLine2;

	Time = Max(Time, 0.0f);
	Time = Min(Time, 1.0f);

	float SpeedVariation = CalcSpeedVariationInBend(startCoors, endCoors, startDir->x, startDir->y, endDir->x, endDir->y);

	// Find where the ray from start position would intersect with end ray
	float DistToPoint1 =
	    DistForLineToCrossOtherLine(startCoors->x, startCoors->y, startDir->x, startDir->y, endCoors->x, endCoors->y, endDir->x, endDir->y);

	// Find where the ray from end position would intersect with start ray (negative because direction is flipped)
	float DistToPoint2 =
	    DistForLineToCrossOtherLine(endCoors->x, endCoors->y, -endDir->x, -endDir->y, startCoors->x, startCoors->y, startDir->x, startDir->y);

	// Initialize variables that must always have a value
	BendDist = 0.0f;
	BendDistOneSegment = 0.0f;
	StraightDist2 = 0.0f;

	if(DistToPoint1 <= 0.0f || DistToPoint2 <= 0.0f) {
		// Case when the lines do not intersect properly
		CVector diff = *startCoors - *endCoors;
		StraightDist1 = diff.Magnitude2D();

		Interpol = StraightDist1 / (1.0f - SpeedVariation);

		CurrentDist_Time = CalcCorrectedDist(Time * Interpol, Interpol, SpeedVariation, &OurTime);

		CoorsOnLine1 = *startCoors + *startDir * CurrentDist_Time;
		CoorsOnLine2 = *endCoors + *endDir * (CurrentDist_Time - StraightDist1);

		*resultCoor = CoorsOnLine1 * (1.0f - OurTime) + CoorsOnLine2 * OurTime;

		TotalDist_Time = StraightDist1; // Only straight distance
	} else {
		// Case when there is a valid curve
		StraightDist2 = Min(DistToPoint1, DistToPoint2);
		StraightDist2 = Min(5.0f, StraightDist2);

		BendDist = DistToPoint1 - StraightDist2;
		BendDistOneSegment = DistToPoint2 - StraightDist2;
		TotalDist_Time = BendDist + (StraightDist2 * 2.0f) + BendDistOneSegment;
		BendDist_Time = Time * TotalDist_Time;

		if(BendDist_Time < BendDist) {
			// We are in the first straight segment
			*resultCoor = *startCoors + *startDir * BendDist_Time;
		} else if(BendDist_Time < BendDist + (StraightDist2 * 2.0f)) {
			// We are in the curve section
			float BendInter = (BendDist_Time - BendDist) / (StraightDist2 * 2.0f);

			CVector BendStartCoors = *startCoors + *startDir * BendDist + *startDir * (StraightDist2 * BendInter);

			CVector BendEndCoors = *endCoors - *endDir * BendDistOneSegment - *endDir * (StraightDist2 * (1.0f - BendInter));

			*resultCoor = BendStartCoors * (1.0f - BendInter) + BendEndCoors * BendInter;
		} else {
			// We are in the last straight segment
			BendDist_Time = BendDist_Time - TotalDist_Time;
			*resultCoor = *endCoors + *endDir * BendDist_Time;
		}
	}

	// Calculate the resulting speed
	float SpeedMillisFactor = (float)TraverselTimeInMillis / 1000.0f;

	resultSpeed->x = (TotalDist_Time * ((Time * endDir->x) + ((1.0f - Time) * startDir->x))) / SpeedMillisFactor;
	resultSpeed->y = (TotalDist_Time * ((Time * endDir->y) + ((1.0f - Time) * startDir->y))) / SpeedMillisFactor;
	resultSpeed->z = 0.0f;
}

#else
float
CCurves::CalcSpeedScaleFactor(CVector *pPoint1, CVector *pPoint2, float dir1X, float dir1Y, float dir2X, float dir2Y)
{
	CVector2D dir1(dir1X, dir1Y);
	CVector2D dir2(dir2X, dir2Y);
	float distance = (*pPoint1 - *pPoint2).Magnitude2D();
	float dp = DotProduct2D(dir1, dir2);
	if(dp > 0.9f)
		return distance + Abs((pPoint1->x * dir1Y - pPoint1->y * dir1X) - (pPoint2->x * dir1Y - pPoint2->y * dir1X));
	else
		return ((1.0f - dp) * 0.25f + 1.0f) * distance;
}

void
CCurves::CalcCurvePoint(CVector *pPos1, CVector *pPos2, CVector *pDir1, CVector *pDir2, float between, int32 timeOnCurve, CVector *pOutPos, CVector *pOutDir)
{
	float actualFactor = CalcSpeedScaleFactor(pPos1, pPos2, pDir1->x, pDir1->y, pDir2->x, pDir2->y);
	CVector2D dir1 = *pDir1 * actualFactor;
	CVector2D dir2 = *pDir2 * actualFactor;
	float t1 = Abs(DotProduct2D(*pPos2 - *pPos1, *pDir1));
	float t2 = Abs(DotProduct2D(*pPos1 - *pPos2, *pDir2));
	float curveCoef;
	if(t1 > t2) {
		float coef = (t1 - t2) / (t1 + t2);
#ifdef FIX_BUGS
		if(between <= coef)
#else
		if(between < coef)
#endif
			curveCoef = 0.0f;
		else
			curveCoef = 0.5f - 0.5f * Cos(3.1415f * (between - coef) * (t1 + t2) / (2 * t2));
	} else {
		float coef = 2 * t1 / (t1 + t2);
#ifdef FIX_BUGS
		if(coef <= between)
#else
		if(coef < between)
#endif
			curveCoef = 1.0f;
		else
			curveCoef = 0.5f - 0.5f * Cos(3.1415f * between * (t1 + t2) / (2 * t1));
	}
	*pOutPos = CVector((pPos1->x + between * dir1.x) * (1.0f - curveCoef) + (pPos2->x - (1 - between) * dir2.x) * curveCoef,
	                   (pPos1->y + between * dir1.y) * (1.0f - curveCoef) + (pPos2->y - (1 - between) * dir2.y) * curveCoef, 0.0f);
	*pOutDir = CVector((dir1.x * (1.0f - curveCoef) + dir2.x * curveCoef) / (timeOnCurve * 0.001f),
	                   (dir1.y * (1.0f - curveCoef) + dir2.y * curveCoef) / (timeOnCurve * 0.001f), 0.0f);
}
#endif