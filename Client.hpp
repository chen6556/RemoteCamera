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
        cv::VideoWriter* _writer = new cv::VideoWriter("./temp000.avi", cv::VideoWriter::fourcc('M','J','P', 'G'), 30, cv::Size(1280, 720)); 
        bool _if_write_video = false;

        void receive();
        void show();
        void close();
        void start_record();
        void stop_record();

    public:
        Client(boost::asio::io_context&, const char[], const char[]);
        ~Client();
};