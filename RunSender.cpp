#include "Sender.hpp"
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: <host> <port>\n";
        return 1;
    }
    try
    {
        boost::asio::io_context context;

        Sender s(context, argv[1], argv[2]);

        context.run();
        
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}