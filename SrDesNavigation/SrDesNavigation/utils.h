#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
		
#pragma once

class im_utils {
public:
	static void threshold(cv::Mat & inputArray, cv::Mat & outputArray, double inTh = -1) {
		IplImage* im = new IplImage(inputArray);//assign the image to an IplImage pointer
		IplImage* finalIm = cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_8U, 1);
		if (inTh == -1)
		{
			double otsuThreshold = cvThreshold(im, finalIm, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
			double th = (otsuThreshold*1.5 <= 250.0) ? otsuThreshold*1.5 : 249;
			cvThreshold(im, finalIm, th, 255, cv::THRESH_BINARY);
		}else
			cvThreshold(im, finalIm, inTh, 255, cv::THRESH_BINARY);
		outputArray = cv::cvarrToMat(finalIm);
		return;
	}
	static void removeMask(cv::Mat & inputArray, int maskVal)
	{
		cv::Mat Mask;
		cv::compare(inputArray, maskVal, Mask, cv::CMP_EQ);
		Mask /= 255;
		Mask *= maskVal;
		cv::subtract(inputArray, Mask, inputArray);
	}

};