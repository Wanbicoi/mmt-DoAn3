#include <iostream>
#include <asio.hpp>
#include <functional>
#include "types.h"
#include "client.h"
using asio::ip::tcp;

int screen_width = 1920, screen_height = 1080;
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

void void_read() {

}

void ScreenSocketGetScreen(void *data) {
	asio::error_code error;
	int size;
	asio::read(screen_socket, asio::buffer(&size, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(data, size), error);
}

int ScreenSocketGetMouseInfo(int *mouse_x, int *mouse_y) {
	asio::error_code error;
	int mouse_changed = 0;
	asio::read(screen_socket, asio::buffer(mouse_x, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(mouse_y, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(&mouse_changed, sizeof(int)), error);
	return mouse_changed;
}

unsigned char* ScreenSocketGetMouse(int *mouse_width, int *mouse_height) {
	asio::error_code error;
	asio::read(screen_socket, asio::buffer(mouse_width, sizeof(int)), error);
	asio::read(screen_socket, asio::buffer(mouse_height, sizeof(int)), error);
	int size;
	asio::read(screen_socket, asio::buffer(&size, sizeof(int)), error);
	unsigned char *data = (unsigned char*)malloc(size);
	asio::read(screen_socket, asio::buffer(data, size), error);
	return data;
}

void ScreenSocketClose() {
	screen_socket.shutdown(asio::ip::tcp::socket::shutdown_both);
	screen_socket.close();
}

//---------------------------------------------------------------

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

void ControlSocketSendControl(OperationCode opcode, int data, void *raw_data) {
	ControlBuffer buf = {opcode, data};
	asio::error_code error;
	asio::write(control_socket, asio::buffer(&buf, sizeof(buf)), error);
	if (raw_data) {
		asio::write(control_socket, asio::buffer(raw_data, data), error);
	}
}

// template <typename T>
// void ControlSocketSendType(T obj) {
// 	asio::error_code error;
// 	asio::write(control_socket, asio::buffer(&obj, sizeof(obj)), error);
// }

// template void ControlSocketSendType<int>(int);
// template void ControlSocketSendType<float>(float);

void ControlSocketGetData(void *data, int size) {
	asio::error_code error;
	asio::read(control_socket, asio::buffer(data, size), error);
}

std::string ControlSocketGetString() {
	asio::error_code error;
	std::string res;
	int size;
	asio::read(control_socket, asio::buffer(&size, sizeof(int)), error);
	res.resize(size);
	asio::read(control_socket, asio::buffer(res, size), error);
	return res;
}

std::vector<ProcessInfo> ControlSocketGetProcesses() {
	ControlSocketSendControl(PROCESS_LIST);
	ControlBuffer buf;
	ControlSocketGetData(&buf, sizeof(buf));
	if (buf.opcode != PROCESS_LIST) return {};
	std::vector<ProcessInfo> processes(buf.data);
	for (auto &process: processes) {
		ControlSocketGetData(&process.pid, sizeof(int));
		process.name = ControlSocketGetString();
		ControlSocketGetData(&process.type, sizeof(char));
		// if (process.type == 1)
		// 	std::cout << "Application: ";
		// else
		// 	std::cout << "Process: ";
		// std::cout << process.name << " | PID: " << process.pid << std::endl;
	}
	return processes;
}