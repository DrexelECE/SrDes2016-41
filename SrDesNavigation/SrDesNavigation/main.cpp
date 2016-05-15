#include "Nav.h"
#include <vector>
#include <boost\asio.hpp>
#include <string>
#include <iostream>
#include "main.h"
#include "utils.h"

using boost::asio::ip::tcp;

int main(int argc, char** argv)
{
	cv::Mat image, bw, filtered; cv::Point centroid;
	std::vector<cv::Mat> RGBchannels; RGBchannels.resize(3);
	std::vector<bool*> pattern; bool terminated = false;
	std::vector<char> buffer;
	bool p1[3] = { true, false, false };
	bool p2[3] = { true, false, false };
	bool p3[3] = { true, false, false };
	pattern.push_back(p1);
	pattern.push_back(p2);
	pattern.push_back(p3);
	con_http con(argv[1], atoi(argv[2]));
	//Nav::init(con);
	//Nav::takeoff(con);

	while(!terminated){

		con.getImage(buffer);
		cv::Mat buf_image(buffer);
		image = cv::imdecode(buf_image, CV_LOAD_IMAGE_COLOR);
		image.convertTo(image, CV_8U);
		cv::blur(image, filtered, cv::Size(5, 5));
		split(image, RGBchannels.data());

		Nav::frameSegmenter(RGBchannels, bw);
		Nav::patternRec(bw, RGBchannels, pattern, centroid);
		terminated = true;
	

	}

	std::cout << "Center = " << centroid;// << std::endl;
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
		cv::imshow("Display window", filtered);
	cv::waitKey();
	return 0;
	
}
	