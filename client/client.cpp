#include <iostream>
#include <asio.hpp>
using asio::ip::tcp;

int main()
{
    std::cout << "Welcome" << std::endl;
    try
    {
        std::string raw_ip_address = "192.168.1.10";
        unsigned short port_num = 13;
        asio::ip::tcp::endpoint
            ep(asio::ip::address::from_string(raw_ip_address),
               port_num);
        asio::io_service ios;
        asio::ip::tcp::socket socket(ios, ep.protocol());

        socket.connect(ep);

        for (;;)
        {
            char buf[128];
            asio::error_code error;

            size_t len = socket.read_some(asio::buffer(buf), error);

            if (error == asio::error::eof)
                break; // Connection closed cleanly by peer.
            else if (error)
                throw asio::system_error(error); // Some other error.

            std::cout.write(buf, len);
        }
    }
    catch (std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}