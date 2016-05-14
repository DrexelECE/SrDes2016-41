#include "Nav.h"
#include <vector>
#include <boost\asio.hpp>
#include <string>
#include <iostream>
#include "main.h"
using boost::asio::ip::tcp;

int main(int argc, char** argv)
{
	std::string host, path;
	host = "10.0.0.10:8000";
	path = "/srDes/HttpTest/image/\n";
	try {
		boost::asio::io_service io_service;
		tcp::socket socket(io_service);
		boost::system::error_code error = boost::asio::error::host_not_found;
	
		boost::asio::ip::address addr = boost::asio::ip::address::from_string("10.0.0.10", error);
		tcp::endpoint endpoint(addr, 8000);
		socket.connect(endpoint, error);

		if (error)
			throw boost::system::system_error(error);
	
			boost::asio::streambuf request;
			std::ostream request_stream(&request);
	
			request_stream << "GET " << path << " HTTP/1.1\r\n";
			request_stream << "Host: " << host << "\r\n";
	//		request_stream << "Accept: */*\r\n";
			request_stream << "Connection: close\r\n\r\n";
			boost::asio::write(socket, request);
			std::vector<char>buf;
			buf.resize(30000000);
			socket.read_some(boost::asio::buffer(buf));

			boost::asio::streambuf response;
			boost::asio::read_until(socket, response, "\r\n");

			std::istream response_stream(&response);
			std::string http_version;
			response_stream >> http_version;
			unsigned int status_code;
			response_stream >> status_code;
			std::string status_message;
			std::getline(response_stream, status_message);
			if (!response_stream  || http_version.substr(0, 5) != "HTTP/")
			{
				std::cout << "Invalid response\n";
				return 1;
			}
			if (status_code != 200)
			{
				std::cout << "Response returned with status code " << status_code << "\n";
				return 1;
			}
			// Read the response headers, which are terminated by a blank line.
			boost::asio::read_until(socket, response, "\r\n\r\n");

			// Process the response headers.
			std::string header;
			while (std::getline(response_stream, header) && header != "\r")
				std::cout << header << "\n";
			std::cout << "\n";

			// Write whatever content we already have to output.
			if (response.size() > 0)
				std::cout << &response;

			// Read until EOF, writing data to output as we go.
			while (boost::asio::read(socket, response,
				boost::asio::transfer_at_least(1), error))
				std::cout << &response;
			if (error != boost::asio::error::eof)
				throw boost::system::system_error(error);
		}
		catch (std::exception& e)
		{
			std::cout << "Exception: " << e.what() << "\n";
			std::string pp;
			std::getline(std::cin, pp);


		}
		std::string pp;
		std::getline(std::cin, pp);
		
		
	
}
	

	
	/*cv::Mat image, bw, filtered; cv::Point centroid;
	std::vector<cv::Mat> RGBchannels; RGBchannels.resize(3);
	std::vector<bool*> pattern; 
	bool p1[3] = { true, false, false };
	bool p2[3] = { true, false, false };
	bool p3[3] = { true, false, false };
	pattern.push_back(p1);
	pattern.push_back(p2);
	pattern.push_back(p3);

	image = cv::imread(argv[1], CV_LOAD_IMAGE_COLOR);   // Read the file

	cv::blur(image, filtered, cv::Size(7, 7));
	split(image, RGBchannels.data());

	Nav::frameSegmenter(RGBchannels, bw);
	Nav::patternRec(bw, RGBchannels, pattern, centroid);

	std::cout << "Center = " << centroid << std::endl;
	cv::namedWindow("Display window", cv::WINDOW_AUTOSIZE);// Create a window for display.
	cv::waitKey();
	return 0;*/

