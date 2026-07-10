#include "tasker.hpp"
#include <shared_mutex>

namespace robert::server {

// task

Task::Task(task_id_t id, const commands::RapidRequest& request)
    : m_id(id), m_request(request), m_state(TaskState::PENDING)
{
    m_created_time = std::chrono::steady_clock::now();
}

bool Task::markStarted() {
    if (m_state != TaskState::PENDING) {
        return false;
    }

    m_state = TaskState::IN_PROGRESS;
    m_start_time = std::chrono::steady_clock::now();

    return true;
}

bool Task::markCompleted(const std::string& response) {
    if (m_state != TaskState::IN_PROGRESS) {
        return false;
    }

    m_state = TaskState::COMPLETED;
    m_end_time = std::chrono::steady_clock::now();
    m_response = response;

    return true;
}

bool Task::markFailed() {
    if (m_state != TaskState::IN_PROGRESS) {
        return false;
    }

    m_state = TaskState::FAILED;
    m_end_time = std::chrono::steady_clock::now();

    return true;
}

task_id_t Task::getTaskId() const {
    return m_id;
}

TaskState Task::getState() const {
    return m_state;
}

const commands::RapidRequest& Task::getRequest() const {
    return m_request;
}

std::chrono::milliseconds Task::getLatency() const {
    if (m_state == TaskState::PENDING || m_state == TaskState::IN_PROGRESS) {
        return std::chrono::milliseconds(0); // return 0 if task is not completed yet
    } else {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_created_time);
    }
}

std::chrono::milliseconds Task::getExecutionTime() const {
    if (m_state == TaskState::PENDING || m_state == TaskState::IN_PROGRESS) {
        return std::chrono::milliseconds(0); // return 0 if task is not completed yet
    } else {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_end_time - m_start_time);
    }
}


const std::string& Task::getResponse() const {
    return m_response;
}

// tasker

task_id_t Tasker::addTask(const commands::RapidRequest& request) {
    task_id_t id = m_next_task_counter.fetch_add(1, std::memory_order_relaxed);

    {
        std::unique_lock map_lock(m_map_mutex);
        m_task_map.emplace(id, Task(id, request));
    }

    {
        std::lock_guard queue_lock(m_queue_mutex);
        m_task_queue.push(id);
    }

    m_queue_cv.notify_one();
    return id;
}

std::optional<Task> Tasker::getTask(task_id_t task_id) {

    std::shared_lock lock(m_map_mutex); // shared lock since we are doing read only operations

    auto it = m_task_map.find(task_id); // only one lookup, aka optimizations no one asked for haha
    if (it == m_task_map.end()) {
        return std::nullopt;
    }

    return it->second;
}

bool Tasker::startTask(task_id_t task_id) {
    std::unique_lock map_lock(m_map_mutex);

    auto it = m_task_map.find(task_id);
    if (it == m_task_map.end()) {
        return false;
    }

    Task& task = it->second;
    task.markStarted();

    return true;
}

bool Tasker::completeTask(task_id_t task_id, const std::string& response) {
    std::unique_lock map_lock(m_map_mutex);

    auto it = m_task_map.find(task_id);
    if (it == m_task_map.end()) {
        return false;
    }

    Task& task = it->second;
    task.markCompleted(response);

    return true;
}

bool Tasker::failTask(task_id_t task_id) {
    std::unique_lock map_lock(m_map_mutex);

    auto it = m_task_map.find(task_id);
    if (it == m_task_map.end()) {
        return false;
    }

    Task& task = it->second;
    task.markFailed();

    return true;
}


bool Tasker::removeTask(task_id_t task_id) {
    std::unique_lock map_lock(m_map_mutex);

    auto it = m_task_map.find(task_id);
    if (it == m_task_map.end()) {
        return false;
    }

    m_task_map.erase(it);
    return true;
}

void Tasker::wakeUpAll() {
    m_queue_cv.notify_all();
}

std::optional<task_id_t> Tasker::waitForNextTask() {
    std::unique_lock queue_lock(m_queue_mutex);

    m_queue_cv.wait(queue_lock, [this] {
        return !m_task_queue.empty() || m_shutdown;
    });

    if (m_shutdown && m_task_queue.empty()) {
        return std::nullopt;
    }

    task_id_t task_id = m_task_queue.front();
    m_task_queue.pop();
    return task_id;
}

void Tasker::stop() {
    {
        std::lock_guard lock(m_queue_mutex);
        m_shutdown = true;
    }
    m_queue_cv.notify_all();
}

} // namespace robert::server
