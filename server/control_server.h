#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <asio.hpp>
#include "types.h"
#include "processes.h"
#include "input.h"
#include "fs.h"
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
		asio::async_read(socket_, asio::buffer(&opcode, sizeof(OperationCode)), 
			std::bind(&ControlConnection::handle_read, shared_from_this(), std::placeholders::_1));
	}

private:
	ControlConnection(asio::io_context& io_context) : socket_(io_context) {}

	template <class T>
	T read() {
		T obj;
		asio::read(socket_, asio::buffer(&obj, sizeof(obj)));
		return obj;
	}

	std::string readString() {
		int size = read<int>();
		std::string str;
		str.resize(size);
		asio::read(socket_, asio::buffer(str, size));
		return str;
	}

	void write(void *data, int size) {
		asio::error_code error;
		asio::write(socket_, asio::buffer(data, size), error);
	}

	void write(std::string str) {
		asio::error_code error;
		int size = str.size();
		asio::write(socket_, asio::buffer(&size, sizeof(int)), error);
		asio::write(socket_, asio::buffer(str, size), error);
	}

	void handle_read(const asio::error_code& error) {
		if (!error) {
			std::cout << "Opcode: " << opcode << std::endl;
			switch (opcode) {
				case PROCESS_LIST: {
					auto processes = get_current_processes();
					int size = processes.size();
					write(&size, sizeof(int));
					for (auto &process: processes) {
						write(&process.pid, sizeof(int));
						write(process.name);
						write(&process.type, sizeof(char));
					}
					break;
				}
				case PROCESS_SUSPEND:
					suspend_process(read<int>());
					break;
				case PROCESS_RESUME:
					resume_process(read<int>());
					break;
				case PROCESS_KILL:
					terminate_process(read<int>());
					break;
				case MOUSE_MOVE: {
					MousePosition mp = read<MousePosition>();
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
				case FS_INIT: {
					write(filesystem_get_default_location());
					break;
				}
				case FS_LIST: {
					std::string path = readString();
					auto files_list = filesystem_list(path);
					int size = files_list.size();
					write(&size, sizeof(int));
					for (auto &entry: files_list) {
						write(entry.name);
						write(&entry.type, sizeof(char));
					}
					break;
				}
				case FS_COPY:
					break;
				case FS_MOVE:
					break;
				case FS_WRITE:
					break;
				case FS_ASK_OPTION:
					break;
				case FS_DELETE:
					break;
			}
			asio::async_read(socket_, asio::buffer(&opcode, sizeof(OperationCode)), 
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
	OperationCode opcode;
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