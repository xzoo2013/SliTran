#include "stdafx.h"
#include <stdlib.h>
using namespace std;
using namespace cv;

//Global variables definition=====================
static CvMemStorage* storage = 0;
static CvHaarClassifierCascade* cascadebody = 0;
static CvHaarClassifierCascade* cascadefeature = 0;
static CvScalar colors[] = {{{0,0,255}},{{0,128,255}},{{0,255,255}},{{0,255,0}},{{255,128,0}},{{255,255,0}},{{255,0,0}},{{255,0,255}}};
//Customisable variables--------------
static int bodyDetectSize=100;
static int faceDetectSize=20;
const char* cascadebody_name = "haarcascade_mcs_upperbody.xml";
const char* cascadefeature_name = "hand.xml";
SYSTEMTIME currentTime;

clock_t  stepTimeBodyDect;	
clock_t  stepTimeFeatDectS1;	
clock_t  stepTimeFeatDectS2;
clock_t  actionLockClock;
int flagBodyDetected=0;
int flagFeatureDetectedS1=0;
int flagFeatureDetectedS2=0;
int isDealWithPdfFile=1;
int camW=640;
int camH=480;

//Function difinition============================= 
CvRect detectbody_and_draw( IplImage* image, CvRect rectTemp );
CvRect detectface_and_draw( IplImage* image, CvRect rectTemp,HWND hWnd);

