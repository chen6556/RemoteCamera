#include "base/RemoteCamera.hpp"

int main(int, char**) {
    try
    {
        boost::property_tree::ptree config;
        boost::property_tree::read_json("./config.json", config);
        boost::asio::io_context context;
        short port = config.get<short>("port");

        RemoteCamera r(context, port);
        context.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}