#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "define.h"
using asio::ip::tcp;

class ScreenConnection : public std::enable_shared_from_this<ScreenConnection> {
public:
	typedef std::shared_ptr<ScreenConnection> pointer;

	static pointer create(asio::io_context& io_context) {
		return pointer(new ScreenConnection(io_context));
	}

	tcp::socket& socket() {
		return socket_;
	}

	void start(void *data, int width, int height, size_t data_size) {
		data_ = data;
		data_size_ = data_size;

		asio::error_code ignored_error;
		asio::write(socket_, asio::buffer(&width, sizeof(int)), ignored_error);
		asio::write(socket_, asio::buffer(&height, sizeof(int)), ignored_error);
		asio::async_write(socket_, asio::buffer(data, data_size),
				std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

private:
	ScreenConnection(asio::io_context& io_context) : socket_(io_context) {}

	void handle_write(asio::error_code error) {
		if (!error) {
			asio::write(socket_, asio::buffer(data_, data_size_), error);
			handle_write(error);
		}
	}

	tcp::socket socket_;
	void *data_;
	size_t data_size_;
};

class ScreenServer {
public:
	ScreenServer(asio::io_context& io_context, void *data, int width, int height, size_t data_size)
		: io_context_(io_context)
		, acceptor_(io_context, tcp::endpoint(tcp::v4(), SOCKET_SCREEN_PORT))
		, data_(data), width_(width), height_(height), data_size_(data_size) {
		start_accept();
	}

private:
	void start_accept() {
		ScreenConnection::pointer new_connection = ScreenConnection::create(io_context_);

		acceptor_.async_accept(new_connection->socket(),
				std::bind(&ScreenServer::handle_accept, this, new_connection, std::placeholders::_1 /*error*/));
	}

	void handle_accept(ScreenConnection::pointer new_connection, const asio::error_code& error) {
		if (!error) {
			new_connection->start(data_, width_, height_, data_size_);
		}
		start_accept();
	}

	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	void *data_;
	int width_;
	int height_;
	size_t data_size_;
};


//-------------------------------------------------------------------------------


class ControlConnection : public std::enable_shared_from_this<ControlConnection> {
public:
	typedef std::shared_ptr<ControlConnection> pointer;

	static pointer create(asio::io_context& io_context) {
		return pointer(new ControlConnection(io_context));
	}

	tcp::socket& socket() {
		return socket_;
	}

	void start() {
		// asio::async_write(socket_, asio::buffer(data, data_size),
		// 		std::bind(&ControlConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

private:
	ControlConnection(asio::io_context& io_context) : socket_(io_context) {}

	void handle_write(asio::error_code error) {
		if (!error) {
			//asio::write(socket_, asio::buffer(data_, data_size_), error);
			handle_write(error);
		}
	}

	tcp::socket socket_;
	void *data_;
	size_t data_size_;
};

class ControlServer {
public:
	ControlServer(asio::io_context& io_context)
		: io_context_(io_context)
		, acceptor_(io_context, tcp::endpoint(tcp::v4(), SOCKET_CONTROL_PORT)) {
		start_accept();
	}

private:
	void start_accept() {
		ControlConnection::pointer new_connection = ControlConnection::create(io_context_);

		acceptor_.async_accept(new_connection->socket(),
				std::bind(&ControlServer::handle_accept, this, new_connection, std::placeholders::_1 /*error*/));
	}

	void handle_accept(ControlConnection::pointer new_connection, const asio::error_code& error) {
		if (!error) {
			new_connection->start();
		}
		start_accept();
	}

	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
};