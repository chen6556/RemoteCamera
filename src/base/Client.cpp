#include "base/Client.hpp"
#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <chrono>

Client::Client(boost::asio::io_context & context, const char ip[], const char port[], bool gui)
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    _context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    _code.reserve(89600);
    _if_gui = gui;
    receive();
    _writer->release();
    boost::filesystem::remove("./temp000.avi");
}

Client::Client(boost::asio::io_context & context, const std::string ip, const std::string port, bool gui)
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    _context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    _code.reserve(89600);
    _if_gui = gui;
    receive();
    _writer->release();
    boost::filesystem::remove("./temp000.avi");
}

Client::~Client()
{
    _shutdown = true;
    if (_qr_detector != nullptr)
    {
        delete _qr_detector;
    }
    _writer->release();
    delete _writer;
    if (!_if_gui)
    {
        cv::destroyAllWindows();
    }
    if (_socket.is_open())
    {
        _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
        _socket.close();  
        // _socket.release(); // 此方法只支持windows10及更高版本,故弃用
    }
    if (!_context_ptr->stopped())
    {
        _context_ptr->stop();
    }
}

void Client::receive()
{
    _socket.send_to(boost::asio::buffer("Next Frame", 10), *_endpoints.begin());
    _socket.async_receive_from(boost::asio::buffer(_cache, 1024), _sender_endpoint,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
            if (!ec && std::strncmp("od", (char *)_cache, 2) == 0)
            {
                if (std::strncmp("odClose", (char *)_cache, 7) == 0)
                {
                    if (_if_gui)
                    {
                        _shutdown = true;
                    }
                    else
                    {
                        close();
                    }
                }
                else if (std::strncmp("odQR", (char *)_cache, 4) == 0)
                {
                    _if_decode_qr = !_if_decode_qr;
                    if (_qr_detector == nullptr)
                    {
                        _qr_detector = new cv::QRCodeDetector();
                    }       
                }       
                else if (std::strncmp("odRe", (char *)_cache, 4) == 0)
                {
                    start_record();
                }
                else if (std::strncmp("odStopRe", (char *)_cache, 8) == 0)
                {
                    stop_record();
                }
                else{
                    if (!_if_gui)
                    {
                        for (size_t i = 2; i < bytes_recvd; ++i)
                        {              
                            std::cout << _cache[i];
                        }
                        std::cout << std::endl;
                    }
                }
                std::fill_n(_cache, 64, '\0');
            }
            else if (!ec && std::strncmp("rp", (char *)_cache, 2) != 0)
            {
                show();
            }
            
            if (!_shutdown)
            {
                receive();
            }
        }); 
}

void Client::show()
{   
    _cache[5] = '\0';
    _length = std::atoi((char *)_cache);
    _code.clear();
    for (size_t count = 0; count < _length; count += _size)
    {
        _size = _socket.receive_from(boost::asio::buffer(_cache, 1024), _sender_endpoint);
        _code.insert(_code.end(), _cache, _cache + _size);
    }
    _frame = cv::imdecode(_code, cv::IMREAD_COLOR);
    if (_if_decode_qr)
    {
        std::vector<cv::Point> points;
        _qr_detector->detectAndDecode(_frame, points);
        if (!points.empty())
        {
            cv::polylines(_frame, points, true, cv::Scalar(49, 85, 252), 2);
        }
    }
    if (_if_write_video && !_frame.empty())
    {
        _writer->write(_frame);
    }
    if (_if_gui)
    {
        return;
    }
    cv::imshow("RemoteCamera", _frame);
    cv::waitKey(30);
    if (cv::getWindowProperty("RemoteCamera", cv::WND_PROP_VISIBLE) < 1)
    {
        close();
    }
}

void Client::close()
{
    _shutdown = true;
    if (!_if_gui)
    {
        std::cout << "Client will close." << std::endl;
    }
    _writer->release();
    if (_qr_detector != nullptr)
    {
        delete _qr_detector;
        _qr_detector = nullptr;
    }
    if (!_if_gui)
    {
        cv::destroyAllWindows();
    }
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    if (!_if_gui)
    {
        _context_ptr->stop();
    }
}

void Client::start_record()
{
    if (_if_write_video)
    {
        return;
    }
    if (!boost::filesystem::exists(_video_path))
    {
        boost::filesystem::create_directory(_video_path);
    }
    _socket.send_to(boost::asio::buffer("odSendPara", 10), *_endpoints.begin());
    char message[8];

    _size = _socket.receive_from(boost::asio::buffer(message, 8), _sender_endpoint);
    message[_size] = '\0';
    int fps = std::atoi(message);

    _size = _socket.receive_from(boost::asio::buffer(message, 8), _sender_endpoint);
    message[_size] = '\0';
    int width = std::atoi(message);

    _size = _socket.receive_from(boost::asio::buffer(message, 8), _sender_endpoint);
    message[_size] = '\0';
    int height = std::atoi(message);
    
    _size = std::count_if(boost::filesystem::directory_iterator(_video_path), boost::filesystem::directory_iterator(), 
                            [](const boost::filesystem::path& p){return boost::filesystem::is_regular_file(p);});
    _writer->open(_video_path.append("/video_").append(std::to_string(_size)).append(".avi"), cv::VideoWriter::fourcc('M','J','P','G'), fps, cv::Size(width, height));
    _if_write_video = true;
}

void Client::stop_record()
{
    if (_if_write_video)
    {
        _writer->release();
    }
    _if_write_video = false;
}

const cv::Mat& Client::frame() const
{
    return _frame;
}

void Client::set_video_path(const std::string& path)
{
    if (_if_write_video)
    {
        return;
    }
    _video_path = path;
}

void Client::set_video_path(const char path[])
{
    if (_if_write_video)
    {
        return;
    }
    _video_path = path;
}