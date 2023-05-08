#include <iostream>
#include <asio.hpp>
#include "define.h"
using asio::ip::tcp;

int screen_width, screen_height;
asio::io_service io_service;
tcp::socket screen_socket(io_service);
tcp::socket control_socket(io_service);

int ScreenSocketGetWidth() {
	return screen_width;
}

int ScreenSocketGetHeight() {
	return screen_height;
}

void ScreenSocketConnect(const char *address) {
	try {
		std::string raw_ip_address(address);// = "192.168.56.1";
		tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), SOCKET_SCREEN_PORT);
		screen_socket = tcp::socket(io_service, ep.protocol());

		screen_socket.connect(ep);
		screen_socket.read_some(asio::buffer(&screen_width, sizeof(int)));
		screen_socket.read_some(asio::buffer(&screen_height, sizeof(int)));
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void ScreenSocketGetData(void *data, size_t size) {
	asio::error_code error;
	size_t len = asio::read(screen_socket, asio::buffer(data, size), error);
}

void ScreenSocketClose() {
	screen_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	screen_socket.close();
}