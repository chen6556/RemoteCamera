#include "Sender.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>

Sender::Sender(boost::asio::io_context & context, const char ip[], const char port[], bool gui)
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    _context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    _if_gui = gui;
    receive();
    send();
}

Sender::Sender(boost::asio::io_context & context, const std::string ip, const std::string port, bool gui)
: _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 0)), _resolver(context)
{
    _context_ptr = &context;
    _endpoints = _resolver.resolve(boost::asio::ip::udp::v4(), ip, port);
    _if_gui = gui;
    receive();
    send();
}

Sender::~Sender()
{
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

void Sender::send()
{
    _cmd.clear();
    if (!_if_gui)
    {
        std::cout << "Input Order: ";
        std::getline(std::cin, _cmd);
    }
    else
    {
        switch (_gui_cmd)
        {
        case -2:
            _cmd = "Close";
            break;
        case 0:
            _cmd = "Download\0";
            break;
        case 1:
            _cmd = "Re\0";
            break;
        case 2:
            _cmd = "StopRe\0";
            break;
        default:
            break;
        }
        _gui_cmd = -1;
    }
    if (_cmd.length() > 62 || _cmd == "Help")
    {
        _cmd.clear();
        if (!_if_gui)
        {
            help();
        }
    }
    else if (_cmd == "Exit")
    {
        exit();
    }
    else if (_cmd == "Close")
    {
        close();
        return;
    }
    
    if (_cmd.empty())
    {
        _socket.async_send_to(boost::asio::buffer(std::string(""), 0), *_endpoints.begin(), 
                    [this](boost::system::error_code, std::size_t)
                    {
                        send();
                    });
    }
    else
    {
        _socket.async_send_to(boost::asio::buffer(std::string("od").append(_cmd).c_str(), 2+_cmd.length()), *_endpoints.begin(), 
                        [this](boost::system::error_code ec, std::size_t bytes_recvd)
                        {
                            if (!ec && _cmd == "Download")
                            {
                                download();
                            }
                            send();
                        });  
    }
}

void Sender::receive()
{
    _socket.async_receive_from(
        boost::asio::buffer(_cache, 64), _sender_endpoint,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec)
          {
            if (std::strncmp("rp", _cache, 2) == 0)
            {
                if (!_if_gui)
                {
                    for (size_t i = 2; i < bytes_recvd; ++i)
                    {              
                        std::cout << _cache[i];
                    }
                }
                std::fill_n(_cache, 64, '\0');
            }
          }
          receive();
        }); 
}

void Sender::help() const
{
    if (_if_gui)
    {
        return;
    }
    std::cout << "Help:" << std::endl;
    std::cout << "    - Exit : Close Sender." << std::endl;
    std::cout << "    - Para : Get RemoteCamera parameters." << std::endl;
    std::cout << "    - ReConfig : RemoteCamera reloads configs." << std::endl;
    std::cout << "    - Download : Download current frame." << std::endl;
    std::cout << "    - QR : Turn on or turn off QR detector." << std::endl;
    std::cout << "    - Re : Write frames to video." << std::endl;
    std::cout << "    - StopRe : Stop writing frames to video." << std::endl;
    std::cout << "    - CloseCam : Close RemoteCamera." << std::endl;
    std::cout << "    - Close : Close Sender, Client and RemoteCamera." << std::endl;
}

void Sender::exit()
{
    if (_socket.is_open())
    {
        _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    }
    if (!_context_ptr->stopped())
    {
        _context_ptr->stop();
    }
}

void Sender::download()
{
    _socket.receive_from(boost::asio::buffer(_cache, 64), _sender_endpoint);
    _cache[6] = '\0';
    _length = std::atoi(_cache);
    std::vector<u_char> code;
    code.reserve(163840);
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 100};
    for (size_t count = 0; count < _length; count += _size)
    {
        _size = _socket.receive_from(boost::asio::buffer(_cache, 64), _sender_endpoint);
        code.insert(code.end(), _cache, _cache + _size);
    }
    cv::Mat frame = cv::imdecode(code, cv::IMREAD_COLOR);
    if (!boost::filesystem::exists("./frames/"))
    {
        boost::filesystem::create_directory("./frames/");
    }
    _size = std::count_if(boost::filesystem::directory_iterator("./frames/"), boost::filesystem::directory_iterator(), 
                            [](const boost::filesystem::path& p){return boost::filesystem::is_regular_file(p);});
    cv::imwrite(std::string("./frames/frame_").append(std::to_string(_size)).append(".png"), frame, params);
}

void Sender::close()
{
    _socket.async_send_to(boost::asio::buffer("odCloseCam", 10), *_endpoints.begin(), [](boost::system::error_code, std::size_t){});
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    if (!_if_gui)
    {
        _context_ptr->stop();
    }
}

void Sender::get_cmd(const int& value)
{
    switch (value)
    {
    case -2: // Close
    case 0: // Download
    case 1: // Re
    case 2: // StopRe
        _gui_cmd = value;
        break;
    default:
        _gui_cmd = -1;
        break;
    }
}