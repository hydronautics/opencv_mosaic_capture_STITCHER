//#include <cv.h>
//#include <highgui.h>
//#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>

using namespace std;

const string winName = "camera capture";
const string fileBegin = "Image";
const string fileEnd = ".jpg";
const char ESC = 27;
const char ENTER = 13;

bool volatile waitingForMouseEvents = false;

// snapshot location is 60x100 centimeters
const double ROI_ratio = 3.0/5;

void drawTarget(IplImage* img, int x, int y, int radius)
{
        cvCircle(img,cvPoint(x, y),radius,CV_RGB(250,0,0),1,8);
        cvLine(img, cvPoint(x-radius/2, y-radius/2), cvPoint(x+radius/2, y+radius/2),CV_RGB(250,0,0),1,8);
        cvLine(img, cvPoint(x-radius/2, y+radius/2), cvPoint(x+radius/2, y-radius/2),CV_RGB(250,0,0),1,8);
}

// обработчик событий от мышки
void myMouseCallback( int event, int x, int y, int flags, void* param )
{
	if (waitingForMouseEvents)
	{
		IplImage* img = (IplImage*) param;

		switch( event ){
		case CV_EVENT_MOUSEMOVE:
			break;

		case CV_EVENT_LBUTTONDOWN:
			cout << x << " " << y << endl;
			drawTarget(img, x, y, 10);
			waitingForMouseEvents = false;
			break;

		case CV_EVENT_LBUTTONUP:
			break;
		}
	}
}

int main()
{

	
	CvCapture *capture = cvCreateCameraCapture(CV_CAP_ANY);
	if (!capture){
		cout << "Can't access any camera." << endl;
		exit(1);
	}


	double width = cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_WIDTH);
	double height = cvGetCaptureProperty(capture,CV_CAP_PROP_FRAME_HEIGHT);

	cout << "width: " << width << endl;
	cout << "height: " << height << endl;

    int ROI_height = int(height);
    int ROI_width = int(ROI_ratio*width);
    int ROI_x_offset = int((width - ROI_width)/ 2.0); // ROI is in the middle
    int ROI_y_offset = 0;

    CvRect frame_ROI = cvRect(ROI_x_offset,ROI_y_offset,ROI_width,ROI_height);

	cout << "Press Enter for frame capture and Escape for exit" << endl;

	int counter = 0;

	

	for (;;){

		IplImage *frame = cvQueryFrame(capture);
		cvSetMouseCallback(winName.c_str(),myMouseCallback,(void *) frame);
        cvSetImageROI(frame,frame_ROI);
        // Entire image won't be shown
        // Only ROI will be shown
        cvShowImage(winName.c_str(),frame);
        int c = cvWaitKey(1);
		if (c == ESC)
			break;
		else if (c == ENTER){
            waitingForMouseEvents = true;
			
            while (waitingForMouseEvents){
				cvShowImage(winName.c_str(),frame);
				cvWaitKey(1);
			}
            ostringstream ost;
            ost << fileBegin << counter << fileEnd;
            cvSaveImage(ost.str().c_str(),frame);
            cout << "captured: " << ost.str() << endl;
            ++counter;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
	return 0;
}
