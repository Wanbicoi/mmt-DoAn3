#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "define.h"
using asio::ip::tcp;

struct ScreenBuffer {
	int width;
	int height;
	int size;
	void *data;
};

class ScreenConnection : public std::enable_shared_from_this<ScreenConnection> {
public:
	typedef std::shared_ptr<ScreenConnection> pointer;

	static pointer create(asio::io_context& io_context) {
		return pointer(new ScreenConnection(io_context));
	}

	tcp::socket& socket() {
		return socket_;
	}

	template <typename MESSAGE_CALLBACK>
	void start(const MESSAGE_CALLBACK &callback) {
		ScreenBuffer res = callback();
		data_ = res.data;
		data_size_ = res.size;

		asio::error_code ignored_error;
		asio::write(socket_, asio::buffer(&res.width, sizeof(int)), ignored_error);
		asio::write(socket_, asio::buffer(&res.height, sizeof(int)), ignored_error);
		asio::async_write(socket_, asio::buffer(data_, data_size_),
				std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

private:
	ScreenConnection(asio::io_context& io_context) : socket_(io_context) {}

	void handle_write(asio::error_code error) {
		if (!error) {
			asio::async_write(socket_, asio::buffer(data_, data_size_),
				std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
		}
	}

	tcp::socket socket_;
	void *data_;
	size_t data_size_;
};


template <typename MESSAGE_CALLBACK>
class ScreenServer {
public:
	ScreenServer(asio::io_context& io_context, const MESSAGE_CALLBACK &callback)
		: io_context_(io_context)
		, acceptor_(io_context, tcp::endpoint(tcp::v4(), SOCKET_SCREEN_PORT))
		, callback_(callback) {
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
			new_connection->start(callback_);
		}
		start_accept();
	}

	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	const MESSAGE_CALLBACK callback_;
};