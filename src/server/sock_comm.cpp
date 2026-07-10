#include <iostream>

#include "sock_comm.hpp"

namespace robert::sock_comm {

bool initialize()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "[SOCK_COMM] WSAStartup failed.\n";
        return false;
    }
#endif
    return true;
}

void cleanup()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

void close_socket(socket_t fd)
{
    if (fd == INVALID_SOCKET_FD) return;

#ifdef _WIN32
    closesocket(fd);
#else
    close(fd);
#endif
}

bool set_timeouts(socket_t fd, int timeout_ms)
{
    if (fd == INVALID_SOCKET_FD) return false;

#ifdef _WIN32
    // Windows expects a DWORD representing milliseconds passed as a const char*
    DWORD timeout = static_cast<DWORD>(timeout_ms);
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) < 0)
        return false;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout)) < 0)
        return false;
#else
    // POSIX expects a struct timeval passed as a const void*
    struct timeval tv{};
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
        return false;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
        return false;
#endif

    return true;
}

} // namespace robert::sock_comm
