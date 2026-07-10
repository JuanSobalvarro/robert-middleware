ISO/IEC 14882:2020 (C++20), strictly — CMAKE_CXX_STANDARD 20 + CMAKE_CXX_STANDARD_REQUIRED ON + CMAKE_CXX_EXTENSIONS OFF disables GNU/MSVC compiler extensions, so you get portable standard C++, not a GCC dialect.

Modern/target-based CMake (the convention Kitware itself documents as current best practice, and the one taught in Craig Scott's Professional CMake and Daniel Pfeifer's "Effective CMake" — the two most commonly cited references for this): you use target_include_directories/target_link_libraries scoped to ${PROJECT_NAME} rather than the legacy global include_directories()/link_libraries(). No global state leakage 

FetchContent-based dependency management, per CMake's own FetchContent module documentation (the officially recommended approach, cmake.org).
