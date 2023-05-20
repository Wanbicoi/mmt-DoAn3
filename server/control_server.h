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
			std::bind(&ControlConnection::handle_read, shared_from_this(), std::placeholders::_1));
	}

private:
	ControlConnection(asio::io_context& io_context) : socket_(io_context) {}

	template <class T>
	T receive() {
		T obj;
		asio::read(socket_, asio::buffer(&obj, sizeof(obj)));
		return obj;
	}

	void send(void *data, int size) {
		asio::error_code error;
		asio::write(socket_, asio::buffer(data, size), error);
	}

	void send(std::string str) {
		asio::error_code error;
		int size = str.size();
		asio::write(socket_, asio::buffer(&size, sizeof(int)), error);
		asio::write(socket_, asio::buffer(str, size), error);
	}

	void handle_read(const asio::error_code& error) {
		if (!error) {
			std::cout << "Opcode: " << buf.opcode << std::endl;
			switch (buf.opcode) {
				case PROCESS_LIST: {
					auto processes = get_current_processes();
					buf.data = processes.size();
					send(&buf, sizeof(buf));
					for (auto &process: processes) {
						send(&process.pid, sizeof(int));
						send(process.name);
						send(&process.type, sizeof(char));
					}
					break;
				}
				case PROCESS_SUSPEND:
					suspend_process(buf.data);
					break;
				case PROCESS_RESUME:
					resume_process(buf.data);
					break;
				case PROCESS_KILL:
					terminate_process(buf.data);
					break;
				case MOUSE_MOVE: {
					MousePosition mp = receive<MousePosition>();
					mouse_move(mp.x, mp.y, mp.width, mp.height);
					break;
				}
				case MOUSE_LEFT_DOWN:
					break;
				case MOUSE_LEFT_UP:
					break;
				case MOUSE_MIDDLE_DOWN:
					break;
				case MOUSE_MIDDLE_UP:
					break;
				case MOUSE_RIGHT_DOWN:
					break;
				case MOUSE_RIGHT_UP:
					break;
				case MOUSE_WHEEL_V:
					break;
				case MOUSE_WHEEL_H:
					break;
			}
			asio::async_read(socket_, asio::buffer(&buf, sizeof(buf)), 
				std::bind(&ControlConnection::handle_read, shared_from_this(), std::placeholders::_1));
		}
	}

	void handle_write(asio::error_code error) {
		if (!error) {

		}
	}

	void void_write(asio::error_code error) {}

	tcp::socket socket_;
	int mouse_x = 0;
	int mouse_y = 0;
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