#pragma once

#include <boost/asio.hpp>
#include <string>

class Sender
{
    private:
        boost::asio::io_context* _context_ptr;
        boost::asio::ip::udp::socket _socket;
        boost::asio::ip::udp::resolver _resolver;
        boost::asio::ip::udp::resolver::results_type _endpoints;
        boost::asio::ip::udp::endpoint _sender_endpoint;
        char _cache[64];
        std::string _cmd;
        int _gui_cmd = -1;
        bool _if_gui = false;

        std::string _frame_path = "./frames/";
        bool _writing_frame = false;

        size_t _length = 0, _size = 0;
        void send();
        void receive();
        void help() const;
        void download();     
        void close();   
        void exit();

    public:
        Sender(boost::asio::io_context&, const char[], const char[], bool gui = false);
        Sender(boost::asio::io_context&, const std::string, const std::string, bool gui = false);
        ~Sender();
        void get_cmd(const int& value);
        void set_frame_path(const std::string& path);
        void set_frame_path(const char path[]);
};