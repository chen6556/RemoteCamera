#pragma once

#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <thread>

class RemoteCamera
{
    private:
        cv::VideoCapture _video_capture;
        boost::property_tree::ptree _config;
        cv::Mat _frame;
        bool _running = true;
        std::thread _record_thread;
        u_char _cache[1024];
        char _order[64];
        char _message[64];
        size_t _order_length;
        std::vector<u_char> _code;
        std::vector<int> _params;
        int _quality;

        boost::asio::io_context* _context_ptr;
        boost::asio::ip::udp::socket _socket;
        boost::asio::ip::udp::endpoint _sender;

        void send_frame();
        void release();
        void refresh_config();
        void close();
        void receive();
        void record();
        void report_parameters();
        void send_parameters();
        void download();

    public:
        RemoteCamera(boost::asio::io_context& context, short port);
        ~RemoteCamera();
};
