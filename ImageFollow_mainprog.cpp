// image follow.cpp : Defines the entry point for the console application.


#include "stdafx.h"
#include <highgui.h>

#include <cv.h>
#include<windows.h>

#include<string.h>

#include<conio.h>

#include<iostream>
 
using namespace std;

// Initialize the serial port variable and parameters
HANDLE hPort = CreateFile(TEXT("COM1"), GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
DCB dcb; // Create a DCB struct variable
/*********************Function for Sending Data***********************/

bool writebyte(char* data)

{

DWORD byteswritten;

if (!GetCommState(hPort,&dcb))

{

cout<<"\nSerial port cant b opened\n";

return false;

}

dcb.BaudRate = CBR_19200;  //9600 Baud 
dcb.ByteSize = 8;	//8 data bits
dcb.Parity = NOPARITY;    //no parity 
dcb.StopBits = ONESTOPBIT; //1 stop

int retVal=0;

if (SetCommState(hPort,&dcb)) //If Com port cannot be configured accordingly return false;
	retVal = WriteFile(hPort,data,1,&byteswritten,NULL); //Write the data to be sent to Serial port 
if (retVal==0)
	return false;
else
	return true;

// return true if the data is written
}
int centre(CvPoint* at ,int n) // calculates and returns centroid of the figure
{
	int x=0,i;
	for(i=0;i<n;i++)
	{
		x+=at[i].x;
	}
	
	return (x/n);
}

IplImage* thatonly(IplImage* m1,IplImage* m2,IplImage* m3,int contrast) // returns the image containing only one particular channel(m3)
{
	cvAdd(m1, m2, m2);
        cvSub(m3, m2, m3);
	cvScale(m3, m3,contrast ); // increasing contrast
	cvDilate(m3, m3, 0,1);
	cvErode(m3,m3, 0, 1);
	return m3;
}


IplImage* threshImg(IplImage* img,int param1) // performs thresholding depending upon the value of the 'param1' parameter
{
	IplImage* imghsv = cvCreateImage(cvGetSize(img),8,1);
	cvThreshold(img,imghsv,param1,255,CV_THRESH_BINARY);
	return imghsv;
}



int main()
{
	CvCapture* capture = 0;
	IplImage* img=0,* blue=0,*bonly=0 ,*red=0,*green=0;
	IplImage *imgthresh=0;
	CvPoint *pt[4];
	char data = 'f';
	int x=0,i=0,c=0,thresh=60,min=4000,max=7000,contrast=2,area,center,dir=0;
	area= (min+max)/2;
	CvSeq *contours,*result;
    CvMemStorage *storage = cvCreateMemStorage(0);
	
	capture = cvCaptureFromCAM(1);
	if(!capture)
	{
		printf("Could not initialize capturing....\n");
		return -1;
	}

	cvNamedWindow("Original");
	cvNamedWindow("Channel");
	cvNamedWindow("Threshold");
	cvCreateTrackbar("Thresh","Threshold",&thresh,255,NULL);
	cvCreateTrackbar("Contrast","Threshold",&contrast,40,NULL);
    cvCreateTrackbar("MIN AREA","Original",&min,1000000,NULL);
	cvCreateTrackbar("MAX AREA","Original",&max,9000000,NULL);
	cvCreateTrackbar("Direction","Channel",&dir,1,NULL);
		
	while(true)
	{
		
		img=cvQueryFrame(capture);
		if(!img)
			break;
	center=img->width/2;	
	area= (min+max)/2;
	cvShowImage("Original",img);
	red=cvCreateImage(cvGetSize(img), 8, 1);
	green=cvCreateImage(cvGetSize(img), 8, 1);
	blue=cvCreateImage(cvGetSize(img), 8, 1);
	bonly=cvCreateImage(cvGetSize(img), 8, 1);
		
		cvSplit(img,blue, green, red, NULL);
		blue=thatonly(green,red,blue,contrast);
		bonly=cvCloneImage(blue);
				
		cvReleaseImage(&red); // releasing unused images
	    cvReleaseImage(&green);
	    cvReleaseImage(&blue);
					
		imgthresh = threshImg(bonly,thresh);
	
		cvShowImage("Threshold",imgthresh);
		cvShowImage("Channel",bonly);
				
			cvFindContours(imgthresh, storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	
	while(contours)
    {
         // approximating contours boundaries to get rid of torned edges
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP,cvContourPerimeter(contours)*0.05, 0);//0.03 initial
	      x=result->total ; // calculating the total no of edges in a approximated contour(blob)
                // if x==4 then its a square and fabs()-> gives abolute value and with cvContourArea we are setting the min and max blob size to be detected in image 
		
		if(x==4 && fabs(cvContourArea(result,CV_WHOLE_SEQ))>100 && cvCheckContourConvexity(result))
		{ 
			area = fabs(cvContourArea(result,CV_WHOLE_SEQ));
		 //   CvPoint* at= (CvPoint*)malloc(x*sizeof(CvPoint));//http://stackoverflow.com/questions/260915/how-can-i-create-a-dynamically-sized-array-of-structs
			                        // loop to store the coordinates of the edges of the detected contour and storing then in CvPoint structure
		//	CvPoint *pt[4];
			for( i=0 ; i<x;i++)
			{
				pt[i]=(CvPoint*)cvGetSeqElem(result,i);
				//printf("Point  x = %d   y= % d \n",pt->x,pt->y);
			//	at[i]=cvPoint(pt->x,pt->y);
			
			}
			
                        // loop to draw boundary around the contour
			for(i=0;i<x-1;i++)
				cvLine(img,*pt[i],*pt[i+1],cvScalar(0,0,255),2,4,0);
			cvLine(img,*pt[i],*pt[0],cvScalar(0,0,255),2,4,0);
			center=centre(*pt,x);
			
			//free(at);
			printf("\nArea -  %d", area);
		}
		
					
		contours = contours->h_next;
	}
	//printf(" final Area %d",area);
	if(dir==0)
	{
		if (area>max)
		{
			data='b';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		if(area<min)
		{
			data='f';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		if(area>min && area<max)
		{
			data='n';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
	}
	if(dir==1)
	{
		if(center>(img->width/2)+50)
		{
			data='l';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		if(center<(img->width/2)-50)
		{
			data='r';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
		if(center<(img->width/2)+50 && center>(img->width/2)-50 )
		{
			data='n';
			if(writebyte(&data))
				printf("\n%c ",data);
		}
	}

	cvShowImage("Original",img);
	c = cvWaitKey(20); // waiting for 20 milli seconds
		if(c==27)  // detecting esc key press
			break;
                 // doing some cleaning up
		
		cvReleaseImage(&bonly);
		
		cvReleaseImage(&imgthresh);
		
	}

	data='n';
	if(writebyte(&data))
		printf("\n%c ",data);
	
	cvReleaseMemStorage(&storage);
	
	cvReleaseCapture(&capture);
	return 0;
}




