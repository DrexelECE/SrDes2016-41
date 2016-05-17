#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <iostream>
#include "utils.h"
#pragma once

class Nav {
public:
	static void frameSegmenter(std::vector<cv::Mat> & inputImage, cv::Mat & outImage)
	{
		cv::Mat mask, stats, centroids, bw;
		outImage = inputImage[0] == -1;
		for (int i = 0; i < inputImage.size(); i++)
		{
			im_utils::threshold(inputImage[i], bw);
			cv::erode(bw, bw, cv::Mat(), cv::Point(1, 1), 2, 1, 1);
			cv::dilate(bw, bw, cv::Mat(), cv::Point(1, 1), 1, 1, 1);
			cv::connectedComponentsWithStats(bw, bw, stats, centroids, 4);
			bw.convertTo(bw, CV_8U);
			int num = stats.rows;
			for (int j = 1; j < num; j++)
			{
				double ra, height, width; int area;
				area = stats.at<int>(j, cv::CC_STAT_AREA);

				width = stats.at<int>(j, cv::CC_STAT_WIDTH);
				height = stats.at<int>(j, cv::CC_STAT_HEIGHT);
				ra = (width < height) ? width / height : height / width;
				if (area > 1000 || area < 40 || ra < 0.88)
					im_utils::removeMask(bw, j);
			}
			cv::bitwise_or((bw >= 1), outImage, outImage);
		}
		return;
	}
static void patternRec(cv::Mat & inputBW, std::vector<cv::Mat> & inputImage, std::vector<bool[3]> pattern, cv::Point & centroid)
{
	cv::Mat labels, labels_mask, stats, stats_mask, centroids,
			centroids_mask, bw_dilate, bw_obj, bw_masked;
	double height, width, ra; int expected = pattern.size(), centroidInd, j;
	bw_dilate = inputBW.clone(); 

	cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
	for ( int i = 0; i <= 20 && stats_mask.rows != (expected + 1); i++)
	{
		cv::dilate(bw_dilate, bw_dilate, cv::Mat(), cv::Point(1, 1), 1, 1, 1);
		cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
		int count = stats.rows;
		for ( j = 0; j < count; j++) 
		{
			width = stats.at<int>(j, cv::CC_STAT_WIDTH);
			height = stats.at<int>(j, cv::CC_STAT_HEIGHT);
			ra = (width < height) ? width / height : height / width;
			if (ra < .4)
			{
				bw_obj = labels == j;
				cv::bitwise_and(bw_obj, inputBW, bw_masked);
				cv::connectedComponentsWithStats(bw_masked, labels_mask, stats_mask, centroids_mask, 8);
				if (stats_mask.rows == (expected + 1))
				{
					for (int k = 0; k < expected; k++)
					{
						cv::Mat obj_mask, maskedImage;
						//check color
						maskedImage.copyTo(inputImage.data(), )
					}
					break;
				}
			} 
		}
	}
	centroidInd = j;
	int x, y;
	x = stats.at<int>(centroidInd, cv::CC_STAT_LEFT)+(width/2);
	y = stats.at<int>(centroidInd, cv::CC_STAT_TOP) + (height / 2);
	centroid = cv::Point(x, y);
}
};