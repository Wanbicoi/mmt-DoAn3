#include <iostream>
#include <asio.hpp>
using asio::ip::tcp;

int client_width, client_height;
asio::io_service io_service;
tcp::socket client_socket(io_service);

int get_client_width() {
	return client_width;
}

int get_client_height() {
	return client_height;
}

void connect(const char *address) {
	try {
		std::string raw_ip_address(address);// = "192.168.56.1";
		tcp::endpoint ep(asio::ip::address::from_string(raw_ip_address), 13);
		client_socket = tcp::socket(io_service, ep.protocol());

		client_socket.connect(ep);
		client_socket.read_some(asio::buffer(&client_width, sizeof(int)));
		client_socket.read_some(asio::buffer(&client_height, sizeof(int)));
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
}

void get_image(void *data, size_t size) {
	asio::error_code error;
	size_t len = client_socket.read_some(asio::buffer(data, size), error);
}