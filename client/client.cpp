#include <iostream>
#include <asio.hpp>
#include "define.h"
#include "client.h"
using asio::ip::tcp;

int screen_width = 960, screen_height = 540;
asio::io_context io_context;
tcp::socket screen_socket(io_context);
tcp::socket control_socket(io_context);

int ScreenSocketGetWidth() {
	return screen_width;
}

int ScreenSocketGetHeight() {
	return screen_height;
}

void ScreenSocketConnect(const char *address) {
	try {
		std::string raw_ip_address(address);
		tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), SOCKET_SCREEN_PORT);
		screen_socket = tcp::socket(io_context, ep.protocol());

		screen_socket.connect(ep);
		asio::read(screen_socket, asio::buffer(&screen_width, sizeof(int)));
		asio::read(screen_socket, asio::buffer(&screen_height, sizeof(int)));
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void ScreenSocketGetData(void *data, size_t size) {
	asio::error_code error;
	asio::read(screen_socket, asio::buffer(data, size), error);
}

void ScreenSocketClose() {
	screen_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	screen_socket.close();
}

//---------------------------------------------------------------

struct ControlBuffer {
	uint16_t opcode;
	int size;
};

void ControlSocketConnect(const char *address) {
	try {
		std::string raw_ip_address(address);
		tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), SOCKET_CONTROL_PORT);
		control_socket = tcp::socket(io_context, ep.protocol());

		control_socket.connect(ep);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void ControlSocketClose() {
	control_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	control_socket.close();
}

void ControlSocketSendData(uint16_t opcode, int size, void *data) {
	ControlBuffer buf = {opcode, size};
	asio::error_code error;
	asio::write(control_socket, asio::buffer(&buf, sizeof(buf)), error);
}

void ControlSocketGetData(void *data, int size) {
	asio::error_code error;
	asio::read(control_socket, asio::buffer(data, size), error);
}

void ControlSocketGetData(std::string &str, int size) {
	asio::error_code error;
	asio::read(control_socket, asio::buffer(str, size), error);
}

std::vector<std::pair<std::string, int>> ControlSocketGetProcesses() {
	ControlSocketSendData(PROCESS_LIST, 0, NULL);
	ControlBuffer buf;
	ControlSocketGetData(&buf, sizeof(buf));
	std::vector<std::pair<std::string, int>> result(buf.size);
	for (auto &each: result) {
		int size;
		ControlSocketGetData(&size, sizeof(int));
		each.first.resize(size);
		ControlSocketGetData(each.first, size);
		ControlSocketGetData(&each.second, sizeof(int));
		std::cout << "Process: " << each.first << " | PID: " << each.second << std::endl;
	}
	return result;
}

std::vector<std::pair<std::string, int>> ControlSocketGetApplications() {
	ControlSocketSendData(APP_LIST, 0, NULL);
	ControlBuffer buf;
	ControlSocketGetData(&buf, sizeof(buf));
	std::vector<std::pair<std::string, int>> result(buf.size);
	for (auto &each: result) {
		int size;
		ControlSocketGetData(&size, sizeof(int));
		each.first.resize(size);
		ControlSocketGetData(each.first, size);
		ControlSocketGetData(&each.second, sizeof(int));
		std::cout << "App: " << each.first << " | PID: " << each.second << std::endl;
	}
	return result;
}