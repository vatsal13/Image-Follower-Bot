// Angle_origin Shift.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <highgui.h>

#include <cv.h>
#include "math.h"
CvPoint circle[2]; // global variable to store centre of circular tags on bot
CvPoint nr_sq;     // global variable to store centre of nearest square
CvPoint new_Cord[3]; //0 -> red 1-> blue 3 -> nearest cube

double angle_bot=0.0 , angle_cube=0.0 , angle_diff=0.0;

void bot_rotation()
{
	angle_diff = angle_bot - angle_cube;
	if(fabs(angle_diff) < 181 )
	{
		if( angle_diff >0 )
			printf("CW");
		if(angle_diff <0)
			printf("CCW");
	}
	if(fabs(angle_diff) > 180 )
	{
		if( angle_diff >0 )
			printf("CCW");
		if(angle_diff <0)
			printf("CW");
	}
}

void findangle(IplImage *img)     // finds angle b/w the square center and the vector of bot orientation
{
	 double angle = 10.0;
	int x=0;
	circle[0].y *= -1;
	circle[1].y *= -1;
	nr_sq.y *= -1;
	new_Cord[0].x = circle[0].x - circle[1].x ;
	new_Cord[0].y = circle[0].y - circle[1].y ;
	new_Cord[1].x = 0;
	new_Cord[1].y = 0;
	new_Cord[2].x = nr_sq.x - circle[1].x ;
	new_Cord[2].y = nr_sq.y - circle[1].y ;
	printf("\nnew_Cord[0].x- %d      new_Cord[0].y- %d \n",new_Cord[0].x,new_Cord[0].y); 
	printf("\nnew_Cord[1].x- %d      new_Cord[1].y- %d \n",new_Cord[1].x,new_Cord[1].y); 
	printf("\nnew_Cord[2].x- %d      new_Cord[2].y- %d \n",new_Cord[2].x,new_Cord[2].y); 

    for(x=0;x<=2;x++)
	{
		if(x==1)
			continue;
		if(new_Cord[x].x==0 && new_Cord[x].y > 0)
		{
			angle= 90.0;
			goto a1;
		}
		if(new_Cord[x].x==0 && new_Cord[x].y < 0)
		{
			angle= 270.0;
			goto a1;
		}
		if(new_Cord[x].y==0 && new_Cord[x].x == 0 )//|| new_Cord[x].x > 0)
		{
			angle= 0.0;
			goto a1;
			//continue;
		}
		//this was obvious but still .....
		if(new_Cord[x].y==0 && new_Cord[x].x > 0)
		{
			angle = 0.0;
			goto a1;
			//continue;
		}

		angle = atan((double)((double)new_Cord[x].y / (double)new_Cord[x].x));
	    angle = angle * (180/3.146);
		//printf("\n Angle - %f\n",angle);
		if(new_Cord[x].y==0 && new_Cord[x].x < 0)
		{
			angle = 180.0;
			goto a1;
		}
	/*	//this was obvious but still .....
		if(new_Cord[x].y==0 && new_Cord[x].x > 0)
		{
		//	angle = 0.0;
		//	goto a1;
			continue;
		}
		*/
		if(new_Cord[x].y > 0 && new_Cord[x].x < 0)
		{
			angle += 180.0;
			goto a1;
		}
		if(new_Cord[x].y < 0 && new_Cord[x].x < 0)
		{
			angle += 180.0;
			goto a1;
		}
		if(new_Cord[x].y < 0 && new_Cord[x].x > 0)
		{
			angle += 360.0;
			goto a1;
		}
a1: if(x==0)
		angle_bot = angle;
	if(x==2)
		angle_cube = angle;
	}
	
	/*if(new_Cord[0].x != 0)
	{
	angle = atan((double)((double)new_Cord[0].y / (double)new_Cord[0].x));
	    angle = angle * (180/3.146);
		printf("\n Angle - %f\n",angle);
	}
	*/

	circle[0].y *= -1;
	circle[1].y *= -1;
	nr_sq.y *= -1;
	// draw lines to join the centres of circles(vector) to see bot orientation
	cvLine(img,circle[0],circle[1],CV_RGB(255,0,0),3,4,0);
        // draw line b/w center of red circle to sq center
	cvLine(img,circle[1],nr_sq,CV_RGB(255,0,0),3,4,0);
	
	// calculates and prints the angle between the two lines
	//angle=fabs(atan((double)(nr_sq.y-circle[0].y)/(nr_sq.x-circle[0].x))-atan((double)(circle[1].y-circle[0].y)/(circle[1].x-circle[0].x)));
	printf("\n angle_bot - %f\n angle_cube - %f \n",angle_bot,angle_cube);
	bot_rotation();
	
}
	


IplImage* thatonly(IplImage* m1,IplImage* m2,IplImage* m3,int contrast) // returns the image containing only one particular channel(m3)
{
	cvAdd(m1, m2, m2);
        cvSub(m3, m2, m3);
	cvScale(m3, m3,contrast ); // increasing contrast
	cvDilate(m3, m3, 0,1);
	cvErode(m3,m3, 0, 1);
	//cvReleaseImage(&m1);
	//cvReleaseImage(&m2);
	return m3;
}

