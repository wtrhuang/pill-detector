#include "PillDetect.h"


PillDetect::PillDetect(void)
{
	noOfCordinates = 0;  s
	modelDefined = false; 
}

int PillDetect::CreatePillDetectModel(const void *templateArr,double maxContrast,double minContrast)
{

	CvMat *gx = 0;		
	CvMat *gy = 0;	
	CvMat *nmsEdges = 0;		
	CvSize Ssize;
		
	// Convert IplImage to Matrix for integer operations
	CvMat srcstub, *src = (CvMat*)templateArr;
	src = cvGetMat( src, &srcstub );
	if(CV_MAT_TYPE( src->type ) != CV_8UC1)
	{	
		return 0;
	}
	
	Ssize.width =  src->width;
	Ssize.height= src->height;
	modelHeight =src->height;		
	modelWidth =src->width;			

	noOfCordinates=0;												
	cordinates =  new CvPoint[ modelWidth *modelHeight];		
	
	edgeMagnitude = new double[ modelWidth *modelHeight];		
	edgeDerivativeX = new double[modelWidth *modelHeight];			
	edgeDerivativeY = new double[modelWidth *modelHeight];			


	gx = cvCreateMat( Ssize.height, Ssize.width, CV_16SC1 );		
	gy = cvCreateMat( Ssize.height, Ssize.width, CV_16SC1 );		
	cvSobel( src, gx, 1,0, 3 );					
	cvSobel( src, gy, 0, 1, 3 );	
	
	nmsEdges = cvCreateMat( Ssize.height, Ssize.width, CV_32F);		//create Matrix to store Final nmsEdges
	const short* _sdx; 
	const short* _sdy; 
	double fdx,fdy;	
    double MagG, DirG;
	double MaxGradient=-99999.99;
	double direction;
	int *orients = new int[ Ssize.height *Ssize.width];
	int count = 0,i,j; 
	
	double **magMat;
	CreateDoubleMatrix(magMat ,Ssize);
	
	for( i = 1; i < Ssize.height-1; i++ )
    {
    	for( j = 1; j < Ssize.width-1; j++ )
        { 		 
				_sdx = (short*)(gx->data.ptr + gx->step*i);
				_sdy = (short*)(gy->data.ptr + gy->step*i);
				fdx = _sdx[j]; fdy = _sdy[j];        s

				MagG = sqrt((float)(fdx*fdx) + (float)(fdy*fdy)); 
				direction =cvFastArctan((float)fdy,(float)fdx);	 
				magMat[i][j] = MagG;
				
				if(MagG>MaxGradient)
					MaxGradient=MagG; 

				
					// get closest angle from 0, 45, 90, 135 set
                        if ( (direction>0 && direction < 22.5) || (direction >157.5 && direction < 202.5) || (direction>337.5 && direction<360)  )
                            direction = 0;
                        else if ( (direction>22.5 && direction < 67.5) || (direction >202.5 && direction <247.5)  )
                            direction = 45;
                        else if ( (direction >67.5 && direction < 112.5)||(direction>247.5 && direction<292.5) )
                            direction = 90;
                        else if ( (direction >112.5 && direction < 157.5)||(direction>292.5 && direction<337.5) )
                            direction = 135;
                        else 
							direction = 0;
				
			orients[count] = (int)direction;
			count++;
		}
	}
	
	count=0; 
	
	double leftPixel,rightPixel;
	
	for( i = 1; i < Ssize.height-1; i++ )
    {
		for( j = 1; j < Ssize.width-1; j++ )
        {
				switch ( orients[count] )
                {
                   case 0:
                        leftPixel  = magMat[i][j-1];
                        rightPixel = magMat[i][j+1];
                        break;
                    case 45:
                        leftPixel  = magMat[i-1][j+1];
						rightPixel = magMat[i+1][j-1];
                        break;
                    case 90:
                        leftPixel  = magMat[i-1][j];
                        rightPixel = magMat[i+1][j];
                        break;
                    case 135:
                        leftPixel  = magMat[i-1][j-1];
                        rightPixel = magMat[i+1][j+1];
                        break;
				 }
				
                if (( magMat[i][j] < leftPixel ) || (magMat[i][j] < rightPixel ) )
					(nmsEdges->data.ptr + nmsEdges->step*i)[j]=0;
                else
                    (nmsEdges->data.ptr + nmsEdges->step*i)[j]=(uchar)(magMat[i][j]/MaxGradient*255);
			
				count++;
			}
		}
	

	int RSum=0,CSum=0;
	int curX,curY;
	int flag=1;

	for( i = 1; i < Ssize.height-1; i++ )
    {
		for( j = 1; j < Ssize.width; j++ )
        {
			_sdx = (short*)(gx->data.ptr + gx->step*i);
			_sdy = (short*)(gy->data.ptr + gy->step*i);
			fdx = _sdx[j]; fdy = _sdy[j];
				
			MagG = sqrt(fdx*fdx + fdy*fdy); 
			DirG =cvFastArctan((float)fdy,(float)fdx);	 
		
			
			flag=1;
			if(((double)((nmsEdges->data.ptr + nmsEdges->step*i))[j]) < maxContrast)
			{
				if(((double)((nmsEdges->data.ptr + nmsEdges->step*i))[j])< minContrast)
				{
					
					(nmsEdges->data.ptr + nmsEdges->step*i)[j]=0;
					flag=0; 
				}
				else
				{   
					if( (((double)((nmsEdges->data.ptr + nmsEdges->step*(i-1)))[j-1]) < maxContrast)	&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*(i-1)))[j]) < maxContrast)		&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*(i-1)))[j+1]) < maxContrast)	&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*i))[j-1]) < maxContrast)		&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*i))[j+1]) < maxContrast)		&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*(i+1)))[j-1]) < maxContrast)	&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*(i+1)))[j]) < maxContrast)		&&
						(((double)((nmsEdges->data.ptr + nmsEdges->step*(i+1)))[j+1]) < maxContrast)	)
					{
						(nmsEdges->data.ptr + nmsEdges->step*i)[j]=0;
						flag=0;
						
					}
				}
				
			}
			
			curX=i;	curY=j;
			if(flag!=0)
			{
				if(fdx!=0 || fdy!=0)
				{		
					RSum=RSum+curX;	CSum=CSum+curY; 
					
					cordinates[noOfCordinates].x = curX;
					cordinates[noOfCordinates].y = curY;
					edgeDerivativeX[noOfCordinates] = fdx;
					edgeDerivativeY[noOfCordinates] = fdy;
					
					
					if(MagG!=0)
						edgeMagnitude[noOfCordinates] = 1/MagG;  
					else
						edgeMagnitude[noOfCordinates] = 0;
															
					noOfCordinates++;
				}
			}
		}
	}

	centerOfGravity.x = RSum /noOfCordinates; 
	centerOfGravity.y = CSum/noOfCordinates ;	
		
	for(int m=0;m<noOfCordinates ;m++)
	{
		int temp;

		temp=cordinates[m].x;
		cordinates[m].x=temp-centerOfGravity.x;
		temp=cordinates[m].y;
		cordinates[m].y =temp-centerOfGravity.y;
	}
	
	
	delete[] orients;
	cvReleaseMat( &gx );
    cvReleaseMat( &gy );
	cvReleaseMat(&nmsEdges);

	ReleaseDoubleMatrix(magMat ,Ssize.height);
	
	modelDefined=true;
	return 1;
}


