//Vaibhav Devekar

//OpenCV Headers
#include<cv.h>
#include<highgui.h>
//Blob Library Headers
#include<cvblob.h>

//Input-Output
#include<stdio.h>
//Windows Headers
#include<windows.h>

//Definitions
#define h 240
#define w 320

//NameSpaces
using namespace cvb;
using namespace std;

void traceMouse(CvBlobs &blobs,int screenx,int screeny);
void clickLeft(CvBlobs &blobs);
void clickRight(CvBlobs &blobs);


void main()
{
	//Structure to get feed from CAM
	CvCapture* capture=cvCreateCameraCapture(0);

	//Structure to hold blobs
	CvBlobs blobs,blobsL,blobsR;

	//Windows
	cvNamedWindow("Live",CV_WINDOW_AUTOSIZE);
	cvNamedWindow("Thresh",CV_WINDOW_AUTOSIZE);

	//Image Variables
	IplImage *frame=cvCreateImage(cvSize(w,h),8,3); //Original Image
	IplImage *hsvframe=cvCreateImage(cvSize(w,h),8,3);//Image in HSV color space

	//Image Variable for blobs
	IplImage *labelImg=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);
	IplImage *labelImgL=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);
	IplImage *labelImgR=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);

	//Image variables for color-threshholded images
	IplImage *threshy=cvCreateImage(cvSize(w,h),8,1); 
	IplImage *threshL=cvCreateImage(cvSize(w,h),8,1); 
	IplImage *threshR=cvCreateImage(cvSize(w,h),8,1); 

	//Getting the screen information
	int screenx = GetSystemMetrics(SM_CXSCREEN);
	int screeny = GetSystemMetrics(SM_CYSCREEN);

	//blue for position of mouse cursor
	int h1=100;int s1=140;int v1=127;
	int h2=128;int s2=255;int v2=255;

	//yellow for left click
	int h3=25;int s3=89;int v3=204;
	int h4=40;int s4=255;int v4=255;

	//red for right click
	int h5=170;int s5=89;int v5=153;
	int h6=180;int s6=255;int v6=255;

	while(1)
	{
		//Getting the current frame
		IplImage *fram=cvQueryFrame(capture);
		if(!fram) break;

		//Resizing the capture
		cvResize(fram,frame,CV_INTER_LINEAR );
		//Flipping the frame
		cvFlip(frame,frame,1);
		//Changing the color space
		cvCvtColor(frame,hsvframe,CV_BGR2HSV);


		//Thresholding the frame for color 1
		cvInRangeS(hsvframe,cvScalar(h1,s1,v1),cvScalar(h2,s2,v2),threshy);
		//Thresholding the frame for color 2
		cvInRangeS(hsvframe, cvScalar(h3,s3,v3), cvScalar(h4,s4,v4), threshL);
		//Thresholding the frame for color 3
		cvInRangeS(hsvframe, cvScalar(h5,s5,v5), cvScalar(h6,s6,v6), threshR);

		//Filtering the frame
		cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
		cvSmooth(threshL,threshL,CV_MEDIAN,7,7);
		cvSmooth(threshR,threshR,CV_MEDIAN,7,7);

		//Finding the blobs
		unsigned int result=cvLabel(threshy,labelImg,blobs);
		unsigned int resultL=cvLabel(threshL,labelImgL,blobsL);
		unsigned int resultR=cvLabel(threshR,labelImgR,blobsR);

		//Rendering the blobs
		cvRenderBlobs(labelImg,blobs,frame,frame);
		cvRenderBlobs(labelImgL,blobsL,frame,frame);
		cvRenderBlobs(labelImgR,blobsR,frame,frame);

		traceMouse(blobs,screenx,screeny);
		clickLeft(blobsL);
		clickRight(blobsR);

		//Combine thresholded images
		cvAdd(threshy, threshL, threshy);
		cvAdd(threshy, threshR, threshy);

		//Showing the images
		cvShowImage("Thresh",threshy);
		cvShowImage("Live",frame);

		//Escape Sequence
		char c=cvWaitKey(33);
		if(c==27) break;
	}

	//Cleanup
	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
}



void traceMouse(CvBlobs &blobs,int screenx,int screeny)
{
	//Filtering the blobs
	cvFilterByArea(blobs,30,500);
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;
		//Variable for holding position
		int x1;
		int y1;
		//Calculating the current position
		x1 = moment10/area;
		y1 = moment01/area;
		//Mapping to the screen coordinates
		int x=(int)(x1*screenx/w);
		int y=(int)(y1*screeny/h);
		//Printing the position information
		cout<<"X: "<<x<<" Y: "<<y<<endl;
		//Moving the mouse pointer
		SetCursorPos(x,y);
	}

}


void clickLeft(CvBlobs &blobs)
{
	static bool prevDown=false;

	//Filtering the blobs
	cvFilterByArea(blobs,30,500);
	bool flag=false;
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;

		static int posX = 0;
		static int posY = 0;

		int lastX = posX;
		int lastY = posY;

		posX = moment10/area;
		posY = moment01/area;

		if(lastX>0 && lastY>0 && posX>0 && posY>0)
			flag=true;

	}

	if (flag) {cout<<"Left Down\n";mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); prevDown=true;}
	else if(prevDown) {mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);prevDown=false;} 

}


void clickRight(CvBlobs &blobs)
{
	static bool prevUp=false;

	//Filtering the blobs
	cvFilterByArea(blobs,30,500);
	bool flag=false;
	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;

		static int posX = 0;
		static int posY = 0;

		int lastX = posX;
		int lastY = posY; 

		posX = moment10/area;
		posY = moment01/area;

		if(lastX>0 && lastY>0 && posX>0 && posY>0)
			flag=true;

	}

	if (flag) {cout<<"Right Down\n";mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, 0); prevUp=true;}
	else {mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, 0);prevUp=false; }
}