# SHCR Limitations

While SHCR provides robust resilience against specific classes of errors, it faces inherent limitations imposed by the hardware, OS, and the "Black Box" nature of runtime patching.

## 1. Repeated Identical Faults
SHCR patches the **instruction**, not the **code logic**.
- If a bug is inside a loop (e.g., `for (i=0; i<100; i++) array[i] = 0` where `array` is NULL), SHCR will patch the instruction once (to NOP).
- The loop will continue, effectively skipping the assignment for ALL iterations.
- **Result**: The program survives, but the array is never populated. Operations expecting data in that array may fail later.
- **Limitation**: We cannot infer the *programmer's intent*, only neuter the *crash*.

## 2. Logic Bugs vs Memory Safety
SHCR fixes **symptoms** (Segfaults), not **root causes** (Bad Logic).
- **Example**: `if (user.isAdmin) { ... }` where `user` is NULL.
- SHCR patches the dereference of `user.isAdmin`. The code might default to `false` (safe) or garbage (unsafe).
- We cannot guarantee the application state is valid, only that it is *running*.

## 3. Performance Overhead
- **Interception**: Context switching from User -> Kernel -> SHCR Supervisor is expensive (Microseconds).
- **Analysis**: Python scripts analysis adds latency (Milliseconds).
- **Patching**: `mach_vm_protect` and `mach_vm_write` involve syscalls and TLB flushes.
- **Impact**: The *first* crash incurs a significant penalty (~50-100ms). Subsequent executions of the patched code run at native speed (NOP is fast).
- **Risk**: "Zombie" loops (crash-patch-retry cycles) can cause 100% CPU usage before Escalation kicks in.

## 4. Multi-Threading
- Mach exceptions suspend the entire Task (Process).
- When SHCR patches memory, it affects ALL threads executing that code.
- If Thread A crashes and is patched, Thread B will also execute the patched (NOP'd) code immediately. This is generally desired but can lead to race conditions if the patch wasn't atomic (which it isn't fully, though single instruction replacement is close).
