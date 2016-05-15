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
			double otsuThreshold = cvThreshold(im, finalIm, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
			double th = (otsuThreshold*2 <= 250.0) ? otsuThreshold*1.5 : 249;
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
	con_http(char* host, int port) :
		socket(io_service), host(host), port(port),
		endpoint(boost::asio::ip::address::from_string(host, error), port) {}

	void http_get(std::string path, std::vector<char> & buffer) {
		try {
			boost::asio::streambuf request;
			std::ostream request_stream(&request);

			request_stream << "GET " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << host << ":" << port << "\r\n";
			request_stream << "Connection: close\r\n\r\n";
			boost::asio::write(socket, request);

			while (boost::asio::read(socket, boost::asio::buffer(buffer),
				boost::asio::transfer_at_least(70000), error))
				if (error && error != boost::asio::error::eof)
					throw boost::system::system_error(error);
			boost::asio::streambuf response;

//			boost::asio::read_until(socket, response, "\r\n\r\n");
			std::vector<char>::iterator it;
			std::vector<char> ret; ret.push_back('\r'); ret.push_back('\n'); //ret.push_back('\r'); ret.push_back('\n'); //ret.push_back('\0'); 
			it = std::find_end(buffer.begin(), buffer.end(), ret.begin(), ret.end());
			int size = std::distance(buffer.begin(), it);
			buffer.resize(size);
		}
		catch (std::exception& e) {
			std::cout << "Exception: " << e.what() << "\n";
		}	
	}
	void getImage(std::vector<char> & buffer) {
		socket.connect(endpoint, error);
		buffer.resize(70000); 
		http_get("/srDes/HttpTest/image/\n", buffer);
//		http_get("/image/\n", buffer);
		//std::cout << buffer.size() << std::endl;

	}
	void updatePWM(std::vector<char> & buffer, std::string update) {
		socket.connect(endpoint, error);
		buffer.resize(256);
		std::string uString = "/srDes/HttpTest/fly/?" + update + "\n";
	//	std::string uString = "/fly/?" + update + "\n";
		http_get(uString, buffer);
		for (std::vector<char>::const_iterator  it = buffer.begin(); it != buffer.end(); it++)
			std::cout << *it;
		std::cout << std::endl;
	}
	
};