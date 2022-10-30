#pragma once

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

class Client
{
    private:
        boost::asio::io_context* context_ptr;
        boost::asio::ip::udp::socket _socket;
        boost::asio::ip::udp::resolver _resolver;
        boost::asio::ip::udp::resolver::results_type _endpoints;
        boost::asio::ip::udp::endpoint _sender_endpoint;
        uchar cache[1024];
        std::vector<u_char> code;
        size_t length=0, size=0;
        cv::Mat frame;
        cv::QRCodeDetector* _qr_detector = nullptr;
        bool _if_decode_qr = false;

        void receive();
        void show();
        void close();

    public:
        Client(boost::asio::io_context&, const char[], const char[]);
        ~Client();
};