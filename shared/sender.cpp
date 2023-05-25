#include <asio.hpp>

class MulticastServer {
public:
	MulticastServer(asio::io_context& io_context, const asio::ip::address& multicast_address)
		: endpoint_(multicast_address, multicast_port)
		, socket_(io_context, endpoint_.protocol())
		, timer_(io_context) {
		do_send();
	}

private:
	void do_send() {

		socket_.async_send_to(asio::buffer(message_), endpoint_,
			[this](std::error_code ec, std::size_t /*length*/) {
				if (!ec)
					do_timeout();
			});
	}

	void do_timeout() {
		timer_.expires_after(std::chrono::seconds(1));
		timer_.async_wait([this](std::error_code ec) {
			if (!ec)
				do_send();
		});
	}

private:
	asio::ip::udp::endpoint endpoint_;
	asio::ip::udp::socket socket_;
	asio::steady_timer timer_;
	std::string message_;
};

int main(int argc, char* argv[])
{
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: MulticastServer <multicast_address>\n";
			std::cerr << "  For IPv4, try:\n";
			std::cerr << "    MulticastServer 239.255.0.1\n";
			std::cerr << "  For IPv6, try:\n";
			std::cerr << "    MulticastServer ff31::8000:1234\n";
			return 1;
		}

		asio::io_context io_context;
		MulticastServer s(io_context, asio::ip::make_address(argv[1]));
		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}
