#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>

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

	vector<IplImage*> captured_images;

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

	IplImage *frame_copy = NULL;
	cvNamedWindow(winName.c_str(),CV_WINDOW_AUTOSIZE);
	

	for (;;){

		IplImage *frame = cvQueryFrame(capture);
		
        cvSetImageROI(frame,frame_ROI);
        // Entire image won't be shown
        // Only ROI will be shown
        cvShowImage(winName.c_str(),frame);
        int c = cvWaitKey(1);
		if (c == ESC)
			break;
		else if (c == ENTER){
            waitingForMouseEvents = true;
			frame_copy = cvCloneImage(frame); // memory allocation
			cvSetMouseCallback(winName.c_str(),myMouseCallback,(void *) frame_copy);
            while (waitingForMouseEvents){
				cvShowImage(winName.c_str(),frame);
				cvWaitKey(1);
			}
            ostringstream ost;
            ost << fileBegin << counter << fileEnd;
            cvSaveImage(ost.str().c_str(),frame_copy);
			cvReleaseImage(&frame_copy);
			frame_copy = NULL;
            cout << "captured: " << ost.str() << endl;
            ++counter;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
	return 0;
}
