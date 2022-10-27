#pragma once

#include <opencv2/opencv.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <thread>

class RemoteCamera
{
    private:
        cv::VideoCapture _VideoCapture;
        boost::property_tree::ptree _Config;
        cv::Mat _Frame;
        bool _Running = true;
        std::thread _RecordThread;
        u_char _Cache[1024];
        char _Order[64];
        std::vector<u_char> code;
        std::vector<int> _params;
        int _quality;

        boost::asio::io_context* context_ptr;
        boost::asio::ip::udp::socket _Socket;
        boost::asio::ip::udp::endpoint _Sender;

        void send_frame();
        void release();
        void refresh_config();
        void close();
        void receive();
        void record();
        void report_parameters();
        // void download();

    public:
        RemoteCamera(boost::asio::io_context& context, short port);
        ~RemoteCamera();
};
