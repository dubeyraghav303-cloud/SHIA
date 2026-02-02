# SHCR Threat Model

This document defines the scope of protection provided by SHCR, what it explicitly does NOT cover, and the security rationale behind its design.

## 1. What SHCR Protects Against (In Scope)

SHCR is designed to mitigate **Accidental Memory Safety Violations** that are transient or local in nature.

| Threat / Fault | Description | Mitigation Strategy |
| :--- | :--- | :--- |
| **Null Pointer Dereference** | Read/Write to address 0x0. | **Skip**: Replace instruction with NOP. (Assumes logic can handle uninitialized data). |
| **Out-Of-Bounds Read** | Reading slightly past buffer end. | **Clamp/Skip**: Return 0 or skip read. |
| **Integer Divide-by-Zero** | `DIV` operation with zero denominator. | **Skip/Replace**: Replace result with MAX_INT or 0. |

**Goal**: Keep the service Available (CIA Triad: **Availability**) during non-critical faults.

## 2. What SHCR Refuses to Fix (Out of Scope)

SHCR actively refuses to heal faults that imply compromised Control Flow Integrity (CFI) or fundamental logic errors.

| Threat / Fault | Description | Action | Rationale |
| :--- | :--- | :--- | :--- |
| **Control Flow Violation** | Jumping to 0x0, unmapped memory, or gadget chains. | **ABORT** | Indicates stack corruption or ROP attack. Healing would mask an exploit. |
| **Semantic Infinite Loops** | Logic error causing endless recurrence (e.g. `while(1) { *p=0; }`). | **ABORT** | A "Healed" infinite loop creates a "Zombie" process that consumes CPU without doing work. |
| **Stack Smash** | Stack pointer exhaution or guard page hit. | **ABORT** | Stack frame is corrupted; safe return is impossible. |
| **Privileged Instruction Fault** | User-mode code executing `MSR`, `MRS` etc. | **ABORT** | Security violation. |

## 3. Escalation & Safety

### Why Escalation Exists?
Blindly healing every crash creates **systems that cannot die but cannot work**. 
- **The "Zombie" Problem**: A process that crashes, gets patched, retries, and crashes again forever.
- **Mitigation**: 
    - **Fault History**: Limit 3 retries per address.
    - **Non-Progress Detection**: If CPU state (Registers) does not change between patches, the patch is ineffective. The process is terminated.

### Rust Guard
To prevent the Runtime itself from being tricked into generating unsafe patches (e.g. overwriting the wrong function), the **Rust Guard** acts as a verified logic gate. It parses the *intent* of the patch and approves it only if it meets strict safety policies (e.g. "Never NOP a Branch").
