//Vaibhav Devekar
//Use of flick to change picture and pinch gesture for zoom in-out and moving image.
//Blue for flick, blue and yellow for pinch gesture, only yellow to drag image across screen. Blue also tracks cursor position.

//OpenCV Headers
#include<cv.h>
#include<highgui.h>
//Input-Output
#include<stdio.h>
//Blob Library Headers
#include<cvblob.h>
//Windows Headers
#include<windows.h>
#include<math.h>

//Definitions
#define h 240
#define w 320
//NameSpaces
using namespace cvb;
using namespace std;

void detectFlick(CvBlobs &blobs,int screenx,int screeny);
void detectZoom(CvBlobs &blobs,CvBlobs &blobsL,int screenx,int screeny);
void moveImage(CvBlobs &blobsL,int screenx,int screeny);

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

	IplImage *labelImg=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);//Image Variable for blobs
	IplImage *labelImgL=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);
	//IplImage *labelImgR=cvCreateImage(cvSize(w,h),IPL_DEPTH_LABEL,1);

	IplImage *threshy=cvCreateImage(cvSize(w,h),8,1); //Threshold image of yellow color
	IplImage *threshL=cvCreateImage(cvSize(w,h),8,1); // find red dot 
	//IplImage *threshR=cvCreateImage(cvSize(w,h),8,1); // find red dot 

	//Getting the screen information
	int screenx = GetSystemMetrics(SM_CXSCREEN);
	int screeny = GetSystemMetrics(SM_CYSCREEN);

	//blue
	int h1=100;int s1=140;int v1=127;
	int h2=128;int s2=255;int v2=255;

	//yellow
	int h3=25;int s3=89;int v3=204;
	int h4=40;int s4=255;int v4=255;

	//red
	int h5=170;int s5=89;int v5=153;
	int h6=180;int s6=255;int v6=255;

	while(1)
	{
		//Getting the current frame
		IplImage *fram=cvQueryFrame(capture);
		//If failed to get break the loop
		if(!fram)
			break;
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
		//cvInRangeS(hsvframe, cvScalar(h5,s5,v5), cvScalar(h6,s6,v6), threshR);

		//Filtering the frame
		cvSmooth(threshy,threshy,CV_MEDIAN,7,7);
		cvSmooth(threshL,threshL,CV_MEDIAN,7,7);
		//cvSmooth(threshR,threshR,CV_MEDIAN,7,7);

		//Finding the blobs
		unsigned int result=cvLabel(threshy,labelImg,blobs);
		unsigned int resultL=cvLabel(threshL,labelImgL,blobsL);
		//unsigned int resultR=cvLabel(threshR,labelImgR,blobsR);

		//Rendering the blobs
		cvRenderBlobs(labelImg,blobs,frame,frame);
		cvRenderBlobs(labelImgL,blobsL,frame,frame);
		//cvRenderBlobs(labelImgR,blobsR,frame,frame);

		//Filtering the blobs
		cvFilterByArea(blobs,20,500);
		cvFilterByArea(blobsL,20,500);

		if(blobsL.size()==0) detectFlick(blobs,screenx,screeny);
		else if(blobs.size()==0) moveImage(blobsL,screenx,screeny); 
		else detectZoom(blobs,blobsL,screenx,screeny);
		
		cvAdd(threshy, threshL, threshy);
		
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



void detectFlick(CvBlobs &blobs,int screenx,int screeny)
{
	static int x2=0,y2=0;
	static double timestamp2,last_change=0;
	double timestamp = (double)clock()/CLOCKS_PER_SEC;
	double vel=0;

	int x=0,y=0,x1,y1,xt,yt;

	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);


	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		double moment10 = it->second->m10;
		double moment01 = it->second->m01;
		double area = it->second->area;

		//Calculating the current position
		x1 = moment10/area;
		y1 = moment01/area;


		//Mapping to the screen coordinates
		xt=(int)(x1*screenx/w);
		yt=(int)(y1*screeny/h);
		if(xt>0 && yt>0) {x=xt;y=yt;}
		SetCursorPos(xt,yt);
	}

	vel=(x-x2)/(timestamp-timestamp2);

	//Printing the position information
	//cout<<"X: "<<x<<" Y: "<<y<<" Time: "<<timestamp<<" Vel: "<<vel<<endl;




	if(x>0 && x2>0) {
		if(x-x2>60 && vel>500 && timestamp>last_change+0.5){
			keybd_event(VK_RIGHT,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_RIGHT,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);

			cout<<"RIGHT"<<endl;
			cout<<"X: "<<x<<" Y: "<<y<<" Time: "<<timestamp<<" Vel: "<<vel<<endl;
			last_change=timestamp;
		}
		else if(x2-x>60 && -vel>500 && timestamp>last_change+0.5){
			keybd_event(VK_LEFT,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(VK_LEFT,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);

			cout<<"LEFT"<<endl;
			cout<<"X: "<<x<<" Y: "<<y<<" Time: "<<timestamp<<" Vel: "<<vel<<endl;
			last_change=timestamp;
		}
	}

	x2=x; y2=y; timestamp2=timestamp;
}


void detectZoom(CvBlobs &blobs,CvBlobs &blobsL,int screenx,int screeny)
{
	static double dist2=0,moment10,moment01,area;
	double dist1=0;
	int x1=0,x2=0,y1=0,y2=0;
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);

	for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it)
	{
		moment10 = it->second->m10;
		moment01 = it->second->m01;
		area = it->second->area;
		x1 = moment10/area;


		y1 = moment01/area;

		int x=(int)(x1*screenx/w);
		int y=(int)(y1*screeny/h);
		SetCursorPos(x,y);
	}

	for (CvBlobs::const_iterator it=blobsL.begin(); it!=blobsL.end(); ++it)
	{
		moment10 = it->second->m10;
		moment01 = it->second->m01;
		area = it->second->area;

		x2 = moment10/area;
		y2 = moment01/area;
	}

	if(x1>0 && x2>0){
		dist1=sqrt((double)(x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
		if(dist1>dist2+5) {
			keybd_event(187,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(187,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
			cout<<"Plus Key\n";
		}
		else if(dist2>dist1+5) {
			keybd_event(189,0,KEYEVENTF_EXTENDEDKEY|0,0);
			keybd_event(189,0,KEYEVENTF_EXTENDEDKEY|KEYEVENTF_KEYUP,0);
			cout<<"Minus Key\n";
		}
		dist2=dist1;
	}

}



void moveImage(CvBlobs &blobsL,int screenx,int screeny)
{
	static double moment10,moment01,area;
	double dist1=0;
	int x=0,y=0;

	for (CvBlobs::const_iterator it=blobsL.begin(); it!=blobsL.end(); ++it)
	{
		moment10 = it->second->m10;
		moment01 = it->second->m01;
		area = it->second->area;

		x = moment10/area;
		y = moment01/area;

		x=(int)(x*screenx/w);
		y=(int)(y*screeny/h);

		if(x>0 && y>0){
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			SetCursorPos(x,y);
			
		}
	}

}