//Main============================================
int _tmain(int argc, _TCHAR* argv[])
{
	//Varibles initialization--------
	CvCapture* capture = 0;
	IplImage *frame, *frame_copy = 0;	
	cascadebody = (CvHaarClassifierCascade*)cvLoad( cascadebody_name, 0, 0, 0 );
	cascadefeature = (CvHaarClassifierCascade*)cvLoad( cascadefeature_name, 0, 0, 0 );
	CvRect rectTemp=cvRect(0,0,3,3);
	CvRect rectTempS1=cvRect(0,0,3,3);
	CvRect rectTempS2=cvRect(0,0,3,3);
	CvRect rectTempS3=cvRect(0,0,3,3);
	if( !cascadebody || !cascadefeature){
        printf( "ERROR: Could not load classifier cascade\n" );
        return -1;
    }
	HWND hWnd; 


	stepTimeBodyDect=clock();
	printf("curenttimeis  %d\n", stepTimeBodyDect );

	//Get the target window handle---- 
	if(isDealWithPdfFile==1)
	{
		hWnd=FindWindow(_T("AcrobatSDIWindow"),NULL);
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVToolBarHostView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVSplitterView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVSplitationPageView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVSplitterView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVScrolledPageView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVScrollView"));
		hWnd = FindWindowEx(hWnd, NULL,_T("AVL_AVView"),_T("AVPageView"));
		if(hWnd!=NULL){
			printf("PDF detected, id= %x\n",(int) hWnd );
		}else{
			 printf( "ERROR: Could not load classifier cascade\n" );
			return -2;
		}
	}

	storage = cvCreateMemStorage(0);
	capture = cvCaptureFromCAM( 0 );
	cvNamedWindow( "imgBody", 1 );
	cvNamedWindow( "imgFeature", 0 );
	cvResizeWindow("imgFeature",300,300); 
	actionLockClock=clock()+1000;
	if( capture )
	{
		for(;;)
        {
            if( !cvGrabFrame( capture )) break;
            frame = cvRetrieveFrame( capture );
            if( !frame )break;
            if( !frame_copy ) frame_copy = cvCreateImage( cvSize(frame->width,frame->height),IPL_DEPTH_8U, frame->nChannels );
            cvFlip( frame, frame_copy, 1 );
			//printf("%d    %d", frame->width,frame->height);

			
			GetSystemTime(&currentTime);
			
			if ((int)(clock()-stepTimeBodyDect)>=1000 || flagBodyDetected==0 )
			{				
				//cvShowImage( "result", frame_copy );
				//printf( "started dectec the body with time step: %d ms\n",clock()-stepTimeBodyDect );
				rectTemp=detectbody_and_draw( frame_copy,rectTemp);				
				
			}

			if (1)
			{
				//rectTempS1=detectface_and_draw(frame_copy,rectTemp,hWnd);
				int waitTime=500;
				switch(flagFeatureDetectedS2){
				case 0:
					rectTempS1=detectface_and_draw(frame_copy,rectTemp,hWnd);
					//if dectect a feature and the feature move than go to case 1, or wait for 0.5s to case 0
					if (rectTempS1.x!=5 && flagFeatureDetectedS1==1){
						flagFeatureDetectedS2=1;
						stepTimeFeatDectS2=clock();
					}
					break;
				case 1:
					rectTempS2=detectface_and_draw(frame_copy,rectTemp,hWnd);
					//if dectect a feature and the feature move than go to case 2 or case 3 depending on the first detected direction, or wait for 0.5s to case 0
					if (rectTempS2.x!=5 && flagFeatureDetectedS1==1 && rectTempS2.x-rectTempS1.x>10)
					{
						flagFeatureDetectedS2=2;
						stepTimeFeatDectS2=clock();
						printf(" Succeed to dected the Second feature To Right with (x,y) = ( %d ,%d )\n",rectTempS2.x,rectTempS2.y);
					}else if(rectTempS3.x!=5 && flagFeatureDetectedS1==1 && rectTempS2.x-rectTempS1.x<-10){
						flagFeatureDetectedS2=3;
						stepTimeFeatDectS2=clock();
						printf(" Succeed to dected the Second feature To Left with (x,y) = ( %d ,%d )\n",rectTempS2.x,rectTempS2.y);
					}else if(clock()-stepTimeFeatDectS2>waitTime){
						flagFeatureDetectedS2=0;
						stepTimeFeatDectS2=clock();
						printf(" Failed to dected the Second feature \n");
					}
					break;
					
				case 2:
					rectTempS3=detectface_and_draw(frame_copy,rectTemp,hWnd);
					//if dectect a feature and the feature continue move to the right than go to 4, or wait for 0.5s to case 0
					if (rectTempS3.x!=5 && flagFeatureDetectedS1==1 && rectTempS3.x-rectTempS2.x>10 && ( rectTempS3.width/rectTempS2.width<1.5 || rectTempS3.width/rectTempS2.width>0.7 ))
					{
						flagFeatureDetectedS2=4;
						stepTimeFeatDectS2=clock();
						printf(" Succeed to dected the Third feature To Right with (x,y) = ( %d ,%d )\n",rectTempS3.x,rectTempS3.y);
					}else if(clock()-stepTimeFeatDectS2>waitTime){
						flagFeatureDetectedS2=0;
						stepTimeFeatDectS2=clock();
						printf(" Failed to dected the Third feature To Right\n");
					}						
					break;
				case 3:
					rectTempS3=detectface_and_draw(frame_copy,rectTemp,hWnd);
					//if dectect a feature and the feature continue move to the left than go to 5, or wait for 0.5s to case 0
					if (rectTempS3.x!=5 && flagFeatureDetectedS1==1 && rectTempS3.x-rectTempS2.x<-10 && ( rectTempS3.width/rectTempS2.width<1.5 || rectTempS3.width/rectTempS2.width>0.7 ))
					{
						flagFeatureDetectedS2=5;
						stepTimeFeatDectS2=clock();
						printf(" Succeed to dected the Third feature To Left with (x,y) = ( %d ,%d )\n",rectTempS3.x,rectTempS3.y);
					}else if(clock()-stepTimeFeatDectS2>waitTime){
						flagFeatureDetectedS2=0;
						stepTimeFeatDectS2=clock();
						printf(" Failed to dected the Third feature To Left\n");
					}						
					break;
				case 4:
					//send a "to right " message to the handle and than to case 0
					if(clock()-actionLockClock>2000){
						printf("Right Right Right Right Right Right \n");
						PostMessage(hWnd,WM_KEYDOWN ,VK_RIGHT, 0);
						actionLockClock=clock();
					}
					flagFeatureDetectedS2=0;
					stepTimeFeatDectS2=clock();
					break;
				case 5:
					//send a "to left " message to the handle and than to case 0
					if(clock()-actionLockClock>2000){
						printf("Left Left Left Left Left Left Left \n");
						PostMessage(hWnd,WM_KEYDOWN ,VK_LEFT, 0);
						actionLockClock=clock();
					}
					flagFeatureDetectedS2=0;
					stepTimeFeatDectS2=clock();
					break;
				}
				
			}





			if( cvWaitKey( 10 ) >= 0 )
                break;
		}
		printf( "ee ");
		cvReleaseImage( &frame_copy );
        cvReleaseCapture( &capture );
	}

	return 0;
}

