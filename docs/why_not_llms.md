# Why SHCR Rejects LLMs (The "Why Not AI?" Defense)

In an era of generative AI, SHCR deliberately relies on deterministic, rule-based systems (C++, Rust, Python heuristics). Here is why we strictly enforce **NO LLMs** in the hot path.

## 1. Determinism is Non-Negotiable
- **System Calls**: `mach_vm_write` must write the *exact* bytes requested.
- **LLM Behavior**: Large Language Models are probabilistic. Asking an LLM to "Generate a patch for a null deref" might yield valid assembly 99% of the time, and a hallucinated opcode 1% of the time.
- **Consequence**: A 1% failure rate in a runtime supervisor is catastrophic. It introduces non-deterministic bugs into previously deterministic crashes.

## 2. Safety Verification
- **Rule-Based**: We can mathematically prove that our `synthesizer.py` outputs `0xD503201F` (NOP) for `skip_instruction`.
- **LLM-Based**: Verifying the output of an LLM requires a secondary parser/assembler that is just as complex as the system we built. If we need a deterministic verifier, we might as well use a deterministic generator.

## 3. Reproducibility
- **Debuggability**: When SHCR patches a process, the log must be identical for identical inputs.
- **AI Variance**: Low temperature reduces but does not eliminate variance. Engineers debugging a production incident need to know *exactly* why the runtime accepted a patch. "The AI thought it was safe" is not an acceptable root cause analysis.

## 4. Kernel-Adjacent Constraints
- **Latency**: An LLM inference call (local or API) takes 100ms+. A rule-based lookup takes microseconds.
- **Dependencies**: SHCR runs as a lightweight supervisor. Embedding a 2GB+ Torch/GGUF model or requiring network access (API) violates the "Low Overhead" and "Sandbox" constraints of OS-level tools.
- **Security**: Sending process memory (crash dump) to an external API is a privacy violation.

## Conclusion
SHCR treats code repair as a **Compiler/OS problem**, not a **Natural Language problem**. Machine code has strict semantics; it does not require "reasoning" or "creativity"—it requires **precision**.
