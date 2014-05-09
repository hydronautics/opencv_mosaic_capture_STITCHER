#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>

using namespace std;

const string winName = "camera capture";
const string fileBegin = "Image";
const string fileEnd = ".jpg";
const char ESC = 27;
const char ENTER = 13;

bool volatile waitingForMouseEvents = false;

struct CapturedImage { 

	IplImage* img;
	string windowName;
	CvPoint point;

	CapturedImage(IplImage* i = NULL, const string& wn = "",CvPoint p = cvPoint(0,0)):
		img(i == NULL ? NULL : cvCloneImage(i)),
		windowName(wn),
		point(p)
	{}

	CapturedImage(const CapturedImage& src):
		img(cvCloneImage(src.img)),
		windowName(src.windowName),
		point(src.point)
	{}
	~CapturedImage(){ cvReleaseImage(&img); img = NULL;}

};

vector<CapturedImage> caps;
// snapshot location is 60x100 centimeters
const double ROI_ratio = 3.0/5;

void drawTarget(IplImage* img, int x, int y, int radius)
{
        cvCircle(img,cvPoint(x, y),radius,CV_RGB(250,0,0),1,8);
        cvLine(img, cvPoint(x-radius/2, y-radius/2), cvPoint(x+radius/2, y+radius/2),CV_RGB(250,0,0),1,8);
        cvLine(img, cvPoint(x-radius/2, y+radius/2), cvPoint(x+radius/2, y-radius/2),CV_RGB(250,0,0),1,8);
}

// обработчик событий от мышки
void myMouseCallback( int event, int x, int y, int, void* param )
{
	if (waitingForMouseEvents)
	{
		IplImage* img = (IplImage*) param;

		switch( event ){
		case CV_EVENT_MOUSEMOVE:
			break;

		case CV_EVENT_LBUTTONDOWN:
			{
				cout << x << " " << y << endl;
				drawTarget(img, x, y, 10);
				string window_name;
				// windows will be consequently enumerated from A
				window_name += 'A' + char(caps.size()); 
				caps.push_back(CapturedImage(img,window_name,cvPoint(x,y)));
				waitingForMouseEvents = false;
			}
			break;

		case CV_EVENT_LBUTTONUP:
			break;
		}
	}
}

int main()
try {

	
	CvCapture *capture = cvCreateCameraCapture(CV_CAP_ANY);
	if (!capture){
		throw runtime_error("Can't access any camera.");
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

	cvNamedWindow(winName.c_str(),CV_WINDOW_AUTOSIZE);
	

	for (;;){

		IplImage *frame = cvQueryFrame(capture);
		if (!frame) throw runtime_error("Failed to query frame");
        cvSetImageROI(frame,frame_ROI);
        // Entire image won't be shown
        // Only ROI will be shown
        cvShowImage(winName.c_str(),frame);
        int c = cvWaitKey(1);
		if (c == ESC)
			break;
		else if (c == ENTER){
            waitingForMouseEvents = true;
			cvSetMouseCallback(winName.c_str(),myMouseCallback,(void *) frame);
            while (waitingForMouseEvents){
				cvShowImage(winName.c_str(),frame);
				cvWaitKey(1);
			}
            ostringstream ost;
            ost << fileBegin << counter << fileEnd;
            cvSaveImage(ost.str().c_str(),frame);

            cout << "captured: " << ost.str() << endl;
			for (vector<CapturedImage>::size_type i = 0; i < caps.size(); ++i){
				CapturedImage& this_img = caps.at(i);
				cvShowImage(this_img.windowName.c_str(),this_img.img);
			
			}

            ++counter;
		}
	}
	cvReleaseCapture(&capture);
	cvDestroyAllWindows();
	return 0;
}
catch (exception& e){
	cerr << e.what() << endl;
	cerr << "Press Enter to quit" << endl;
	cin.ignore(1);
	return 1;
}
