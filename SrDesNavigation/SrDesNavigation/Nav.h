#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <ctime>
#include <iostream>
#include "utils.h"
#pragma once



class Nav {
	//const int maxThrottle = (int)()
public:
	static void frameSegmenter(std::vector<cv::Mat> & inputImage, cv::Mat & outImage)
	{
		cv::Mat mask, stats, centroids, bw, normalized;
		outImage = inputImage[0] == -1;
		//for (int i = 0; i < inputImage.size(); i++)
		{
			int i = 0;
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
				if (area > 500 || area < 10 || ra < 0.7)
					im_utils::removeMask(bw, j);
				
			}
			cv::bitwise_or((bw >= 1), outImage, outImage);
			
		}

		
		return;
	}

	static void patternRec(cv::Mat & inputBW, std::vector<cv::Mat> & inputImage, std::vector<bool*> pattern, cv::Point & centroid)
	{
		cv::Mat labels, labels_mask, stats, stats_mask, centroids,
				centroids_mask, bw_dilate, bw_obj, bw_masked;
		double height, width, ra; int expected = (int)pattern.size(), centroidInd, j;
		bw_dilate = inputBW.clone(); 

		cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
		for ( int i = 0; i <= 20 && stats_mask.rows != (expected ); i++)
		{
			cv::dilate(bw_dilate, bw_dilate, cv::Mat(), cv::Point(1, 1), 1, 1, 1);
			cv::connectedComponentsWithStats(bw_dilate, labels, stats, centroids, 8);
			int count = stats.rows;
			for ( j = 1; j < count; j++) 
			{
				width = stats.at<int>(j, cv::CC_STAT_WIDTH);
				height = stats.at<int>(j, cv::CC_STAT_HEIGHT);
				//ra = (width < height) ? width / height : height / width;
				ra = height / width;
				if (ra < .5)
				{
					bw_obj = labels == j;
					cv::bitwise_and(bw_obj, inputBW, bw_masked);
					cv::connectedComponentsWithStats(bw_masked, labels_mask, stats_mask, centroids_mask, 8);
					if (stats_mask.rows == (expected ))	break;
				} 
				
			}
		}
		centroidInd = j;
		double x, y;
		x = stats.at<int>(centroidInd, cv::CC_STAT_LEFT) +(width / 2);
		y = stats.at<int>(centroidInd, cv::CC_STAT_TOP) + (height / 2);
		centroid = cv::Point((int)x, (int)y);
	}

	static void pwmTransform(con_http & con, cv::Point centroid, bool isLowerCamera, std::string & out)
	{
		bool isUpperCamer = !isLowerCamera;
		if (isUpperCamer)
		{

		}
		else //isLowerCamer
		{

		}
	}
	static void takeoff(con_http &con)
	{
		int maxThrottle = 3270;
		int takeoffThrottle = .4*maxThrottle;
		double takeoffTime = 5.0;
		std::string aux = std::to_string(maxThrottle);
		std::string pwmString = std::to_string(takeoffThrottle) + ",2048,2048,2048,2048," + aux;
		std::vector<char>buffer;
		con.updatePWM(buffer, pwmString);
		clock_t begin = clock();
		while (double(clock() - begin) < takeoffTime);
		con.updatePWM(buffer, "0,0,0,0,0,0");

	}
	static void init(con_http &con)
	{
		int maxThrottle = 3270;
		int minThrottle = (int)(4096.0 * 0.2);
		double runTime = 0.5;
		clock_t begin = clock();
		std::string aux = "1000";
		std::string pwmString = std::to_string(minThrottle) + ",0,0,4096,4096";
		std::vector<char>buffer;
		con.updatePWM(buffer, pwmString);
		while (double(clock() - begin) < runTime);
		con.updatePWM(buffer, "0,0,0,0,0,0,0");

	}
};

/*
for (int k = 1; k <= expected; k++)
{
cv::Mat obj_mask, maskedImage;
//check color
cv::compare(labels_mask, k, maskedImage, cv::CMP_EQ);
//						maskedImage = maskedImage * inputImage[0];
cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
cv::imshow("Display window", maskedImage);
cv::waitKey();
}*/