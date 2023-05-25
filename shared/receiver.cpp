//
// receiver.cpp
// ~~~~~~~~~~~~
//
// Copyright (c) 2003-2023 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <array>
#include <iostream>
#include <string>
#include "asio.hpp"

class receiver {
public:
	receiver(asio::io_context& io_context,
			const asio::ip::address& listen_address,
			const asio::ip::address& multicast_address)
		: multicast_socket(io_context) {
		// Create the socket so that multiple may be bound to the same address.
		asio::ip::udp::endpoint listen_endpoint(asio::ip::make_address("0.0.0.0"), multicast_port);
		multicast_socket.open(listen_endpoint.protocol());
		multicast_socket.set_option(asio::ip::udp::socket::reuse_address(true));
		multicast_socket.bind(listen_endpoint);

		// Join the multicast group.
		multicast_socket.set_option(asio::ip::multicast::join_group(multicast_address));

		do_receive();
	}

private:
	void do_receive() {
		multicast_socket.async_receive_from(asio::buffer(data_), sender_endpoint_,
			[this](std::error_code ec, std::size_t length) {
				if (!ec) {
					std::cout.write(data_.data(), length);
					std::cout << std::endl;

					//do_receive();
				}
			});
	}

	asio::ip::udp::socket multicast_socket;
	asio::ip::udp::endpoint sender_endpoint_;
	std::array<char, 1024> data_;
};
