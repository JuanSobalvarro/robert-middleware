#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
#include <optional>
#include <mutex>


namespace robert::server {

struct Session {
    std::string username;
    std::chrono::steady_clock::time_point last_seen; // last request from the client
};


class SessionManager {
public:
    SessionManager() = default;

    bool load_users_from_file(const std::string& filepath);

    std::optional<std::string> login(const std::string& username, const std::string& password);
    void logout(const std::string& token);
    bool is_valid_token(const std::string& token);
    void touch_session(const std::string& token); // update last_seen
    void sweep_expired_sessions(std::chrono::seconds timeout);

    bool acquire_lock(std::string& token);
    bool release_lock(const std::string& token);
    bool has_lock(const std::string& token) const;
    bool is_locked() const;

    size_t num_users() const;

private:
    std::unordered_map<std::string, std::string> users_;
    std::unordered_map<std::string, Session> sessions_;
    std::optional<std::string> current_lock_;

    mutable std::mutex mutex_; // thread-safe

    std::string generate_uuid();
};

} // namespace robert::server
