#pragma once

#include "commands/commands.hpp"
#include "protocol/protocol.pb.h"
#include "server/session.hpp"
#include "server/tasker.hpp"
#include "robot/robot.hpp"
#include <vector>
#include <memory>

namespace robert::server {

class RequestHandler {
public:
    RequestHandler(SessionManager& session_manager, Tasker& tasker, const std::vector<std::unique_ptr<robot::Robot>>& robots);

    [[nodiscard]] protocol::ServerResponse handle(const commands::DecodedRequest& decoded_request, std::atomic<bool>& running_flag);

private:
    SessionManager& session_manager_;
    Tasker& tasker_;
    const std::vector<std::unique_ptr<robot::Robot>>& robots_;

    [[nodiscard]] bool validate_token(const commands::DecodedRequest& request) const;
    [[nodiscard]] bool validate_hardware_lock(const commands::DecodedRequest& request) const;

    void handle_login(const commands::DecodedRequest& req, protocol::ServerResponse& res);
    void handle_logout(const commands::DecodedRequest& req, protocol::ServerResponse& res);
    void handle_acquire(const commands::DecodedRequest& req, protocol::ServerResponse& res);
    void handle_release(const commands::DecodedRequest& req, protocol::ServerResponse& res);
    void handle_check_task(const commands::DecodedRequest& req, protocol::ServerResponse& res);
    void handle_robot_command(const commands::DecodedRequest& req, protocol::ServerResponse& res);
};

} // namespace robert::server
