#include "server/request_handler.hpp"
#include "commands/commands.hpp"
#include "commands/decoder.hpp"
#include "server/tasker.hpp"
#include <iostream>

namespace robert::server {

RequestHandler::RequestHandler(SessionManager& session_manager, Tasker& tasker, const std::vector<std::unique_ptr<robot::Robot>>& robots)
: session_manager_(session_manager), tasker_(tasker), robots_(robots) {}

protocol::ServerResponse RequestHandler::handle(const commands::DecodedRequest& decoded_request, std::atomic<bool>& running_flag) {
    protocol::ServerResponse response;
    std::cout << "[HANDLER] Processing Request: " << commands::type_to_string(decoded_request.cmd_type) << std::endl;

    switch (decoded_request.cmd_type) {
        case commands::RapidCommandType::UNKNOWN:
            response.set_status(protocol::ResponseStatus::ERROR);
            response.set_error_message("Unknown command");
            break;

        case commands::RapidCommandType::LOGIN:
            handle_login(decoded_request, response);
            break;

        case commands::RapidCommandType::LOGOUT:
            handle_logout(decoded_request, response);
            break;

        case commands::RapidCommandType::ACQUIRE:
            handle_acquire(decoded_request, response);
            break;

        case commands::RapidCommandType::RELEASE:
            handle_release(decoded_request, response);
            break;
        case commands::RapidCommandType::CHECK_TASK:
            handle_check_task(decoded_request, response);
            break;

        case commands::RapidCommandType::EXIT:
            if (validate_token(decoded_request) && validate_hardware_lock(decoded_request)) {
                response.set_status(protocol::ResponseStatus::SUCCESS);
                response.set_text_payload("OKISUWU");
                std::cout << "[HANDLER] Exit called. I am deleting this shit..." << std::endl;
                // running_flag = false;
            } else {
                response.set_status(protocol::ResponseStatus::ERROR);
                response.set_error_message("Invalid token");
            }
            break;

        case commands::RapidCommandType::PING:
            response.set_status(protocol::ResponseStatus::SUCCESS);
            response.set_text_payload("PONGUWU");
            break;

        case commands::RapidCommandType::PINGR:
            if (validate_token(decoded_request)) {
                response.set_status(protocol::ResponseStatus::SUCCESS);
                response.set_error_message("PONGR_NOT_IMPLEMENTED");
            } else {
                response.set_status(protocol::ResponseStatus::ERROR);
                response.set_error_message("Invalid token");
            }
            break;

        default:
            handle_robot_command(decoded_request, response);
            break;
    }

    return response;
}

bool RequestHandler::validate_token(const commands::DecodedRequest& request) const {
    std::string token = request.session_token.value_or("");

    if (session_manager_.is_valid_token(token)) {
        session_manager_.touch_session(token);
        return true;
    }

    return session_manager_.is_valid_token(request.session_token.value_or(""));
}

bool RequestHandler::validate_hardware_lock(const commands::DecodedRequest& request) const {
    return session_manager_.has_lock(request.session_token.value_or(""));
}

// specific handlers
void RequestHandler::handle_login(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    std::string token = session_manager_.login(req.username.value_or(""), req.password.value_or("")).value_or("");
    if (token.empty()) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid credentials");
    } else {
        res.set_status(protocol::ResponseStatus::SUCCESS);
        res.set_text_payload(token);
    }
}

void RequestHandler::handle_logout(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    std::string token = req.session_token.value_or("");
    if (token.empty()) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("No token provided");
        return;
    }
    if (!validate_token(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid token");
        return;
    }
    session_manager_.logout(token);
    res.set_status(protocol::ResponseStatus::SUCCESS);
    res.set_text_payload("Logged out successfully");
}

void RequestHandler::handle_acquire(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    if (!validate_token(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid token");
        return;
    }
    if (validate_hardware_lock(req)) {
        res.set_status(protocol::ResponseStatus::WARNING);
        res.set_error_message("You already have a hardware lock");
        return;
    }
    std::string token = req.session_token.value_or("");
    if (token.empty()) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("No token provided");
        return;
    }

    if (!session_manager_.acquire_lock(token)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Failed to acquire lock. Someone else may have it locked.");
        return;
    }
    res.set_status(protocol::ResponseStatus::SUCCESS);
    res.set_text_payload("Lock acquired successfully");
}

void RequestHandler::handle_release(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    if (!validate_token(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid token");
        return;
    }
    if (!validate_hardware_lock(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Hardware lock not acquired");
        return;
    }
    session_manager_.release_lock(req.session_token.value_or(""));
    res.set_status(protocol::ResponseStatus::SUCCESS);
    res.set_text_payload("Lock released successfully");
}

void RequestHandler::handle_check_task(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    if (!validate_token(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid token");
        return;
    }
    if (!validate_hardware_lock(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Hardware lock not acquired");
        return;
    }

    task_id_t id = req.task_id.value();
    auto task = tasker_.getTask(id);

    if (!task) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Task ID not found in registry");
        return;
    }

    TaskState state = task->getState();
    const std::string& message = task->getResponse();

    if (state == TaskState::COMPLETED) {
        res.set_status(protocol::ResponseStatus::SUCCESS);
        res.set_task_status(protocol::TaskStatus::TASK_COMPLETED);

        if (task->getRequest().command_id == commands::RapidCommandType::GETSTATUS) {
            const std::vector<uint8_t> raw_data(message.begin(), message.end());
            bool success = commands::Decoder::unpack_robot_status(raw_data, res.mutable_robot_status());

            if (!success) {
                res.set_status(protocol::ResponseStatus::ERROR);
                res.set_error_message("Failed to unpack telemetry payload");
            }
        } else {
            res.set_text_payload(message);
        }
        tasker_.removeTask(id);
    }
    else if (state == TaskState::PENDING) {
        res.set_status(protocol::ResponseStatus::SUCCESS);
        res.set_task_status(protocol::TaskStatus::TASK_PENDING);
    }
    else if (state == TaskState::IN_PROGRESS) {
        res.set_status(protocol::ResponseStatus::SUCCESS);
        res.set_task_status(protocol::TaskStatus::TASK_IN_PROGRESS);
    }
    else {
        res.set_status(protocol::ResponseStatus::SUCCESS);
        res.set_task_status(protocol::TaskStatus::TASK_FAILED);
        res.set_error_message("Task execution failed at robot controller");
    }
}

void RequestHandler::handle_robot_command(const commands::DecodedRequest& req, protocol::ServerResponse& res) {
    if (!validate_token(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Invalid token");
        return;
    }
    if (!validate_hardware_lock(req)) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Hardware lock not acquired");
        return;
    }

    if (robots_.empty() || !robots_[0]->is_connected()) {
        res.set_status(protocol::ResponseStatus::ERROR);
        res.set_error_message("Robot not connected");
        return;
    }

    const commands::RapidRequest robot_req = commands::create_rapid_request(req);
    task_id_t new_task_id = tasker_.addTask(robot_req);
    res.set_status(protocol::ResponseStatus::SUCCESS);
    res.set_task_status(protocol::TaskStatus::TASK_PENDING);
    res.set_task_id(new_task_id);
    res.set_text_payload("TaskAdded.uwunyanichan");
}

} // namespace robert::server
