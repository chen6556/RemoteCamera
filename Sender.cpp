#include "Sender.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>

Sender::Sender(boost::asio::io_context & context, const char ip[], const char port[])
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    receive();
    send();
}

Sender::~Sender()
{
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    context_ptr->stop();
}

void Sender::send()
{
    std::cout << "Input Order: ";
    cmd.clear();
    std::getline(std::cin, cmd);
    if (cmd.length() > 64 || cmd == "Help")
    {
        cmd.clear();
        help();
    }
    else if (cmd == "Exit")
    {
        exit();
    }
    else if (cmd == "Close")
    {
        close();
    }
    
    _socket.async_send_to(boost::asio::buffer(cmd, cmd.length()), *_endpoints.begin(), 
                        [this](boost::system::error_code ec, std::size_t bytes_recvd)
                        {
                            send();
                        });
}

void Sender::receive()
{
    _socket.async_receive_from(
        boost::asio::buffer(cache, 64), _sender_endpoint,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec)
          {
            if (std::strncmp("rp", cache, 2) == 0)
            {
                for (size_t i = 2; i < bytes_recvd; ++i)
                {              
                    std::cout << cache[i];
                }
                std::fill_n(cache, 64, '\0');
            }
          }
          receive();
        }); 
}

void Sender::help() const
{
    std::cout << "Help:" << std::endl;
    std::cout << "    - Help : Get help." << std::endl;
    std::cout << "    - Exit : Close Sender." << std::endl;
    std::cout << "    - Para : Get RemoteCamera parameters." << std::endl;
    std::cout << "    - ReConfig : RemoteCamera reloads configs." << std::endl;
    std::cout << "    - CloseCam : Close RemoteCamera." << std::endl;
    std::cout << "    - Close : Close Sender, Client and RemoteCamera." << std::endl;
}

void Sender::exit()
{
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    context_ptr->stop();
}

void Sender::download()
{
    cache[6] = '\0';
    length = std::atoi(cache);
    std::vector<u_char> code;

    for (size_t count = 0; count < length; count += size)
    {
        size = _socket.receive_from(boost::asio::buffer(cache, 64), _sender_endpoint);
        code.insert(code.end(), cache, cache + size);
    }
    cv::Mat frame = cv::imdecode(code, cv::IMREAD_COLOR);
    cv::imshow("RemoteCamera", frame);
    cv::waitKey();
}

void Sender::close()
{
    _socket.async_send_to(boost::asio::buffer("CloseCam", 8), *_endpoints.begin(), [](boost::system::error_code, std::size_t){});
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    context_ptr->stop();
}