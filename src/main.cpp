#include <csignal>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <iostream>
#include <stdexcept>
#include <string>

#include "server/server.hpp"
#include "debug.hpp"

std::mutex g_wait_mutex;
std::condition_variable g_wait_cv;
std::atomic<bool> g_shutdown_requested{false};

void signal_handler(int signum)
{
    DEBUG_LOG("\n[MAIN] Interrupt signal (" << signum << ") received. Initiating graceful shutdown...\n");
    g_shutdown_requested = true;
    g_wait_cv.notify_all();
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        throw std::invalid_argument("Usage: program conf_filepath");
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    const std::string& conf_filepath = argv[1];

    DEBUG_LOG("RobertServer running with arguments: " << conf_filepath << std::endl);

    if (!robert::sock_comm::initialize()) {
        std::cerr << "Failed to initialize sockets." << std::endl;
        return 1;
    }

    robert::server::Server server("*", 42069, conf_filepath);

    server.start();

    // block main thread until a signal is received
    {
        std::unique_lock<std::mutex> lock(g_wait_mutex);
        g_wait_cv.wait(lock, [] { return g_shutdown_requested.load(); });
    }

    server.stop();

    robert::sock_comm::cleanup();

    return 0;
}
