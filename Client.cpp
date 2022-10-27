#include "Client.hpp"
#include <string>
#include <iostream>

Client::Client(boost::asio::io_context & context, const char ip[], const char port[])
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    code.reserve(89600);
    receive();
}

Client::~Client()
{
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    context_ptr->stop();
}

void Client::receive()
{
    _socket.send_to(boost::asio::buffer("Next Frame", 10), *_endpoints.begin());
    _socket.async_receive_from(boost::asio::buffer(cache, 1024), _sender_endpoint,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
            if (!ec && std::strncmp("od", (char *)cache, 2) == 0)
            {
                if (std::strncmp("odClose", (char *)cache, 7) == 0)
                {
                    close();
                }
            }
            else if (!ec && std::strncmp("rp", (char *)cache, 2) != 0)
            {
                show();
            }
            
            receive();
        }); 
}

void Client::show()
{   
    cache[5] = '\0';
    length = std::atoi((char *)cache);
    code.clear();
    for (size_t count = 0; count < length; count += size)
    {
        size = _socket.receive_from(boost::asio::buffer(cache, 1024), _sender_endpoint);
        code.insert(code.end(), cache, cache + size);
    }

    frame = cv::imdecode(code, cv::IMREAD_COLOR);
    cv::imshow("RemoteCamera", frame);
    cv::waitKey(30);
    if (cv::getWindowProperty("RemoteCamera", cv::WND_PROP_VISIBLE) < 1)
    {
        cv::destroyAllWindows();
        _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
        _socket.close();
        context_ptr->stop();
    }
}

void Client::close()
{
    std::cout << "Client will close." << std::endl;
    cv::destroyAllWindows();
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    context_ptr->stop();
}