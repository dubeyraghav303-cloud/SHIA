cmake_minimum_required(VERSION 3.15)
project(SHCR VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/app/SHCR.app/Contents/MacOS)

# Rust Integration
set(RUST_LIB_DIR ${CMAKE_SOURCE_DIR}/core/runtime_guard/target/release)
set(RUST_LIB_NAME libshcr_guard.a)

# Add custom command to build Rust
add_custom_target(build_rust
    COMMAND cargo build --release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/core/runtime_guard
    COMMENT "Building Rust Runtime Guard"
)

# Core Files
set(CORE_SOURCES
    core/interceptor/signal_handler.cpp
    core/interceptor/mach_exceptions.cpp
    core/analyzer/context_analyzer.cpp
    core/transformer/patch_apply.cpp
)

# Include Directories
include_directories(
    core/interceptor
    core/analyzer
    core/transformer
    core/runtime_guard/src # For headers if we had a generated one
)

# Main Executable
add_executable(shcr_runtime
    ${CORE_SOURCES}
    # Main entry point will be arguably in one of these or separate. 
    # Let's assume we need a main.cpp or one of the files has main.
    # The prompt implies 'shcr_runtime' is the artifact. I'll add a main.cpp later or 
    # assume the logic is in a new file, but the structure doesn't list main.cpp.
    # I will create a core/main.cpp to drive it, but wait, the prompt "REQUIRED FOLDER STRUCTURE"
    # doesn't show a main.cpp. It implies `interceptor/mach_exceptions.cpp` or similar might host main
    # or I should put it in `core/interceptor`.
    # Actually, strict structure means strict structure. 
    # I will put the main function in `core/interceptor/mach_exceptions.cpp` as that is the likely supervisor.
)

target_link_libraries(shcr_runtime
    PRIVATE
    ${RUST_LIB_DIR}/${RUST_LIB_NAME}
    # System libraries for Mach APIs
)
add_dependencies(shcr_runtime build_rust)

# Demos
add_subdirectory(demos/null_deref)
add_subdirectory(demos/divide_by_zero)
add_subdirectory(demos/out_of_bounds)
add_subdirectory(demos/use_after_free)
add_subdirectory(demos/infinite_loop)

# Phase 2 Demos
add_subdirectory(demos/loop_null)
add_subdirectory(demos/readonly_write)
add_subdirectory(demos/bad_jump)
add_subdirectory(demos/stack_smash)
add_subdirectory(demos/threads)
