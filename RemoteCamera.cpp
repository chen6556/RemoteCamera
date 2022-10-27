#include "RemoteCamera.hpp"
#include <vector>
#include <iostream>

RemoteCamera::RemoteCamera(boost::asio::io_context& context, short port)
    : _Socket(context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port))
{
    context_ptr = &context;
    boost::property_tree::read_json("./config.json", _Config);
    _VideoCapture.open(_Config.get<u_char>("index"));
    _VideoCapture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','P','4','2'));
    _VideoCapture.set(cv::CAP_PROP_FPS, _Config.get<u_char>("fps"));
    _VideoCapture.set(cv::CAP_PROP_FRAME_WIDTH, _Config.get<u_int>("width"));
    _VideoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, _Config.get<u_int>("height"));
    
    _params.push_back(cv::IMWRITE_JPEG_QUALITY);
    _params.push_back(_Config.get<u_char>("quality") < 85 ? _Config.get<u_char>("quality") : 85);
    _quality = _params[1];
    code.reserve(89600);
    record();
    receive();
    std::cout << "Ready to send frame." << std::endl;
}

RemoteCamera::~RemoteCamera()
{
    _Running = false;
    _VideoCapture.release();
    _Socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
    _Socket.close();
    context_ptr->stop();
}

void RemoteCamera::release()
{
    _Running = false;
    _VideoCapture.release();
}

void RemoteCamera::refresh_config()
{
    _Running = false;
    _Config.clear();
    boost::property_tree::read_json("./config.json", _Config);
    _VideoCapture.release();
    _VideoCapture = cv::VideoCapture(_Config.get<u_char>("index"));
    _VideoCapture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M','P','4','2'));
    _VideoCapture.set(cv::CAP_PROP_FPS, _Config.get<u_char>("fps"));
    _VideoCapture.set(cv::CAP_PROP_FRAME_WIDTH, _Config.get<u_int>("width"));
    _VideoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, _Config.get<u_int>("height"));
    _params[1] = _Config.get<u_char>("quality") < 85 ? _Config.get<u_char>("quality") : 85;
    record();
    std::cout << "Refresh config successfully." << std::endl;
    _Socket.send_to( boost::asio::buffer("rpRefresh config successfully.", 30), _Sender);
}

void RemoteCamera::record()
{
    _Running = true;
    _RecordThread = std::thread( [this](){while (_Running){_VideoCapture >> _Frame;}} );
    _RecordThread.detach();
}

void RemoteCamera::close()
{
    _Socket.async_receive_from(
        boost::asio::buffer(_Order, 64), _Sender,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec)
          {
            if (_Running && std::strncmp("Next Frame", _Order, 10) == 0)
            {
                _Socket.async_send_to(boost::asio::buffer("Close", 5), _Sender, [](boost::system::error_code, std::size_t){});
            }
            _Socket.shutdown(boost::asio::ip::udp::socket::shutdown_both);
            _Socket.close();
            context_ptr->stop();
            std::fill_n(_Order, 64, '\0');
          }
        }); 
    _Running = false;
    _VideoCapture.release();    
}

void RemoteCamera::receive()
{
    _Socket.async_receive_from(
        boost::asio::buffer(_Order, 64), _Sender,
        [this](boost::system::error_code ec, std::size_t bytes_recvd)
        {
          if (!ec)
          {
            if (_Running && std::strncmp("Next Frame", _Order, 10) == 0)
            {
                send_frame(); 
            }
            else if (std::strncmp("Para", _Order, 4) == 0)
            {
                report_parameters();
            }
            else if (std::strncmp("CloseCam", _Order, 8) == 0) 
            {
                close();
            }
            else if (std::strncmp("ReConfig", _Order, 8) == 0)
            {
                refresh_config();
            }
            std::fill_n(_Order, 64, '\0');
          }
          receive();
        }); 
}

void RemoteCamera::send_frame()
{   
    if (_Frame.empty())
    {
        return;
    }
    code.clear();
    cv::imencode(".jpg", _Frame, code, _params);
    size_t length = code.size();
    while (length > 89600)
    {
        code.clear();
        --_params[1];
        cv::imencode(".jpg", _Frame, code, _params);
        length = code.size();
    }
    _params[1] = _quality;
    _Socket.send_to( boost::asio::buffer(std::to_string(length), std::to_string(length).length()), _Sender);
    for (size_t i = 0, end = length/1024; i < end; ++i) 
    {
        std::memcpy(_Cache, &code[i*1024], 1024 * sizeof(code[0]));
        _Socket.send_to( boost::asio::buffer(_Cache, 1024), _Sender);
    }
    if (length % 1024 > 0)
    {
        std::memcpy(_Cache, &code[length - length % 1024], (length % 1024) * sizeof(code[0]));
        _Socket.send_to( boost::asio::buffer(_Cache, length % 1024), _Sender);
    }
}

void RemoteCamera::report_parameters()
{
    std::string para("rp");
    para.append("Quality: ");
    para.append(std::to_string(_quality));
    para.append("\n");

    para.append("FPS: ");
    para.append(std::to_string(_Config.get<u_char>("fps")));
    para.append("\n");

    para.append("Width: ");
    para.append(std::to_string(_Config.get<u_int>("width")));
    para.append("\n");

    para.append("Height: ");
    para.append(std::to_string(_Config.get<u_int>("height")));
    para.append("\n");

    _Socket.send_to( boost::asio::buffer(para, para.length()), _Sender);
}

/*void RemoteCamera::download()
{
    std::vector<int> temp_params = {cv::IMWRITE_JPEG_QUALITY, 100};
    std::vector<u_char> temp;
    cv::imencode(".jpg", _Frame, temp, temp_params);
    size_t length = temp.size();

    _params[1] = _quality;
    _Socket.send_to( boost::asio::buffer(std::to_string(length), std::to_string(length).length()), _Sender);
    for (size_t i = 0, end = length/64; i < end; ++i) 
    {
        std::memcpy(_Order, &temp[i*64], 64 * sizeof(temp[0]));
        _Socket.send_to( boost::asio::buffer(_Order, 64), _Sender);
    }
    if (length % 64 > 0)
    {
        std::memcpy(_Order, &temp[length - length % 64], (length % 64) * sizeof(temp[0]));
        _Socket.send_to( boost::asio::buffer(_Order, length % 64), _Sender);
    }
}*/