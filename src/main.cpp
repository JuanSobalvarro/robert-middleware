#include <stdexcept>
#include <string>

#include "server/server.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        throw std::invalid_argument("Usage: program robots_conf_filepath users_filepath");
    }

    const std::string& robots_filepath = argv[1];
    const std::string& users_filepath = argv[2];

    std::cout << "RobertServer running with arguments: " << robots_filepath << std::endl;

    robert::server::Server server("*", 42069);
    server.load_robots_from_file(robots_filepath);
    server.load_users_from_file(users_filepath);

    server.start();

    server.wait();

    return 0;
}
