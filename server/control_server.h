#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "types.h"
#include "processes.h"
#include "input.h"
using asio::ip::tcp;

class ControlConnection : public std::enable_shared_from_this<ControlConnection> {
public:
	typedef std::shared_ptr<ControlConnection> pointer;

	static pointer create(asio::io_context& io_context) {
		return pointer(new ControlConnection(io_context));
	}

	tcp::socket& socket() {
		return socket_;
	}

	template <typename MESSAGE_CALLBACK>
	void start(const MESSAGE_CALLBACK &callback) {
		asio::async_read(socket_, asio::buffer(&buf, sizeof(buf)), 
			std::bind(&ControlConnection::handle_read, shared_from_this(),
			std::placeholders::_1 /*error*/, std::placeholders::_2 /*bytes_transferred*/));

		// asio::async_write(socket_, asio::buffer(&buf, sizeof(buf)),
		// 		std::bind(&ControlConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

private:
	ControlConnection(asio::io_context& io_context) : socket_(io_context) {}

	void send(void *data, int size) {
		asio::async_write(socket_, asio::buffer(data, size),
			std::bind(&ControlConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

	void send(std::string str, int size) {
		asio::async_write(socket_, asio::buffer(str, size),
			std::bind(&ControlConnection::handle_write, shared_from_this(), std::placeholders::_1 /*error*/));
	}

	void send(std::string str) {
		int size = str.size();
		send(&size, sizeof(int));
		send(str, size);
	}

	void handle_read(const asio::error_code& error, std::size_t bytes_transferred) {
		if (!error) {
			std::cout << "Opcode: " << buf.opcode << std::endl;
			switch (buf.opcode) {
				case PROCESS_LIST: {
					auto processes = get_current_processes();
					buf.size = processes.size();
					send(&buf, sizeof(buf));
					for (auto &process: processes) {
						send(&process.pid, sizeof(int));
						send(process.name);
						send(&process.type, sizeof(char));
					}
					break;
				}

			}
			asio::async_read(socket_, asio::buffer(&buf, sizeof(buf)), 
				std::bind(&ControlConnection::handle_read, shared_from_this(),
					std::placeholders::_1 /*error*/, std::placeholders::_2 /*bytes_transferred*/));
		}
	}

	void handle_write(asio::error_code error) {
		if (!error) {
			//asio::write(socket_, asio::buffer(data_, data_size_), error);
		}
	}

	tcp::socket socket_;
	ControlBuffer buf;
};


template <typename MESSAGE_CALLBACK>
class ControlServer {
public:
	ControlServer(asio::io_context& io_context, const MESSAGE_CALLBACK &callback)
		: io_context_(io_context)
		, acceptor_(io_context, tcp::endpoint(tcp::v4(), SOCKET_CONTROL_PORT))
		, callback_(callback) {
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
			new_connection->start(callback_);
		}
		start_accept();
	}

	asio::io_context& io_context_;
	tcp::acceptor acceptor_;
	const MESSAGE_CALLBACK callback_;
};