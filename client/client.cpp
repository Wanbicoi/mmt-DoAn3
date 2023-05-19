#include <iostream>
#include <asio.hpp>
#include <functional>
#include "types.h"
#include "client.h"
using asio::ip::tcp;

asio::io_context io_context;
tcp::socket screen_socket(io_context);
tcp::socket control_socket(io_context);

ScreenClient::ScreenClient() {

}

void ScreenClient::connect(const char *address) {
	try {
		std::string raw_ip_address(address);
		tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), SOCKET_SCREEN_PORT);
		screen_socket = tcp::socket(io_context, ep.protocol());

		screen_socket.connect(ep);
		asio::read(screen_socket, asio::buffer(&screen_info, sizeof(ScreenInfo)));
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

int ScreenClient::getWidth() {
	return screen_info.width;
}

int ScreenClient::getHeight() {
	return screen_info.height;
}

void ScreenClient::getScreenData(void *data) {
	asio::error_code error;
	int size;
	asio::read(screen_socket, asio::buffer(&size, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(data, size), error);
}

int ScreenClient::getMouseInfo(int *mouse_x, int *mouse_y) {
	asio::error_code error;
	int mouse_changed = 0;
	asio::read(screen_socket, asio::buffer(mouse_x, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(mouse_y, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(&mouse_changed, sizeof(int)), error);
	return mouse_changed;
}

unsigned char* ScreenClient::getMouseData(int *mouse_width, int *mouse_height) {
	asio::error_code error;
	asio::read(screen_socket, asio::buffer(mouse_width, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(mouse_height, sizeof(int)), error);
	int size;
	asio::read(screen_socket, asio::buffer(&size, sizeof(int)), error);
	unsigned char *data = (unsigned char*)malloc(size);
	asio::read(screen_socket, asio::buffer(data, size), error);
	return data;
}

ScreenClient::~ScreenClient() {
	//screen_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	//screen_socket.close();
}

//---------------------------------------------------------------

ControlClient::ControlClient() {

}

void ControlClient::getData(void *data, int size) {
	asio::error_code error;
	asio::read(control_socket, asio::buffer(data, size), error);
}

std::string ControlClient::getString() {
	asio::error_code error;
	std::string res;
	int size;
	asio::read(control_socket, asio::buffer(&size, sizeof(int)), error);
	res.resize(size);
	asio::read(control_socket, asio::buffer(res, size), error);
	return res;
}

void ControlClient::connect(const char *address) {
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

void ControlClient::sendControl(OperationCode opcode, int data, void *raw_data) {
	ControlBuffer buf = {opcode, data};
	asio::error_code error;
	asio::write(control_socket, asio::buffer(&buf, sizeof(buf)), error);
	if (raw_data) {
		asio::write(control_socket, asio::buffer(raw_data, data), error);
	}
}

std::vector<ProcessInfo> ControlClient::getProcesses() {
	sendControl(PROCESS_LIST);
	ControlBuffer buf;
	getData(&buf, sizeof(buf));
	if (buf.opcode != PROCESS_LIST) return {};
	std::vector<ProcessInfo> processes(buf.data);
	for (auto &process: processes) {
		getData(&process.pid, sizeof(int));
		process.name = getString();
		getData(&process.type, sizeof(char));
	}
	return processes;
}

ControlClient::~ControlClient() {
	//control_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	//control_socket.close();
}

// template <typename T>
// void ControlClient::sendType(T obj) {
// 	asio::error_code error;
// 	asio::write(control_socket, asio::buffer(&obj, sizeof(obj)), error);
// }

// template void ControlSocketSendType<int>(int);
// template void ControlSocketSendType<float>(float);


void IoContextRun() {
	io_context.run();
}