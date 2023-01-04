#include "RemoteCamera.hpp"
#include <vector>
#include <iostream>

RemoteCamera::RemoteCamera(boost::asio::io_context& context, short port)
    : _socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    _context_ptr = &context;
    boost::property_tree::read_json("./config.json", _config);
    _video_capture.open(_config.get<u_char>("index"));
    _video_capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','P','4','2'));
    _video_capture.set(cv::CAP_PROP_FPS, _config.get<u_char>("fps"));
    _video_capture.set(cv::CAP_PROP_FRAME_WIDTH, _config.get<u_int>("width"));
    _video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, _config.get<u_int>("height"));
    
    _params.push_back(cv::IMWRITE_JPEG_QUALITY);
    _params.push_back(_config.get<u_char>("quality") < 85 ? _config.get<u_char>("quality") : 85);
    _quality = _params[1];
    _code.reserve(89600);
    record();
    receive();
    std::cout << "Ready to send frame." << std::endl;
}

RemoteCamera::~RemoteCamera()
{
    _running = false;
    _video_capture.release();
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _socket.close();
    // _socket.release(); // 此方法只支持windows10及更高版本,故弃用
    _context_ptr->stop();
}

void RemoteCamera::release()
{
    _running = false;
    _video_capture.release();
}

void RemoteCamera::refresh_config()
{
    _running = false;
    _config.clear();
    boost::property_tree::read_json("./config.json", _config);
    _video_capture.release();
    _video_capture = cv::VideoCapture(_config.get<u_char>("index"));
    _video_capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','P','4','2'));
    _video_capture.set(cv::CAP_PROP_FPS, _config.get<u_char>("fps"));
    _video_capture.set(cv::CAP_PROP_FRAME_WIDTH, _config.get<u_int>("width"));
    _video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, _config.get<u_int>("height"));
    _params[1] = _config.get<u_char>("quality") < 85 ? _config.get<u_char>("quality") : 85;
    record();
    std::cout << "Refresh config successfully." << std::endl;
    _socket.send_to( boost::asio::buffer("rpRefresh config successfully.", 30), _sender);
}

void RemoteCamera::record()
{
    _running = true;
    _record_thread = std::thread( [this](){while (_running){_video_capture >> _frame;}} );
    _record_thread.detach();
}

void RemoteCamera::close()
{
    _socket.async_receive_from(boost::asio::buffer(_order, 64), _sender, [](boost::system::error_code, std::size_t){}); 
    _running = false;
    _video_capture.release();    
    _socket.async_send_to(boost::asio::buffer("odClose", 7), _sender, [](boost::system::error_code, std::size_t){});
    _socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _context_ptr->stop();
}

void RemoteCamera::receive()
{
    _socket.async_receive_from(
        boost::asio::buffer(_order, 64), _sender,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec)
          {
            if (_running && std::strncmp("Next Frame", _order, 10) == 0)
            {
                if (_order_length > 0)
                {
                    _socket.async_send_to(boost::asio::buffer(_message, _order_length), _sender,
                            [](boost::system::error_code, std::size_t ){});
                    _order_length = 0;
                }
                else
                {
                    send_frame(); 
                } 
            }
            else if (std::strncmp("od", _order, 2) == 0)
            {
                if (std::strncmp("odPara", _order, 6) == 0)
                {
                    report_parameters();
                }
                else if (std::strncmp("odSendPara", _order, 10) == 0)
                {
                    send_parameters();
                }
                else if (std::strncmp("odCloseCam", _order, 10) == 0) 
                {
                    close();
                }
                else if (std::strncmp("odReConfig", _order, 10) == 0)
                {
                    refresh_config();
                }
                else if (std::strncmp("odDownload", _order, 10) == 0)
                {
                    download();
                }
                else
                {
                    _order_length = bytes_recvd;
                    std::strcpy(_message, _order);
                }
            }
          }
          std::fill_n(_order, 64, '\0');
          receive();
        }); 
}

void RemoteCamera::send_frame()
{   
    if (_frame.empty())
    {
        return;
    }
    _code.clear();
    cv::imencode(".jpg", _frame, _code, _params);
    size_t length = _code.size();
    while (length > 89600)
    {
        _code.clear();
        --_params[1];
        cv::imencode(".jpg", _frame, _code, _params);
        length = _code.size();
    }
    _params[1] = _quality;
    _socket.send_to( boost::asio::buffer(std::to_string(length), std::to_string(length).length()), _sender);
    for (size_t i = 0, end = length/1024; i < end; ++i) 
    {
        std::memcpy(_cache, &_code[i*1024], 1024 * sizeof(_code[0]));
        _socket.send_to( boost::asio::buffer(_cache, 1024), _sender);
    }
    if (length % 1024 > 0)
    {
        std::memcpy(_cache, &_code[length - length % 1024], (length % 1024) * sizeof(_code[0]));
        _socket.send_to( boost::asio::buffer(_cache, length % 1024), _sender);
    }
}

void RemoteCamera::report_parameters()
{
    std::string para("rp");
    para.append("Quality: ");
    para.append(std::to_string(_quality));
    para.append("\n");

    para.append("FPS: ");
    para.append(std::to_string(_config.get<u_char>("fps")));
    para.append("\n");

    para.append("Width: ");
    para.append(std::to_string(_config.get<u_int>("width")));
    para.append("\n");

    para.append("Height: ");
    para.append(std::to_string(_config.get<u_int>("height")));
    para.append("\n");

    _socket.send_to( boost::asio::buffer(para, para.length()), _sender);
}

void RemoteCamera::send_parameters()
{
    _socket.send_to(boost::asio::buffer(std::to_string(_config.get<int>("fps")), std::to_string(_config.get<int>("fps")).length()), _sender);
    _socket.send_to(boost::asio::buffer(std::to_string(_config.get<int>("width")), std::to_string(_config.get<int>("width")).length()), _sender);
    _socket.send_to(boost::asio::buffer(std::to_string(_config.get<int>("height")), std::to_string(_config.get<int>("height")).length()), _sender);
}

void RemoteCamera::download()
{
    _params[1] = 100;
    std::vector<u_char> temp;
    cv::imencode(".jpg", _frame, temp, _params);
    _params[1] = _quality;
    size_t length = temp.size();
    _socket.send_to( boost::asio::buffer(std::to_string(length), std::to_string(length).length()), _sender);
    _socket.send_to( boost::asio::buffer(std::to_string(length), std::to_string(length).length()), _sender);
    for (size_t i = 0, end = length/64; i < end; ++i) 
    {
        std::memcpy(_order, &temp[i*64], 64 * sizeof(temp[0]));
        _socket.send_to( boost::asio::buffer(_order, 64), _sender);
    }
    if (length % 64 > 0)
    {
        std::memcpy(_order, &temp[length - length % 64], (length % 64) * sizeof(temp[0]));
        _socket.send_to( boost::asio::buffer(_order, length % 64), _sender);
    }
}