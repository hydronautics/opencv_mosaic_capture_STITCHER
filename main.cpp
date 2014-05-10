#include <opencv2/opencv.hpp>
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
const char ESC = 27;
const char ENTER = 13;
const int DEFAULT_CAMERA = 0;
const int EXTERNAL_CAMERA = 1;

const int NUMBER_OF_CAPTURES = 3;

// snapshot location is 60x100 centimeters
const double ROI_ratio = 3.0/5;

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

	int counter = 0;

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
        int c = cv::waitKey(1);

		// now we want our window to be topmost and have the focus
		// this cannot be done by opencv, we have to use WinAPI
		HWND winhandle = (HWND) cvGetWindowHandle(winName.c_str());
		SetForegroundWindow(winhandle);

		bool ready_for_stitching = false;

		switch (c){
		case ESC:
			cout << "Vector size is " << imgs.size() << endl;
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
					imgs.push_back(frame.clone());
					if (imgs.size() >= NUMBER_OF_CAPTURES){
						ready_for_stitching = true;
					}

					++counter;

					finished_selecting = true;
					break; // break from switch
				default:
					break;
				}
				if (finished_selecting) break; // break from inner loop
			}
			}
			break; // break from outer switch, causes new iteration of outer loop
		default:
			break;
		}
		if (ready_for_stitching) break;
	}
	cv::destroyWindow(winName);
	cout << "Started stitching of " << imgs.size() << " images..." << endl;
	cv::Mat pano;
	cv::Stitcher stitcher = cv::Stitcher::createDefault(true);
	cv::Stitcher::Status status = stitcher.stitch(imgs, pano);

	if (status != cv::Stitcher::OK)
	{
		cerr << "Can't stitch images, error code = " << int(status) << endl;
		return -1;
	}
	
	cv::imshow("Panorama",pano);
	cv::waitKey();
}
catch (exception& e){
	cerr << e.what() << endl;
	cerr << "Press Enter to quit" << endl;
	cin.ignore(1);
	return 1;
}