CvRect detectbody_and_draw( IplImage* img, CvRect rectTemp){
//initial the variables

	int topLeftX=15;
	int topLeftY=15;
	int botRightX=img->width-15;
	int botRightY=img->height-15;
	CvRect rectReturn;
	CvFont font;
	cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 8);
    IplImage* gray = cvCreateImage( cvSize(img->width,img->height), 8, 1 );
	IplImage* outputimg = cvCreateImage( cvSize(camW/2,camH/2), img->depth, img->nChannels);

	cvResize(img, outputimg, CV_INTER_LINEAR);
    cvCvtColor( img, gray, CV_BGR2GRAY );
    cvEqualizeHist( gray, gray );
    cvClearMemStorage( storage );
	
	rectReturn=rectTemp;
	flagBodyDetected=0;

	//loop until find the body;
    if( cascadebody)
    {		
        double t = (double)cvGetTickCount();
		//body detection funcion
		
		 CvSeq* bodies = cvHaarDetectObjects( gray, cascadebody, storage,
                                            1.2, 2, 1/*CV_HAAR_DO_CANNY_PRUNING*/,
                                            cvSize(bodyDetectSize, bodyDetectSize) );
		//when detected a body-----
		if( bodies->total>0)
		{
			 t = (double)cvGetTickCount() - t;
			// printf( "body detected in time = %gms\n", t/((double)cvGetTickFrequency()*1000.) );
			//reconstruct the rectange from the body, which may contain feature(hand or fist here);
			CvRect* r = (CvRect*)cvGetSeqElem( bodies, 0);
			
			if (r->x + r->width*0.6 < gray->width)
				topLeftX=r->x + r->width*0.6;
			else
				topLeftX=gray->width;

			if (r->y+r->height*0.2>0)
				topLeftY=r->y+r->height*0.2;
			else
				topLeftY=0;

			if (r->x + r->width*1.6 < gray->width)
				botRightX=r->x + r->width*1.6;
			else
				botRightX= gray->width;

			if (r->y + r->height*1.2< gray->height)
				botRightY=r->y + r->height*1.2;
			else
				botRightY= gray->height;

			//print the img into window org with a smaller size----
			cvCircle( outputimg, cvPoint((int)(topLeftX*camW/2/img->width),(int)(topLeftY*camH/2/img->height)) ,10 , CV_RGB(250,250,250),1, 8, 3 );
			cvCircle( outputimg, cvPoint((int)(botRightX*camW/2/img->width),(int)(botRightY*camH/2/img->height)) ,10 , CV_RGB(250,250,250),1, 8, 3 );
			cvRectangle( outputimg,cvPoint((int)(topLeftX*camW/2/img->width),(int)(topLeftY*camH/2/img->height)),cvPoint((int)(botRightX*camW/2/img->width),(int)(botRightY*camH/2/img->height)), colors[100%8], 2, 8, 0 ); 
			cvPutText(outputimg, "Body Detected ! " , cvPoint(outputimg->width/2-outputimg->width*0.4, outputimg->height/2), &font, CV_RGB(200,0,200));
			cvShowImage( "imgBody", outputimg );

			//relaese the resourse and return the rectangle
			cvReleaseImage( &gray );
			cvReleaseImage( &outputimg );
			stepTimeBodyDect = clock();
			flagBodyDetected=1;
			rectReturn=cvRect(topLeftX,topLeftY, botRightX-topLeftX,botRightY-topLeftY);
			return rectReturn;
		}	
    }
	cvPutText(outputimg, "Body Not Detected..." , cvPoint(outputimg->width/2-outputimg->width*0.4, outputimg->height/2), &font, CV_RGB(0,200,200));
	cvShowImage( "imgBody", outputimg );
	cvReleaseImage( &outputimg );
	return rectReturn;
}


