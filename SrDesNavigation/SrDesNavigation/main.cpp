#include "Nav.h"
#include <vector>
int main(int argc, char** argv)
{
	cv::Mat image, bw, filtered; cv::Point p;
	std::vector<cv::Mat> RGBchannels; RGBchannels.resize(3);

	image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

	cv::blur(image, filtered, cv::Size(7, 7));
	split(image, RGBchannels.data());

	Nav::frameSegmenter(RGBchannels, bw);
	Nav::patternRec(bw, RGBchannels, p);

	std::cout << "Center = " << p << std::endl;
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::waitKey();
	return 0;
}
