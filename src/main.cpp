#include <csignal>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>

#include "server/server.hpp"

std::mutex g_wait_mutex;
std::condition_variable g_wait_cv;
std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int signum)
{
    std::cout << "\n[MAIN] Interrupt signal (" << signum << ") received. Initiating graceful shutdown...\n";
    g_shutdown_requested = true;
    g_wait_cv.notify_all();
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        throw std::invalid_argument("Usage: program robots_conf_filepath users_filepath");
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string& robots_filepath = argv[1];
    const std::string& users_filepath = argv[2];

    std::cout << "RobertServer running with arguments: " << robots_filepath << " " << users_filepath << std::endl;

    robert::server::Server server("*", 42069);
    server.load_robots_from_file(robots_filepath);
    server.load_users_from_file(users_filepath);

    server.start();

    // block main thread until a signal is received
    {
        std::unique_lock<std::mutex> lock(g_wait_mutex);
        g_wait_cv.wait(lock, [] { return g_shutdown_requested.load(); });
    }

    server.stop();

    return 0;
}
