cmake_minimum_required(VERSION 3.15)
project(SHIA VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/SHIA.app/Contents/MacOS)

# Core Files
set(CORE_SOURCES
    core/shia_engine.cpp
    core/detector/log_tailer.cpp
    core/detector/worker_pool.cpp
    core/diagnoser/diagnostic_engine.cpp
    core/recoverer/recovery_controller.cpp
    core/circuit_breaker/circuit_breaker.cpp
    core/ai_bridge.cpp
    core/telemetry_bridge.cpp
    core/integrity_manager.cpp
    core/blockchain_bridge.cpp
)

# Include Directories
include_directories(
    core
    core/detector
    core/diagnoser
    core/recoverer
    core/circuit_breaker
)

# Main Executable
add_executable(shia_engine
    ${CORE_SOURCES}
)

# Note: Rust guard is ignored for SHIA as SHIA is external-based. We will keep it for reference or future expansion.
# System libraries if any can be linked here.
target_link_libraries(shia_engine PRIVATE pthread curl)
