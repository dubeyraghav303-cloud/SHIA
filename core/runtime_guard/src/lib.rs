use std::ffi::CStr;
use std::os::raw::c_char;
use serde::{Deserialize, Serialize};

#[derive(Serialize, Deserialize, Debug)]
struct PatchRequest {
    patch_type: String,
    target_address: u64,
    asm_instructions: Vec<String>,
    machine_bytes: Vec<u8>,
}

#[no_mangle]
pub extern "C" fn shcr_validate_patch(json_ptr: *const c_char) -> i32 {
    if json_ptr.is_null() {
        return 0;
    }

    let c_str = unsafe { CStr::from_ptr(json_ptr) };
    let json_str = match c_str.to_str() {
        Ok(s) => s,
        Err(_) => return 0,
    };

    let patch: PatchRequest = match serde_json::from_str(json_str) {
        Ok(p) => p,
        Err(_) => return 0,
    };

    if validate_safety(&patch) {
        1
    } else {
        0
    }
}

fn validate_safety(patch: &PatchRequest) -> bool {
    // Rule 1: No Syscalls
    for instr in &patch.asm_instructions {
        let lower = instr.to_lowercase();
        if lower.contains("syscall") || lower.contains("int 0x80") || lower.contains("svc") {
            return false;
        }
        // Rule 2: No Allocations (basic heuristic scanning for symbol names if present, or dangerous patterns)
        if lower.contains("malloc") || lower.contains("free") || lower.contains("realloc") {
             return false;
        }
    }

    // Rule 3: No loops (Simple heuristic: no backward jumps)
    // In a real disassembler we check relative offsets. 
    // Here we check for explicit 'jmp' with negative offsets or labels that might imply it.
    // For this constraint-compliant demo, we ensure patches are linear blocks.
    for instr in &patch.asm_instructions {
        if instr.to_lowercase().starts_with("jmp -") { // pseudo-asm check
            return false;
        }
    }

    // Rule 4: Patch Type Verification
    match patch.patch_type.as_str() {
        "null_guard" => verify_null_guard(patch),
        "div_zero_guard" => true,
        "bounds_check" => true, 
        "skip_instruction" => true,
        "fallback_return" => true,
        "infinite_loop_break" => true,
        _ => false, // Unknown patch type
    }
}

fn verify_null_guard(patch: &PatchRequest) -> bool {
    // Specifically ensure it contains a check against 0 OR it is a safe return OR a nop
    let has_check = patch.asm_instructions.iter().any(|i| i.contains("cmp") && (i.contains("0") || i.contains("zero")));
    let has_ret = patch.asm_instructions.iter().any(|i| i.contains("ret"));
    let has_nop = patch.asm_instructions.iter().any(|i| i.contains("nop"));
    has_check || has_ret || has_nop
}
