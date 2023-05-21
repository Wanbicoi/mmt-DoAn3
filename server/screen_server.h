#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "types.h"
using asio::ip::tcp;

class ScreenConnection : public std::enable_shared_from_this<ScreenConnection> {
public:
	typedef std::shared_ptr<ScreenConnection> pointer;

	static pointer create(asio::io_context& io_context, const ScreenInfo info, const std::function<FrameBuffer()> &callback) {
		return pointer(new ScreenConnection(io_context, info, callback));
	}

	tcp::socket& socket() {
		return socket_;
	}

	void start() {
		asio::error_code ignored_error;
		asio::async_write(socket_, asio::buffer(&info_, sizeof(info_)),
			std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

private:
	ScreenConnection(asio::io_context& io_context, const ScreenInfo info, const std::function<FrameBuffer()> &callback)
	: socket_(io_context)
	, info_(info)
	, callback_(callback) {}

	void write(void *data, int size) {
		asio::error_code error;
		asio::write(socket_, asio::buffer(data, size), error);
	}

	void handle_write(asio::error_code error) {
		if (!error) {
			FrameBuffer buffer = callback_();
			if (buffer.screen_changed) {
				write(&buffer, sizeof(FrameBuffer));
				if (buffer.mouse_changed) {
					write(buffer.mouse_data, buffer.mouse_size);
				}
				if (buffer.screen_changed) {
					write(buffer.screen_data, buffer.screen_size);
				}
				OperationCode opcode = FRAME_DATA;
				asio::async_write(socket_, asio::buffer(&opcode, sizeof(OperationCode)),
					std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1));
			}
			else {
				socket_.async_wait(asio::ip::tcp::socket::wait_write,
					std::bind(&ScreenConnection::handle_write, shared_from_this(), std::placeholders::_1));
			}
		}
	}

	tcp::socket socket_;
	const ScreenInfo info_;
	const std::function<FrameBuffer()> callback_;
	void *data_;
	size_t data_size_;
};


class ScreenServer {
public:
	ScreenServer(asio::io_context& io_context, const ScreenInfo info, const std::function<FrameBuffer()> &callback)
		: io_context_(io_context)
		, acceptor_(io_context, tcp::endpoint(tcp::v4(), SOCKET_SCREEN_PORT))
		, info_(info)
		, callback_(callback) {
		start_accept();
	}

private:
	void start_accept() {
		ScreenConnection::pointer new_connection = ScreenConnection::create(io_context_, info_, callback_);

		acceptor_.async_accept(new_connection->socket(),
				std::bind(&ScreenServer::handle_accept, this, new_connection, std::placeholders::_1 /*error*/));
	}

	void handle_accept(ScreenConnection::pointer new_connection, const asio::error_code& error) {
		if (!error) {
			new_connection->start();
		}
		start_accept();
	}

	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	ScreenInfo info_;
	const std::function<FrameBuffer()> callback_;
};