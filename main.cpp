#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/stitching/stitcher.hpp>
#include <string>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <windows.h>
using namespace std;

const string winName = "camera capture";
const string fileBegin = "Image";
const string fileEnd = ".jpg";
const string outputFile = "panorama.jpg";
const char ESC = 27;
const char ENTER = 13;
const char SPACE = 32;
const int DEFAULT_CAMERA = 0;
const int EXTERNAL_CAMERA = 1;

unsigned int NUMBER_OF_CAPTURES = 5;


//The width ratio of the ROI can optionally be changed, for example to 3/5.
const double default_ROI_ratio = 1.0; // means that with of ROI is equal to full with of the captured frame

vector<cv::Mat> imgs;

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

	cout << "How many captured images should be stitched?" << endl;
	cin >> NUMBER_OF_CAPTURES;
	if (!cin) cout << "Default number of captures set: " << NUMBER_OF_CAPTURES << endl;

	cout << "Set width ratio of the capture, from 0.1 to 1.0" << endl;
	double ROI_ratio;
	cin >> ROI_ratio;
	if (!cin || ROI_ratio < 0.1 || ROI_ratio > 1.0) {
		ROI_ratio = default_ROI_ratio;
		cout << "Bad ratio specified, using default value: ";}
	else 
		cout << "Ratio value selected: ";

	cout << ROI_ratio << endl;

	double width = capture.get(CV_CAP_PROP_FRAME_WIDTH);
	double height = capture.get(CV_CAP_PROP_FRAME_HEIGHT);

	cout << "Original capture width: " << width << endl;
	cout << "Original capture height: " << height << endl;

    int ROI_height = int(height);
    int ROI_width = int(ROI_ratio*width);
    int ROI_x_offset = int((width - ROI_width)/ 2.0); // ROI is in the middle
    int ROI_y_offset = 0;

	cout << "Region of interest width: " << ROI_width << endl;
	cout << "Region of interest height: " << ROI_height << endl;

	cv::Rect frame_ROI(ROI_x_offset,ROI_y_offset,ROI_width,ROI_height);

	cout << "Press Enter for frame capture and Escape for exit" << endl;

	unsigned int counter = 0;

	cv::namedWindow(winName);

	for (;;){
		cv::Mat original_frame;
		capture >> original_frame;
		//if (!capture) throw runtime_error("Failed to query frame");
		
		// setting ROI for the original frame. All work will be then done on this ROI.
		cv::Mat frame(original_frame,frame_ROI);
        // Entire image won't be shown
        // Only ROI will be shown

		cv::Mat shown_frame(frame.clone());
		cv::Point text_origin(10,60);
		ostringstream ost;
		ost << counter;
		cv::putText(shown_frame,ost.str(),text_origin,cv::FONT_HERSHEY_PLAIN,5,CV_RGB(255,0,0),10);

		cv::imshow(winName,shown_frame);
        unsigned c = (unsigned) cv::waitKey(1); // we only care about actual buttons on keyboard, not special symbols like EOF or smth...

		// now we want our window to be topmost and have the focus
		// this cannot be done by opencv, we have to use WinAPI
		HWND winhandle = (HWND) cvGetWindowHandle(winName.c_str());
		SetForegroundWindow(winhandle);
		BringWindowToTop(winhandle);

		bool ready_for_stitching = false;

		switch (c){
		case ESC:
			return 0;
		case ENTER:{
			cout << "Press Enter to confirm capturing or Esc to discard capture and make a new one." << endl;
			for (;;){
				bool finished_selecting = false;
				cv::imshow(winName,shown_frame);
				int confirm_capture = cv::waitKey(1);	
				switch (confirm_capture){
				case ESC:
					finished_selecting = true;
					break; // break from switch
				case ENTER:
					{
						ostringstream ost;
						ost << fileBegin << counter << fileEnd;
						cv::imwrite(ost.str(),frame);
						cout << "captured: " << ost.str() << endl;
						cv::imshow(ost.str(),shown_frame);
						cv::waitKey(1);

					}
					if (imgs.size() < NUMBER_OF_CAPTURES) imgs.push_back(frame.clone());
					else imgs.at(counter) = frame.clone();
						
					++counter;
					if (counter >= NUMBER_OF_CAPTURES) counter = 0;
					finished_selecting = true;
					break; // break from switch
				default:
					break;
				}
				if (finished_selecting) break; // break from inner loop
			}
			}
			break; // break from outer switch, causes new iteration of outer loop
		case SPACE:
			if (imgs.size() == NUMBER_OF_CAPTURES) ready_for_stitching = true;
			if (imgs.size() > NUMBER_OF_CAPTURES) throw runtime_error("Too many captures in the vector");
			if (imgs.size() < NUMBER_OF_CAPTURES) cout << "Not enough images was captured" << endl;
			
			break;
		default:
			if (c >= '0' && c < ('0' + NUMBER_OF_CAPTURES) && (c - '0') < imgs.size())
				counter = c - '0';
			break;
		}
		if (ready_for_stitching) {
			cv::destroyWindow(winName);
			cout << "Started stitching of " << NUMBER_OF_CAPTURES << " images..." << endl;
			cv::Mat pano;
			cv::Stitcher stitcher = cv::Stitcher::createDefault(true);
			cv::Stitcher::Status status = stitcher.stitch(imgs, pano);

			if (status != cv::Stitcher::OK) cerr << "Can't stitch images, error code = " << int(status) << endl;
			else {
				
				if (pano.total() <= 1) cout << "Stitching failed, result is empty or too small" << endl;
				cout << "Done stitching, press any key to start over or Esc to quit" << endl;
				cv::imshow("Panorama",pano);
				cv::imwrite(outputFile,pano);
				if (cv::waitKey() == ESC) break;
			}
		}
			
	}
	
}
catch (exception& e){
	cerr << e.what() << endl;
	cerr << "Press Enter to quit" << endl;
	cin.ignore(1);
	return 1;
}
catch (...){
	cerr << "Unknown exception" << endl;
	cin.ignore(1);
	return 2;
}