CvPoint centre(CvPoint* at ,int n) // calculates and returns centroid of the figure
{
	int x=0,y=0,i;
	for(i=0;i<n;i++)
	{
		x+=at[i].x;
		y+=at[i].y;
	}
	CvPoint p=cvPoint(x/n,y/n);
	return p;
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
	IplImage* img=0,* red=0,* green=0,* blue=0,*ronly=0,*gonly=0,*bonly=0 ;
	//IplImage* imgthresh=0 ;
	//IplImage* red=0;
	//IplImage* green=0;
	//IplImage* blue=0;
	IplImage *imgthresh[3];
	imgthresh[0]=0;
	imgthresh[1]=0;
	imgthresh[2]=0;
	CvPoint *pt;
	int x=0,i=0,c=0,thresh=10,rthresh=10,gthresh=10,bthresh=10,min=100,max=10000,contrast=4,rcont=4,gcont=4,bcont=4,ch=0,prevch=0,n=0;
	//double sum=0;
	CvSeq *contours,*result;
        //  CvSeq* result;
        CvMemStorage *storage = cvCreateMemStorage(0);
	
	capture = cvCaptureFromCAM(1);
	if(!capture)
	{
		printf("Could not initialize capturing....\n");
		return -1;
	}

	cvNamedWindow("Original");
	cvNamedWindow("Channel");
	//cvNamedWindow("Green");
	//cvNamedWindow("Blue");
	cvNamedWindow("Threshold");
	cvCreateTrackbar("Ch_Select","Channel",&ch,2,NULL);
	cvCreateTrackbar("Thresh","Threshold",&thresh,255,NULL);
	//cvCreateTrackbar("Threshold","Green",&pg,60,NULL);
	//cvCreateTrackbar("Threshold","Blue",&pb,60,NULL);
	cvCreateTrackbar("Contrast","Threshold",&contrast,40,NULL);
	//cvCreateTrackbar("Contrast","Green",&gcont,20,NULL);
	//cvCreateTrackbar("Contrast","Blue",&bcont,20,NULL);
        cvCreateTrackbar("MIN AREA","Original",&min,1000000,NULL);
	cvCreateTrackbar("MAX AREA","Original",&max,9000000,NULL);
	printf("0->RED\n1->GREEN\n2->BLUE");
		
	while(true)
	{
		
		img=cvQueryFrame(capture);
		if(!img)
			break;
		//sum = cvGetCaptureProperty(capture,CV_CAP_PROP_FPS);
		//printf("\n FPS - %f  ",sum);
		cvShowImage("Original",img);
		red=cvCreateImage(cvGetSize(img), 8, 1);
	        green=cvCreateImage(cvGetSize(img), 8, 1);
	        blue=cvCreateImage(cvGetSize(img), 8, 1);
		//ronly=cvCreateImage(cvGetSize(img), 8, 1);
		//gonly=cvCreateImage(cvGetSize(img), 8, 1);
		//bonly=cvCreateImage(cvGetSize(img), 8, 1);
		if(prevch==ch) // to know whether slider for channel select has moved or not
		{
		switch(ch) // if not then update the specific channel related variables
		{
		case 0:
			rcont=contrast;
			rthresh=thresh;
			break;
		case 1:
			gcont=contrast;
			gthresh=thresh;
			break;
		case 2:
			bcont=contrast;
			bthresh=thresh;
			break;
		}
		}
		else  // if moved then show the new channel's variable values 
		{
			prevch=ch;
			switch(ch)
		{
		case 0:
			contrast=rcont;
			thresh=rthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		case 1:
			contrast=gcont;
			thresh=gthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		case 2:
			contrast=bcont;
			thresh=bthresh;
			cvSetTrackbarPos("Thresh","Threshold",thresh);
			cvSetTrackbarPos("Contrast","Threshold",contrast);
			break;
		}

		}



	
		cvSplit(img,blue, green, red, NULL);
		red=thatonly(blue,green,red,rcont);
		ronly=cvCloneImage(red);             // cvClone() func is used and ronly=red is not done because then we are just refering to   same image and when next cvSplit() executes it creates a problem...... becoz red and ronly are pointers..
		//cvShowImage("Red",ronly);


		cvSplit(img,blue, green, red, NULL);
		blue=thatonly(green,red,blue,bcont);
		bonly=cvCloneImage(blue);
		//cvShowImage("Blue",bonly);

		cvSplit(img,blue, green, red, NULL);
		green=thatonly(red,blue,green,gcont);
		gonly=cvCloneImage(green);
		//cvShowImage("Green",gonly);
		
		cvReleaseImage(&red); // releasing unused images
	        cvReleaseImage(&green);
	        cvReleaseImage(&blue);

		imgthresh[0] = threshImg(ronly,rthresh);  // thresholding each channel depending upon the value set by the slider
		imgthresh[1] = threshImg(gonly,gthresh);
		imgthresh[2] = threshImg(bonly,bthresh);
		//cvCanny(imgthresh[0],imgthresh[0],10,30,3);
		//cvCanny(imgthresh[1],imgthresh[1],10,30,3);
		//cvCanny(imgthresh[2],imgthresh[2],10,30,3);
		
		switch(ch) // this helps to display correct images on the windows depending upon the channel select slider value
		{
		case 0:
			cvShowImage("Threshold",imgthresh[0]);
			cvShowImage("Channel",ronly);
			break;
		case 1:
			cvShowImage("Threshold",imgthresh[1]);
			cvShowImage("Channel",gonly);
			break;
		case 2:
			cvShowImage("Threshold",imgthresh[2]);
			cvShowImage("Channel",bonly);
			break;
		}
		//imgthresh=red;
		for(n=0;n<=2;n++) // loop to find contours in red green blue channels
		{
			
			cvFindContours(imgthresh[n], storage, &contours, sizeof(CvContour), CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0,0));
	
	while(contours)
    {
         // approximating contours boundaries to get rid of torned edges
        result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP,cvContourPerimeter(contours)*0.03, 0);
		x=result->total ; // calculating the total no of edges in a approximated contour(blob)
                // if x==4 then its a square and fabs()-> gives abolute value and with cvContourArea we are setting the min and max blob size to be detected in image 
		if(x==4 && fabs(cvContourArea(result,CV_WHOLE_SEQ))>min && fabs(cvContourArea(result,CV_WHOLE_SEQ))<max && cvCheckContourConvexity(result))
		{ 
			
			CvPoint* at= (CvPoint*)malloc(x*sizeof(CvPoint));//http://stackoverflow.com/questions/260915/how-can-i-create-a-dynamically-sized-array-of-structs
			// printing total no of edge detected in the blob
			printf("Result %d",result->total);
                        // loop to store the coordinates of the edges of the detected contour and storing then in CvPoint structure
			for( i=0 ; i<x;i++)
			{
				pt=(CvPoint*)cvGetSeqElem(result,i);
				//printf("Point  x = %d   y= % d \n",pt->x,pt->y);
				at[i]=cvPoint(pt->x,pt->y);
			
			}
			
                        // loop to draw boundary around the contour
			for(i=0;i<x-1;i++)
				cvLine(img,at[i],at[i+1],cvScalar(255),1,4,0);
			cvLine(img,at[i],at[0],cvScalar(255),1,4,0);
			cvCircle(img,centre(at,x),3,CV_RGB(0,255,0),-1,8,0);
			if(n==2)// to store the centroid of square contour in red channel
			{
				nr_sq=centre(at,x);
			}
			free(at);
		}

		if(n!=1 )// entering only when red and blue channel is thresholded( basically i do not want to detect green colour circle, so i wont enter into the loop)
		{
			
                // edges greater than 6 means its a circle like figure and definetly not a square
		if(x>6 && fabs(cvContourArea(result,CV_WHOLE_SEQ))>min && fabs(cvContourArea(result,CV_WHOLE_SEQ))<max && cvCheckContourConvexity(result))
		{ 
			
			CvPoint* at= (CvPoint*)malloc(x*sizeof(CvPoint));//http://stackoverflow.com/questions/260915/how-can-i-create-a-dynamically-sized-array-of-structs
			
			printf("Result %d",result->total);
			for( i=0 ; i<x;i++)
			{
				pt=(CvPoint*)cvGetSeqElem(result,i);
			//	printf("Point  x = %d   y= % d \n",pt->x,pt->y);
				at[i]=cvPoint(pt->x,pt->y);
			}
			for(i=0;i<x-1;i++)
				cvLine(img,at[i],at[i+1],cvScalar(255),3,4,0);
			cvLine(img,at[i],at[0],cvScalar(255),3,4,0);
			cvCircle(img,centre(at,x),3,CV_RGB(0,255,0),-1,8,0);
			circle[n]=centre(at,x);
			if(n==2)
				circle[1]=centre(at,x);
			free(at);
		}
		}
		contours = contours->h_next;
	}
		}
		findangle(img);
	cvShowImage("Original",img);
	c = cvWaitKey(20); // waiting for 20 milli seconds
		if(c==27)  // detecting esc key press
			break;
                 // doing some cleaning up
		cvReleaseImage(&ronly);
		cvReleaseImage(&gonly);
		cvReleaseImage(&bonly);
		cvReleaseImage(&imgthresh[0]);
		cvReleaseImage(&imgthresh[1]);
		cvReleaseImage(&imgthresh[2]);
		//cvClearSeq(result);
		//cvClearSeq(contours);
		//cvReleaseMemStorage(&contours->storage);
		
	}

	
	//cvReleaseImage(&img);
	cvReleaseMemStorage(&storage);
	//cvClearSeq(contours);
	
	cvReleaseCapture(&capture);
	//cvClearSeq(contours);
	return 0;
}

