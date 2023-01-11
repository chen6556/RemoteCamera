#pragma once

#include <boost/asio.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

class Client
{
    private:
        boost::asio::ip::udp::socket _socket;
        boost::asio::ip::udp::resolver _resolver;
        boost::asio::ip::udp::resolver::results_type _endpoints;
        boost::asio::ip::udp::endpoint _sender_endpoint;
        uchar _cache[1024];
        std::vector<u_char> _code;
        size_t _length=0, _size=0;
        cv::Mat _frame;
        cv::QRCodeDetector* _qr_detector = nullptr;
        bool _if_decode_qr = false;
        cv::VideoWriter* _writer = new cv::VideoWriter("./temp000.avi", cv::VideoWriter::fourcc('M','J','P', 'G'), 30, cv::Size(1280, 720)); 
        bool _if_write_video = false;
        bool _if_gui = false, _shutdown = false;

        std::string _video_path = "./videos/";

        void receive();
        void show();
        void start_record();
        void stop_record();

    public:
        Client(boost::asio::io_context&, const char[], const char[], bool gui = false);
        Client(boost::asio::io_context&, const std::string, const std::string, bool gui = false);
        ~Client();
        const cv::Mat& frame() const;
        void close();
        void set_video_path(const std::string& path);
        void set_video_path(const char path[]);
};