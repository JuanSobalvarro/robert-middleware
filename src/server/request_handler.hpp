#pragma once

#include "commands/commands.hpp"
#include "protocol/protocol.pb.h"
#include "server/session.hpp"
#include "server/tasker.hpp"
#include "robot/robot.hpp"
#include <memory>

namespace robert::server {

class RequestHandler {
public:
    RequestHandler(SessionManager& session_manager, Tasker& tasker, const std::unique_ptr<robot::Robot>& robot);

    [[nodiscard]] protocol::ServerResponse handle(const commands::DecodedRequest& decoded_request, std::atomic<bool>& running_flag);

private:
    SessionManager& session_manager_;
    Tasker& tasker_;
    const std::unique_ptr<robot::Robot>& robot_;

    /*
     * @brief Validate the token in the request.
     *
     * So, this is the "security" part of the request handler.
     *
     * @param request The decoded request.
     * @return True if the token is valid, false otherwise.
     */
    [[nodiscard]] bool validate_token(const commands::DecodedRequest& request) const;

    /*
     * @brief Validate the hardware lock in the request.
     *
     * This is important, because it ensures that only one user at a time can control the robot.
     *
     * @param request The decoded request.
     * @return True if the hardware lock is valid, false otherwise.
     */
    [[nodiscard]] bool validate_hardware_lock(const commands::DecodedRequest& request) const;

    /*
     * @brief Handle a login request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_login(const commands::DecodedRequest& req, protocol::ServerResponse& res);

    /*
     * @brief Handle a logout request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_logout(const commands::DecodedRequest& req, protocol::ServerResponse& res);

    /*
     * @brief Handle an acquire request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_acquire(const commands::DecodedRequest& req, protocol::ServerResponse& res);

    /*
     * @brief Handle a release request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_release(const commands::DecodedRequest& req, protocol::ServerResponse& res);

    /*
     * @brief Handle a check task request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_check_task(const commands::DecodedRequest& req, protocol::ServerResponse& res);

    /*
     * @brief Handle a robot command request.
     * @param req The decoded request.
     * @param res The response to send back.
     */
    void handle_robot_command(const commands::DecodedRequest& req, protocol::ServerResponse& res);
};

} // namespace robert::server
