#pragma once

#include <boost/asio.hpp>
#include <string>

class Sender
{
    private:
        boost::asio::io_context* context_ptr;
        boost::asio::ip::udp::socket _socket;
        boost::asio::ip::udp::resolver _resolver;
        boost::asio::ip::udp::resolver::results_type _endpoints;
        boost::asio::ip::udp::endpoint _sender_endpoint;
        char cache[64];
        std::string cmd;

        size_t length = 0, size = 0;
        void send();
        void receive();
        void help() const;
        void download();
        void exit();
        void close();

    public:
        Sender(boost::asio::io_context&, const char[], const char[]);
        ~Sender();
};