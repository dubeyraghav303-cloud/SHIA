# Self-Healing Code Runtime (SHCR)

SHCR is a macOS-native runtime system that intercepts crashes in C/C++ programs and applies safe, deterministic patches live, without recompilation.

## Features
*   **Zero External Dependencies**: No AI/LLM APIs.
*   **Live Patching**: Modifies memory of running processes.
*   **Safety First**: Rust-based guard ensures patches don't introduce malware/instability.
*   **Explainable**: Rule-based Python engine.

## Structure
*   `app/`: The macOS Bundle structure.
*   `core/`: C++, Rust, and Python sources.
*   `demos/`: Vulnerable programs for testing.

## Build Instructions
1.  Ensure you have `cmake`, `cargo`, and macOS.
2.  Run:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ```
3.  Run the runtime:
    ```bash
    ./app/SHCR.app/Contents/MacOS/shcr_runtime ./demos/null_deref/demo_null_deref
    ```

## License
MIT
