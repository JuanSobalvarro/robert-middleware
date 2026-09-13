#pragma once

#ifdef ROBERT_DEBUG

    #include <iostream>
    #include <mutex>

    inline std::mutex& get_debug_mutex() {
        static std::mutex mutex;
        return mutex;
    }

    #define DEBUG_LOG(msg) \
        do { \
            std::lock_guard<std::mutex> lock(get_debug_mutex()); \
            std::cout << "[DEBUG] " << msg << std::endl; \
        } while (0)

#else
    #define DEBUG_LOG(msg) do {} while (0)
#endif
