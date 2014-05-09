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

	cout << "Press Enter for frame capture and Escape for exit" << endl;

	int counter = 0;

	for (;;){
		IplImage *frame = cvQueryFrame(capture);
        cvShowImage(winName.c_str(),frame);
        int c = cvWaitKey(1);
		if (c == ESC)
			break;
		else if (c == ENTER){

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
