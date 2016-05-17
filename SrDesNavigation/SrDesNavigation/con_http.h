#include <string>
#include <vector>
#include <boost\asio.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>

#pragma once

using boost::asio::ip::tcp;

class con_http
{
private:
	const std::string host; int port;
	boost::asio::io_service io_service;
	tcp::socket socket; std::vector<char> ret;
	boost::system::error_code error = boost::asio::error::host_not_found;
	tcp::endpoint endpoint;

public:
	con_http(char* host, int port) :
		socket(io_service), host(host), port(port),
		endpoint(boost::asio::ip::address::from_string(host, error), port) {
		ret.push_back('\r'); ret.push_back('\n'); ret.push_back('\r'); ret.push_back('\n');
	//	socket.connect(endpoint, error);
	//	checkCon();
	}
	void checkCon()
	{
		if (!socket.is_open())
		{
			socket.connect(endpoint, error);
			if (error && error != boost::asio::error::host_not_found)
				throw boost::system::system_error(error);
		}
	}
	void http_get(std::string path, std::vector<char> & buffer) {
		boost::asio::streambuf request;
		std::vector<char>::iterator itend, itbegin; 
		std::ostream request_stream(&request);

		request_stream << "GET " << path << " HTTP/1.1\r\n";
		request_stream << "Host: " << host << ":" << port << "\r\n";
		request_stream << "Connection: close\r\n\r\n";
		try {
			checkCon();
			boost::asio::write(socket, request);

			while (boost::asio::read(socket, boost::asio::buffer(buffer),
				boost::asio::transfer_at_least(80000), error))
				if (error && error != boost::asio::error::eof)
					throw boost::system::system_error(error);
			socket.close();

			itend = std::find_end(buffer.begin(), buffer.end(), ret.begin(), ret.begin()+2);
			itbegin = std::search(buffer.begin(), buffer.end(), ret.begin(), ret.end());
			buffer.assign(itbegin + 4, itend);
		}
		catch (std::exception& e) {
			std::cout << "Exception: " << e.what() << "\n";
			clock_t begin = clock();
			while (double(clock() - begin) < 0.5);
			socket.close();

		}
	}

	void getImage(std::vector<char> & buffer) {
		buffer.resize(80000);
		//http_get("/srDes/HttpTest/image/\n", buffer);
				http_get("/image?1\n", buffer);
	/*	for (std::vector<char>::const_iterator it = buffer.begin(); it != buffer.end(); it++)
			std::cout << *it;
		std::cout << std::endl;
		std::cout << buffer.size() << std::endl;*/
	}

	void updatePWM(std::vector<char> & buffer, std::string update) {
		socket.connect(endpoint, error);
		buffer.resize(256);
		//std::string uString = "/srDes/HttpTest/fly/?" + update + "\n";
		std::string uString = "/fly/?" + update + "\n";
		http_get(uString, buffer);
	/*	for (std::vector<char>::const_iterator it = buffer.begin(); it != buffer.end(); it++)
			std::cout << *it;
		std::cout << std::endl;*/
	}
};

