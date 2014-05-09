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

	cv::Mat img;
	string windowName;
	cv::Point left_point;
	cv::Point right_point;

	CapturedImage(const cv::Mat& i, const string& wn,cv::Point lp, cv::Point rp):
		img(i.clone()),
		windowName(wn),
		left_point(lp),
		right_point(rp)
	{}

	CapturedImage(const CapturedImage& src):
		img(src.img.clone()),
		windowName(src.windowName),
		left_point(src.left_point),
		right_point(src.right_point)
	{}

};

vector<CapturedImage> caps;
// snapshot location is 60x100 centimeters
const double ROI_ratio = 3.0/5;

typedef enum Target {
	LEFT_TARGET,
	RIGHT_TARGET,
};

void drawTarget(cv::Mat& img, int x, int y, int radius, Target t){

		
		cv::Scalar target_color;

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
		cv::circle(img,cv::Point(x, y),radius,target_color);
        cv::line(img, cv::Point(x-radius/2, y-radius/2), cv::Point(x+radius/2, y+radius/2),target_color);
        cv::line(img, cv::Point(x-radius/2, y+radius/2), cv::Point(x+radius/2, y-radius/2),target_color);
}

// обработчик событий от мышки
void myMouseCallback( int event, int x, int y, int, void* param )
{
	static int click_count; // default to 0
	static cv::Point prev_point;
	if (waitingForMouseEvents){
		cv::Mat& img = *(cv::Mat*) param;
		string window_name;

		switch( event ){
		case CV_EVENT_LBUTTONDOWN:
			cout << x << " " << y << endl;
			switch (click_count)
			{
			case 0:
				drawTarget(img, x, y, 10,LEFT_TARGET);
				prev_point = cv::Point(x,y);
				break;
			case 1:
				drawTarget(img, x, y, 10,RIGHT_TARGET);
				//string window_name;
				// windows will be consequently enumerated from A
				window_name += 'A' + char(caps.size()); 
				caps.push_back(CapturedImage(img,window_name,prev_point,cv::Point(x,y)));
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
	cv::VideoCapture capture(cam_index);
	if (!capture.isOpened()){
		throw runtime_error("Can't access selected device.");
	}

	double width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	double height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	cout << "width: " << width << endl;
	cout << "height: " << height << endl;

    int ROI_height = int(height);
    int ROI_width = int(ROI_ratio*width);
    int ROI_x_offset = int((width - ROI_width)/ 2.0); // ROI is in the middle
    int ROI_y_offset = 0;

	cv::Rect frame_ROI(ROI_x_offset,ROI_y_offset,ROI_width,ROI_height);

	cout << "Press Enter for frame capture and Escape for exit" << endl;

	int counter = 0;

	cv::namedWindow(winName);

	for (;;){
		cv::Mat frame;
		capture >> frame;
		//if (!capture) throw runtime_error("Failed to query frame");
		
        //cvSetImageROI(frame,frame_ROI);
        // Entire image won't be shown
        // Only ROI will be shown
		cv::imshow(winName,frame);
        
        int c = cv::waitKey(1);

		// now we want our window to be topmost and have the focus
		// this cannot be done by opencv, we have to use WinAPI
		HWND winhandle = (HWND) cvGetWindowHandle(winName.c_str());
		SetForegroundWindow(winhandle);

		if (c == ESC)
			break;
		else if (c == ENTER){
            waitingForMouseEvents = true;
			cv::setMouseCallback(winName,myMouseCallback, (void *) &frame);
            while (waitingForMouseEvents){
				cv::imshow(winName,frame);
				cv::waitKey(1);
			}
            ostringstream ost;
            ost << fileBegin << counter << fileEnd;
			cv::imwrite(ost.str(),frame);

            cout << "captured: " << ost.str() << endl;
			for (vector<CapturedImage>::size_type i = 0; i < caps.size(); ++i){
				CapturedImage& this_img = caps.at(i);
				cv::imshow(this_img.windowName,this_img.img);
			
			}

            ++counter;
		}
	}

	return 0;
}
catch (exception& e){
	cerr << e.what() << endl;
	cerr << "Press Enter to quit" << endl;
	cin.ignore(1);
	return 1;
}
