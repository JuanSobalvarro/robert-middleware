#pragma once

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <basetsd.h>
    typedef SSIZE_T ssize_t;
    using socket_t = SOCKET;
    constexpr socket_t INVALID_SOCKET_FD = INVALID_SOCKET;
#else
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    using socket_t = int;
    constexpr socket_t INVALID_SOCKET_FD = -1;
#endif

namespace robert::sock_comm {

/**
 * @brief Initialize the socket subsystem (Required for Windows WSAStartup)
 * @return true if successful, false otherwise
 */
bool initialize();

/**
 * @brief Cleanup the socket subsystem (Required for Windows WSACleanup)
 */
void cleanup();

/**
 * @brief Cross-platform socket close
 */
void close_socket(socket_t fd);

/**
 * @brief Safely set both send and receive timeouts for a socket
 */
bool set_timeouts(socket_t fd, int timeout_ms);

} // namespace robert::sock_comm