double PillDetect::FindPillDetectModel(const void* srcarr,double minScore,double greediness,CvPoint *resultPoint)
{
	CvMat *Sdx = 0, *Sdy = 0;
	
	double resultScore=0;
	double partialSum=0;
	double sumOfCoords=0;
	double partialScore;
	const short* _Sdx;
	const short* _Sdy;
	int i,j,m ;			
	double iTx,iTy,iSx,iSy;
	double gradMag;    
	int curX,curY;

	double **matGradMag; 
	
	CvMat srcstub, *src = (CvMat*)srcarr;
	src = cvGetMat( src, &srcstub );
	if(CV_MAT_TYPE( src->type ) != CV_8UC1 || !modelDefined)
	{
		return 0;
	}

	CvSize Ssize;
	Ssize.width =  src->width;
	Ssize.height= src->height;
	
	CreateDoubleMatrix(matGradMag ,Ssize); 
		
	Sdx = cvCreateMat( Ssize.height, Ssize.width, CV_16SC1 ); 
	Sdy = cvCreateMat( Ssize.height, Ssize.width, CV_16SC1 ); 
	
	cvSobel( src, Sdx, 1, 0, 3 );  
	cvSobel( src, Sdy, 0, 1, 3 ); 
		
	double normMinScore = minScore /noOfCordinates; 
	double normGreediness = ((1- greediness * minScore)/(1-greediness)) /noOfCordinates; 
		
	for( i = 0; i < Ssize.height; i++ )
    {
		 _Sdx = (short*)(Sdx->data.ptr + Sdx->step*(i));
		 _Sdy = (short*)(Sdy->data.ptr + Sdy->step*(i));
		
		 for( j = 0; j < Ssize.width; j++ )
		{
				iSx=_Sdx[j];  
				iSy=_Sdy[j];  

				gradMag=sqrt((iSx*iSx)+(iSy*iSy)); 
							
				if(gradMag!=0) 
					matGradMag[i][j]=1/gradMag;   
				else
					matGradMag[i][j]=0;
				
		}
	}
	for( i = 0; i < Ssize.height; i++ )
    {
			for( j = 0; j < Ssize.width; j++ )
             { 
				 partialSum = 0; 
				 for(m=0;m<noOfCordinates;m++)
				 {
					 curX	= i + cordinates[m].x ;	
					 curY	= j + cordinates[m].y ; 
					 iTx	= edgeDerivativeX[m];	
					 iTy	= edgeDerivativeY[m];    

					 if(curX<0 ||curY<0||curX>Ssize.height-1 ||curY>Ssize.width-1)
						 continue;
					 
					 _Sdx = (short*)(Sdx->data.ptr + Sdx->step*(curX));
					 _Sdy = (short*)(Sdy->data.ptr + Sdy->step*(curX));
						
					 iSx=_Sdx[curY]; 
					 iSy=_Sdy[curY];
						
					if((iSx!=0 || iSy!=0) && (iTx!=0 || iTy!=0))
					 {
						 partialSum = partialSum + ((iSx*iTx)+(iSy*iTy))*(edgeMagnitude[m] * matGradMag[curX][curY]);
									
					 }

					sumOfCoords = m + 1;
					partialScore = partialSum /sumOfCoords ;
					// check termination criteria
					if( partialScore < (MIN((minScore -1) + normGreediness*sumOfCoords,normMinScore*  sumOfCoords)))
						break;
									
				}
				if(partialScore > resultScore)
				{
					resultScore = partialScore; 
					resultPoint->x = i;				
					resultPoint->y = j;			
				}
			} 
		}
	
	ReleaseDoubleMatrix(matGradMag ,Ssize.height);
	cvReleaseMat( &Sdx );
	cvReleaseMat( &Sdy );
	
	return resultScore;
}
PillDetect::~PillDetect(void)
{
	delete[] cordinates ;
	delete[] edgeMagnitude;
	delete[] edgeDerivativeX;
	delete[] edgeDerivativeY;
}

void PillDetect::CreateDoubleMatrix(double **&matrix,CvSize size)
{
	matrix = new double*[size.height];
	for(int iInd = 0; iInd < size.height; iInd++)
		matrix[iInd] = new double[size.width];
}
void PillDetect::ReleaseDoubleMatrix(double **&matrix,int size)
{
	for(int iInd = 0; iInd < size; iInd++) 
        delete[] matrix[iInd]; 
}


void PillDetect::DrawContours(IplImage* source,CvPoint COG,CvScalar color,int lineWidth)
{
	CvPoint point;
	point.y=COG.x;
	point.x=COG.y;
	for(int i=0; i<noOfCordinates; i++)
	{	
		point.y=cordinates[i].x + COG.x;
		point.x=cordinates[i].y + COG.y;
		cvLine(source,point,point,color,lineWidth);
	}
}

void PillDetect::DrawContours(IplImage* source,CvScalar color,int lineWidth)
{
	CvPoint point;
	for(int i=0; i<noOfCordinates; i++)
	{
		point.y=cordinates[i].x + centerOfGravity.x;
		point.x=cordinates[i].y + centerOfGravity.y;
		cvLine(source,point,point,color,lineWidth);
	}
}

