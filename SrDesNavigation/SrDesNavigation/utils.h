#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <string>
#include <vector>
#include <boost\asio.hpp>
#include <fstream>
#pragma once

using boost::asio::ip::tcp;

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
class con_http
{
	const std::string host; int port;
	boost::asio::io_service io_service;
	tcp::socket socket;
	boost::system::error_code error = boost::asio::error::host_not_found;

	tcp::endpoint endpoint;
public:
	con_http(std::string host, int port) :
		socket(io_service), host(host), port(port),
		endpoint(boost::asio::ip::address::from_string(host, error), port) {}

	void http_get(std::string path, std::vector<char> & buffer) {
		boost::asio::streambuf request;
		std::ostream request_stream(&request);

		request_stream << "GET " << path << " HTTP/1.1\r\n";
		request_stream << "Host: " << host << ":" << port << "\r\n";
		request_stream << "Connection: close\r\n\r\n";
		boost::asio::write(socket, request);

		while (boost::asio::read(socket, boost::asio::buffer(buffer),
			boost::asio::transfer_at_least(1), error));
		//if (error && error != boost::asio::error::eof)
			//throw boost::system::system_error(error);
		std::vector<char>::iterator it;
		std::vector<char> ret; ret.push_back('\r'); ret.push_back('\n'); ret.push_back('\0'); ret.push_back('\0');
		it = std::search(buffer.begin(), buffer.end(), ret.begin(), ret.end());
		int size = std::distance(buffer.begin(), it);
		buffer.resize(size);
		if(false)
		for (auto i = buffer.begin(); i != buffer.end(); ++i)
			std::cout << *i;
		std::cout << std::endl;
		std::ofstream file("picture.jpg", std::ofstream::binary);
//		file.write(buffer.data(), buffer.size());
	}
	void getImage(std::vector<char> & buffer) {
		socket.connect(endpoint, error);
		buffer.resize(500000); 
		http_get("/srDes/HttpTest/image/\n", buffer);
	}
	void updatePWM(std::vector<char> & buffer) {
		socket.connect(endpoint, error);
		buffer.resize(128); 
		http_get("/srDes/HttpTest/fly/\n", buffer);
	}
	
};