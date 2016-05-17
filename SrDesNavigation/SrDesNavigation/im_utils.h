#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <stdlib.h>
#pragma once


class im_utils {
	static void threshold(cv::Mat & inputArray, cv::Mat & outputArray, double inTh = -1) {
		IplImage* im = new IplImage(inputArray);//assign the image to an IplImage pointer
		IplImage* finalIm = cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_8U, 1);
		if (inTh == -1)
		{
			double otsuThreshold = cvThreshold(im, finalIm, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
			double th = (otsuThreshold <= 250.0) ? otsuThreshold*1.5 : 249;
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

public:
	static void frameSegmenter(std::vector<cv::Mat> & inputImage, cv::Mat & outImage)
	{
		cv::Mat mask, stats, centroids, bw, normalized;
		outImage = inputImage[0] == -1;
		for (int i = 0; i < inputImage.size(); i++)
		{
			threshold(inputImage[i], bw);
			//cv::erode(bw, bw, cv::Mat(), cv::Point(1, 1), 1, 1, 1);
			//cv::dilate(bw, bw, cv::Mat(), cv::Point(1, 1), 1, 1, 1);

			cv::connectedComponentsWithStats(bw, bw, stats, centroids, 4);
			bw.convertTo(bw, CV_8U);

			int num = stats.rows;
			for (int j = 1; j < num; j++)
			{
				double ra, height, width, x, y; int area;
				area = stats.at<int>(j, cv::CC_STAT_AREA);

				width = stats.at<int>(j, cv::CC_STAT_WIDTH);
				height = stats.at<int>(j, cv::CC_STAT_HEIGHT);
				ra = (width < height) ? width / height : height / width;
				x = stats.at<int>(j, cv::CC_STAT_LEFT) + (width / 2);
				y = stats.at<int>(j, cv::CC_STAT_TOP) + (height / 2);
				if (area > 500 || area < 10 || ra < 0.7 || x < 20 || y < 20)
					removeMask(bw, j);
			}
			cv::bitwise_or((bw >= 1), outImage, outImage);
		}
	
		return;
	}

	static void patternRec(cv::Mat & inputBW, cv::Mat & inputImage, 
		std::vector<std::vector<bool>> pattern, cv::Point & centroid)
	{
		cv::Mat labels, labels_mask, stats, stats_mask, centroids,
			centroids_mask, bw_dilate, bw_obj, bw_masked;
		double height, width, ra; int expected = pattern.size(), centroidInd, j;
		bool found = false;

		bw_dilate = inputBW.clone();
		cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
		for (int i = 0; i <= 20 && stats_mask.rows < (expected); i++)
		{
			cv::dilate(bw_dilate, bw_dilate, cv::Mat(), cv::Point(1, 1), 1, 1, 1);
			cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
			int count = stats.rows;
			for (j = 1; j < count; j++)
			{
				width = stats.at<int>(j, cv::CC_STAT_WIDTH);
				height = stats.at<int>(j, cv::CC_STAT_HEIGHT);
			
				ra = height / width;
				if (ra < .7)
				{
					bw_obj = labels == j;
					cv::bitwise_and(bw_obj, inputBW, bw_masked);
					cv::connectedComponentsWithStats(bw_masked, labels_mask, stats_mask, centroids_mask, 8);
					if (stats_mask.rows >= (expected)
						& colorCheck(labels_mask, stats_mask, inputImage, pattern))
					{
						found = true;
						break;
					}
				}
			}//Ugly code, too many '}'
			
		}
		
		if (found && stats_mask.rows >= (expected))
		{
			double x, y;
			centroidInd = j;
			x = stats.at<int>(centroidInd, cv::CC_STAT_LEFT) + (width / 2);
			y = stats.at<int>(centroidInd, cv::CC_STAT_TOP) + (height / 2);
			centroid = cv::Point((int)x, (int)y);
		}
		else
			centroid = cv::Point(-1, -1);
	}

	static bool colorCheck(cv::Mat &labels_mask, cv::Mat & stats, cv::Mat &inputImage, 
		std::vector<std::vector<bool>> /*value not&*/pattern)
	{
	
		cv::Mat temp;
		cv::Scalar pixelAvg, pMax;	
		for (int i = 1; i < stats.rows && pattern.size() > 0; i++)
		{
			double max; int area;
			std::vector<bool> bcolor;
			temp = labels_mask == i;
			inputImage.copyTo(temp, temp);
			area = stats.at<int>(i, cv::CC_STAT_AREA);
			pixelAvg = cv::sum(temp) / (255.0*(double)area);
			max = (pixelAvg[0] > pixelAvg[1]) ? pixelAvg[0] : pixelAvg[1];
			max = (max > pixelAvg[2] * 0.8) ? max : pixelAvg[2];
			for (int j = 0; j < 3; j++)
				bcolor.push_back(abs(pixelAvg[j] - max) < .05);
			for (int j = pattern.size()-1; j >= 0; j--)
				if (pattern[j] == bcolor)
				{
					pattern.erase(pattern.begin() + j);
					break;
				}
		}
		return (pattern.size() > 0);
	}
};
//ra = (width < height) ? width / height : height / width;
