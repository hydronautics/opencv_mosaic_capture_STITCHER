#include <opencv2/opencv.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <windows.h>
using namespace std;

const string winName = "camera capture";
const string fileBegin = "Image";
const string fileEnd = ".jpg";
const char ESC = 27;
const char ENTER = 13;
const int DEFAULT_CAMERA = 0;
const int EXTERNAL_CAMERA = 1;

bool volatile waitingForMouseEvents = false;

struct CapturedImage { 

	IplImage* img;
	string windowName;
	CvPoint left_point;
	CvPoint right_point;

	CapturedImage(IplImage* i = NULL, const string& wn = "",CvPoint lp = cvPoint(0,0), CvPoint rp = cvPoint(0,0)):
		img(i == NULL ? NULL : cvCloneImage(i)),
		windowName(wn),
		left_point(lp),
		right_point(rp)
	{}

	CapturedImage(const CapturedImage& src):
		img(cvCloneImage(src.img)),
		windowName(src.windowName),
		left_point(src.left_point),
		right_point(src.right_point)
	{}
	~CapturedImage(){ cvReleaseImage(&img); img = NULL;}

};

vector<CapturedImage> caps;
// snapshot location is 60x100 centimeters
const double ROI_ratio = 3.0/5;

typedef enum Target {
	LEFT_TARGET,
	RIGHT_TARGET,
};

void drawTarget(IplImage* img, int x, int y, int radius, Target t){

		CvScalar target_color;
		switch (t){
		case LEFT_TARGET:
			target_color = CV_RGB(255,0,0);
			break;
		case RIGHT_TARGET:
			target_color = CV_RGB(0,0,255);
			break;
		default:
			throw runtime_error("Unexpected target type");
			return;
		}
        cvCircle(img,cvPoint(x, y),radius,target_color,1,8);
        cvLine(img, cvPoint(x-radius/2, y-radius/2), cvPoint(x+radius/2, y+radius/2),target_color,1,8);
        cvLine(img, cvPoint(x-radius/2, y+radius/2), cvPoint(x+radius/2, y-radius/2),target_color,1,8);
}

// обработчик событий от мышки
void myMouseCallback( int event, int x, int y, int, void* param )
{
	static int click_count; // default to 0
	if (waitingForMouseEvents)
	{
		IplImage* img = (IplImage*) param;
		string window_name;

		switch( event ){
		case CV_EVENT_LBUTTONDOWN:
			cout << x << " " << y << endl;
			switch (click_count)
			{
			case 0:
				drawTarget(img, x, y, 10,LEFT_TARGET);
				break;
			case 1:
				drawTarget(img, x, y, 10,RIGHT_TARGET);
				//string window_name;
				// windows will be consequently enumerated from A
				window_name += 'A' + char(caps.size()); 
				caps.push_back(CapturedImage(img,window_name,cvPoint(x,y)));
				click_count = 0;
				waitingForMouseEvents = false;
				return;
			default:
				throw runtime_error("Unexpected click count");
				return;
			}			
			++click_count;	
			return;
		default:
			return;
		}
	}
}

int main()
try {

	cout << "Select a camera for capturing" << endl;
	cout << DEFAULT_CAMERA << ": Default camera (usually a webcam laptop webcam)" << endl;
	cout << EXTERNAL_CAMERA << ": Additional camera (usually an externally connected device)" << endl;
	int cam_index = DEFAULT_CAMERA;
	cin >> cam_index;
	cout << "Selected device # " << cam_index << endl;
	CvCapture *capture = cvCreateCameraCapture(cam_index);
	if (!capture){
		throw runtime_error("Can't access selected device.");
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

		// now we want our window to be topmost and have the focus
		// this cannot be done by opencv, we have to use WinAPI
		HWND winhandle = (HWND) cvGetWindowHandle(winName.c_str());
		SetForegroundWindow(winhandle);

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
