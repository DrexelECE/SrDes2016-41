#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <ctime>
#include "con_http.h"
#pragma once
class NavControls
{
private:
	const int maxThrottle = 3270; 
	int throttle, ailron, elevator, rutter, aux, claw;
	void setVals(int nthrottle = 0, int nailron = 0, int nelevator = 0,
		int nrutter = 0, int naux = 0, int nclaw = 0)
	{
		std::string pwmString; std::vector<char>buffer;
		throttle = nthrottle; ailron = nailron;
		elevator = nelevator; rutter = nrutter;
		aux = naux;	claw = nclaw;
		getPWMString(pwmString);
		con.updatePWM(buffer, pwmString);
	}

	void getPWMString(std::string &outString)
	{//convert to 3digit hex...
		outString = std::to_string(throttle) + "," + std::to_string(ailron) + ","
			+ std::to_string(elevator) + "," + std::to_string(rutter) + ","
			+ std::to_string(aux) + "," + std::to_string(claw);
	}

	void timer(double s)
	{
		clock_t begin = clock();
		while (double(clock() - begin) < s);
	}

public:
	con_http con;
	NavControls(char* ip, int port) :
		con(ip, port) {
	}
	
	void takeoff()
	{
		setVals((int)(.4*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);
		timer(5.0);
		setVals();
	}
	
	void landing()
	{
		setVals((int)(.3*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);
		timer(2.0);
		setVals((int)(.2*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);
		timer(2.0);
		setVals((int)(.1*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);
		timer(2.0);
		setVals();
	}

	void init()
	{
		int minThrottle = (int)((double)maxThrottle * 0.2);
		setVals(minThrottle, 0, 4096, 4096, 1000, 0);
		timer(0.5);
		setVals();
	}

	void pwmTransform(cv::Point centroid, bool & isLowerCamera, double imwidth, double imheight)
	{
		bool isUpperCamer = !isLowerCamera;
		if (centroid.x == -1.0)
		{
			setVals((int)(0.3*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);//neutral
			return;
		}
		if (isUpperCamer)
		{
			if (centroid.x - (imwidth / 2.0) > imwidth*0.1)
				setVals((int)(0.35*(double)maxThrottle), 2048.0*1.1, 2048.0*0.9, 2048*1.2, maxThrottle, 0);
			else if (centroid.x - (imwidth / 2.0) < (-imwidth*0.1))
				setVals((int)(0.35*(double)maxThrottle), 2048.0*0.9, 2048.0*1.1, 2048 * 0.8, maxThrottle, 0);
			timer(0.1);//run on other thread, cancel if new vals set before it expires
			setVals((int)(0.3*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);
		}
		else //isLowerCamer
		{
			if (centroid.x - (imwidth / 2.0) > 0)
				setVals((int)(0.3*(double)maxThrottle), 2048.0*1.05, 2048.0, 2048, maxThrottle, 0);
			else if (centroid.x - (imwidth / 2.0) < 0)
				setVals((int)(0.3*(double)maxThrottle), 2048.0*0.95, 2048.0, 2048, maxThrottle, 0);

			if (centroid.y - (imheight / 2.0) > 0)
				setVals((int)(0.3*(double)maxThrottle), 2048.0, 2048.0*1.05, 2048, maxThrottle, 0);
			else if (centroid.x - (imwidth / 2.0) < 0)
				setVals((int)(0.3*(double)maxThrottle), 2048.0, 2048.0*0.95, 2048, maxThrottle, 0);
			timer(0.05);
			setVals((int)(0.28*(double)maxThrottle), 2048, 2048, 2048, maxThrottle, 0);

		}
		//Use distance to figure out when claw should be open or closed
		//make a member variable to store pwm of claw, 
		//if (distance is close enough && isLowerCamera) var closedHook = closedValue;
		//...
	}
};