CvRect detectface_and_draw( IplImage* img, CvRect rectTemp,HWND hWnd)
{
	//initialize local variables
	CvRect rectReturn=rectTemp;
	int i;
	int topLeftX=5;
	int topLeftY=5;
	int botRightX=img->width-5;
	int botRightY=img->height-5;	
	int tempCount=1;
	float m[6];
	double t = (double)cvGetTickCount();
	CvFont font;
	cvInitFont( &font, CV_FONT_VECTOR0,0.5, 0.5, 0, 3, 8);


	int angle = 15;
	flagFeatureDetectedS1=0;
	//image gray is for processing while image imgCpy is used for show.
	IplImage* gray = cvCreateImage( cvSize(rectTemp.width,rectTemp.height), 8, 1 );
	IplImage* imgCpy = cvCreateImage( cvSize(rectTemp.width,rectTemp.height), img->depth, img->nChannels );
	
	//set focus region which will be scaned
	cvSetImageROI(img, rectTemp);
	cvCopy(img,imgCpy,NULL);
	cvResetImageROI(img);
	
	//code about initialise rotaion function
	CvMat M = cvMat (2, 3, CV_32F, m);	
	CvPoint2D32f centerTemp;
	centerTemp.x=float (gray->width/2.0+0.5);  
	centerTemp.y=float (gray->height/2.0+0.5);

	for(tempCount=-4;tempCount<0;tempCount++){

		cvClearMemStorage( storage );
		//rotate the image for more accurate detection
		cvCvtColor(imgCpy, gray, CV_BGR2GRAY );
		cvFlip(gray, gray, 1);
		cvEqualizeHist(gray, gray );

		//but first time needn't rotation
		if(tempCount!=-4)
		{
			cv2DRotationMatrix( centerTemp, angle*tempCount,1, &M);
			cvWarpAffine(gray,gray, &M,CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS,cvScalarAll(0) ); 

		}
		if( cascadefeature )
		{
			//start detection....
			CvSeq* faces = cvHaarDetectObjects( gray, cascadefeature, storage,
                                            1.2, 2, 1/*CV_HAAR_DO_CANNY_PRUNING*/,
                                            cvSize(faceDetectSize, faceDetectSize) ); 
			//if detected the Thing....
			if( faces->total>0)
			{
				t = (double)cvGetTickCount() - t;
			//	printf( "Feature detection time = %gms : \n ", t/((double)cvGetTickFrequency()*1000.) );
				CvRect* r = (CvRect*)cvGetSeqElem( faces, 0);

				//flag this dection and let the ROI for the body keep steady for 2 more seconds;
				flagFeatureDetectedS1=1;
				if (clock()-stepTimeFeatDectS1>2000){				
					stepTimeBodyDect=stepTimeBodyDect+2000;
					stepTimeFeatDectS1=clock();
				}

				//copy the points
				if (r->x + r->width < gray->width)
					topLeftX=gray->width - ( r->x + r->width);
				else
					topLeftX= 0;
				
				if (r->y > 0)
					topLeftY=r->y;
				else
					topLeftY=0;

				if (gray->width - r->x >0)
					botRightX=gray->width - r->x;
				else
					botRightX=gray->width;

				if (r->y + r->height < gray->height)
					botRightY=r->y + r->height;
				else
					botRightY= gray->height;

				//draw the img into a window with note words;
				rectReturn=cvRect(topLeftX,topLeftY, botRightX-topLeftX,botRightY-topLeftY);				
				cvRectangle( imgCpy,cvPoint(topLeftX,topLeftY),cvPoint(botRightX,botRightY), colors[100%8], 2, 8, 0 );
				cvPutText(imgCpy, "Feature Detected ! " , cvPoint(10,10), &font, CV_RGB(200,0,200));			
				cvShowImage( "imgFeature", imgCpy );

				cvReleaseImage( &gray );
				cvReleaseImage( &imgCpy );
			
				//printf("(x,y)=(%d ,%d) ::",rectReturn.x,rectReturn.y);
	
				return rectReturn;
			}
			else{			

				flagFeatureDetectedS1=0;
				
			}
		}

	}

	
	//draw the img into a window with note words;
//	printf("(x,y)=(%d ,%d) and the (x',y')=(%d ,%d)",topLeftX,topLeftY, botRightX,botRightY);	
	cvRectangle( imgCpy,cvPoint(topLeftX,topLeftY),cvPoint(botRightX,botRightY), colors[100%8], 2, 8, 0 );
	cvPutText(imgCpy, "Not Detected ... " , cvPoint(10,10), &font, CV_RGB(0,200,200));
	cvShowImage( "imgFeature", imgCpy );

    cvReleaseImage( &gray );
	cvReleaseImage( &imgCpy );
	
	return rectReturn;
}