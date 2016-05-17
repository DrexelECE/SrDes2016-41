#include <vector>
#include <string>
#include <iostream>
#include "im_utils.h"
#include "NavControls.h"
#include "Nav.h"

using boost::asio::ip::tcp;

int main(int argc, char** argv)
{
	bool terminated = false, isLowerCamera = false;
	cv::Mat image, bw, filtered; cv::Point centroid;
	std::vector<cv::Mat> RGBchannels; RGBchannels.resize(3);
	std::vector<std::vector<bool>> pattern; 
	std::vector<char> buffer;
	
	//Set Pattern
	std::vector<bool> p1; p1.push_back(true); p1.push_back(false); p1.push_back(false);
	std::vector<bool> p2; p2.push_back(true); p2.push_back(false); p2.push_back(false);
	std::vector<bool> p3; p3.push_back(false); p3.push_back(false); p3.push_back(true);
	pattern.push_back(p1); pattern.push_back(p2); // pattern.push_back(p3);

	NavControls controls(argv[1], atoi(argv[2]));
	//NavControls::init();
	//NavControls::takeoff();
	//cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	int frameCount = 0;
	while(!terminated){

		controls.con.getImage(buffer);
		cv::Mat buf_image(buffer);
		if (buffer.size() == 0)
			continue;
		image = cv::imdecode(buf_image, CV_LOAD_IMAGE_COLOR);
		if (image.dims == 0)
			continue;
		cv::normalize(image, image, 0.0, 255, cv::NORM_MINMAX);
		image.convertTo(image, CV_8U);

		//cv::blur(image, filtered, cv::Size(5, 5));
		cv::medianBlur(image, filtered, 5);
		split(filtered, RGBchannels.data());

		im_utils::frameSegmenter(RGBchannels, bw);
		im_utils::patternRec(bw, image, pattern, centroid);
		//Need to figure out distance for grabbing object in pattern rec... 
		//Needs more testing to do so.
		buffer.clear();

		controls.pwmTransform(centroid, isLowerCamera, image.cols, image.rows);

		/*//For saving images with plots

		circle(image, centroid, 3, CV_RGB(0,255, 0), 5);

	    std::cout << "Center = " << centroid << std::endl;
		cv::imshow("Display window", image);
		cv::waitKey(1);
		
		std::string filename;
		filename = "C:\\Users\\cscho0415\\Documents\\School\\Spring16\\SrDes\\vidDir\\image" 
			+ std::to_string(frameCount) + ".jpg";
	//	std::cout << filename << std::endl;
		cv::imwrite(filename, image);
		frameCount++;
		*/
	//	terminated = true;
	}	
	//controls.landing();
	
	return 0;
	
}
	