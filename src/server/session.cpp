#include "session.hpp"

#include <fstream>
#include <mutex>
#include <random>
#include <sstream>
#include <iostream>

namespace robert::server {

bool SessionManager::load_users_from_file(const std::string& filepath) {
    std::ifstream file(filepath);
    std::string line;

    if (!file.is_open()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    users_.clear();

    while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string username, password;

            if (std::getline(ss, username, '|') && std::getline(ss, password, '|')) {
                users_[username] = password;
                std::cout << "[SESSION] Registered user: " << username << std::endl;
            }
        }

    return true;
}

std::optional<std::string> SessionManager::login(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(mutex_);

    // check if there is already a session for this user
    for (const auto& [token, session] : sessions_) {
        if (session.username == username) {
            return token;
        }
    }

    // check if the username exists and the password matches
    if (users_.find(username) != users_.end() && users_[username] == password) {
        std::string token = generate_uuid();
        sessions_[token] = {username};
        return token;
    }
    return std::nullopt;
}

void SessionManager::logout(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(token);
}

bool SessionManager::is_valid_token(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);
    return sessions_.find(token) != sessions_.end();
}

void SessionManager::touch_session(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = sessions_.find(token);
    if (it != sessions_.end()) {
        it->second.last_seen = std::chrono::steady_clock::now();
    }
}

void SessionManager::sweep_expired_sessions(std::chrono::seconds timeout) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::steady_clock::now();
    for (auto it = sessions_.begin(); it != sessions_.end(); ) {
        if (now - it->second.last_seen > timeout) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

bool SessionManager::acquire_lock(std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (current_lock_) return false;
    current_lock_ = token;
    return true;
}

bool SessionManager::release_lock(const std::string& token) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (current_lock_ != token) return false;
    current_lock_.reset();
    return true;
}

bool SessionManager::has_lock(const std::string& token) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_lock_ == token;
}

bool SessionManager::is_locked() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_lock_.has_value();
}

size_t SessionManager::num_users() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return users_.size();
}


std::string SessionManager::generate_uuid() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    const char* v = "0123456789abcdef";
    std::string res;
    res.reserve(36);

    for (int i = 0; i < 8; i++) res += v[dis(gen)];
    res += "-";
    for (int i = 0; i < 4; i++) res += v[dis(gen)];
    res += "-4"; // UUID v4 format
    for (int i = 0; i < 3; i++) res += v[dis(gen)];
    res += "-";
    res += v[dis2(gen)];
    for (int i = 0; i < 3; i++) res += v[dis(gen)];
    res += "-";
    for (int i = 0; i < 12; i++) res += v[dis(gen)];

    return res;
}

} // namespace robert::server
