#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <queue>
#include <shared_mutex>
#include <unordered_map>
#include <atomic>

#include "commands/commands.hpp"

namespace robert::server {

typedef uint64_t task_id_t;

enum class TaskState {
    PENDING,
    IN_PROGRESS,
    COMPLETED,
    FAILED
};

class Task {
public:
    explicit Task(task_id_t id, const commands::RapidRequest& request);

    bool markStarted();
    bool markCompleted(const std::string& response);
    bool markFailed();

    task_id_t getTaskId() const;
    TaskState getState() const;
    const commands::RapidRequest& getRequest() const;
    std::chrono::milliseconds getLatency() const;
    std::chrono::milliseconds getExecutionTime() const;
    const std::string& getResponse() const;


private:
    task_id_t m_id;
    TaskState m_state;
    commands::RapidRequest m_request;
    std::chrono::steady_clock::time_point m_created_time;
    std::chrono::steady_clock::time_point m_start_time;
    std::chrono::steady_clock::time_point m_end_time;
    std::string m_response;
};

class Tasker {

public:
    Tasker() = default;
    ~Tasker() = default;

    // server adds tasks that receives from the client
    task_id_t addTask(const commands::RapidRequest& request);
    std::optional<Task> getTask(task_id_t task_id);

    bool startTask(task_id_t task_id);
    bool completeTask(task_id_t task_id, const std::string& response);
    bool failTask(task_id_t task_id);

    bool removeTask(task_id_t task_id);

    void wakeUpAll();

    std::optional<task_id_t> waitForNextTask();

    void stop();

private:
    std::atomic<task_id_t> m_next_task_counter{1};

    // for client polling
    std::unordered_map<task_id_t, Task> m_task_map;
    mutable std::shared_mutex m_map_mutex; // we dont really care for tasks waiting since we execute one task at a time

    std::queue<task_id_t> m_task_queue;
    std::mutex m_queue_mutex;
    std::condition_variable m_queue_cv;

    std::atomic<bool> m_shutdown{false};

};

} // namespace robert::server
