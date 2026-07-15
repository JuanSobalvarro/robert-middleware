#include <stdexcept>
#include <string>

#include "server/server.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        throw std::invalid_argument("Usage: program robots_conf_filepath");
    }

    const std::string& filepath = argv[1];

    std::cout << "RobertServer running with arguments: " << filepath << std::endl;

    robert::server::Server server("*", 42069);
    server.load_robots_from_file(filepath);

    server.start();

    server.wait();

    return 0;
}
