#pragma once

#include <future>
#include <queue>

#include "commands/commands.hpp"
#include "network/sock_comm.hpp"

namespace robert::robot {

/*
* @brief Represents a work item for the robot, containing a command and a promise for the response.
*/
struct RobotWorkItem {
    commands::RapidRequest command;
    std::shared_ptr<std::promise<std::vector<uint8_t>>> response_promise;
};

/*
 * @brief The Robot class handles communication with a robot over raw socket communication.
 *
 * The Robot class provides methods for starting and stopping a session, checking connection status, and queuing requests to be sent to the robot.
 * Relying on future to handle asynchronous responses.
 */
class Robot {
public:
    /*
     * @brief Constructs a Robot instance with the specified host, port, and timeout.
     *
     * @param host The hostname or IP address of the robot.
     * @param port The port number to use for the socket connection.
     * @param timeout_ms The timeout in milliseconds for socket operations (default is 5000ms).
     */
    Robot(const std::string& host, int port, int timeout_ms = 5000);
    ~Robot() noexcept;

    Robot(const Robot&) = delete;
    Robot& operator=(const Robot&) = delete;

    void start_session();
    void stop_session();

    bool is_connected() const;

    /*
     * @brief Queues a request to be sent to the robot and returns a future that will be resolved when the response is received.
     *
     * @param request The request to be sent to the robot.
     * @return A future that will be resolved when the response is received.
     */
    std::future<std::vector<uint8_t>> queue_request(const commands::RapidRequest& request);

private:
    void worker_loop();

    bool send_request(const commands::RapidRequest& request);
    std::vector<uint8_t> receive_buffer();
    std::vector<uint8_t> send_and_receive(const commands::RapidRequest& request);

    bool attempt_connection();

    std::string host_;
    int port_;

    // socket owned by Robot
    socket_t socket_fd_{INVALID_SOCKET_FD};
    int socket_timeout_ms_{35000}; // 35 seconds timeout for send/recv

    // worker thread
    std::atomic<bool> running_{false};
    std::thread worker_thread_;
    std::mutex socket_mutex_;

    // request queue
    std::queue<RobotWorkItem> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;

    // connection state
    std::atomic<bool> connected_{false};
};

} // namespace robert
