
#pragma once
#include <cv.h>
#include <cxcore.h>
#include <math.h>

class PillDetect
{
private:
	int				noOfCordinates;		
	CvPoint			*cordinates;		//Coordinates array to store model points	
	int				modelHeight;		
	int				modelWidth;			
	double			*edgeMagnitude;		
	double			*edgeDerivativeX;	
	double			*edgeDerivativeY;	
	CvPoint			centerOfGravity;	
	
	bool			modelDefined;
	
	void CreateDoubleMatrix(double **&matrix,CvSize size);
	void ReleaseDoubleMatrix(double **&matrix,int size);
public:
	PillDetect(void);
	PillDetect(const void* templateArr);
	~PillDetect(void);

	int CreatePillDetectModel(const void* templateArr,double,double);
	double FindPillDetectModel(const void* srcarr,double minScore,double greediness, CvPoint *resultPoint);
	void DrawContours(IplImage* pImage,CvPoint COG,CvScalar,int);
	void DrawContours(IplImage* pImage,CvScalar,int);
};
