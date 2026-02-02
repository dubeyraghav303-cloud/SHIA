import sys
import json
import subprocess
import os
import tempfile

def synthesize_patch(classification, context):
    patch_type = classification.get("type")
    rip = context.get("rip")
    
    asm_code = []
    
    # Simple Synthesis Strategies for macOS x86_64 / arm64
    # Assuming x86_64 for this specific implementation detail to be concrete,
    # or we can detect arch. The prompt mentions "Apple Silicon + Intel".
    # We will try to emit arch-neutral generic logic or check `os.uname().machine`
    
    arch = os.uname().machine
    is_arm = "arm" in arch
    
    # Strategy: Use internal opcode lookup for stability & speed
    # This avoids toolchain alignment/parsing issues (e.g. otool printing addresses)
    
    machine_bytes = []
    
    if patch_type == "null_guard" or patch_type == "infinite_loop_break" or patch_type == "skip_instruction":
        # Strategy: Replace with NOP to effectively skip
        if is_arm:
            # ARM64 NOP: D5 03 20 1F (Little Endian: 1F 20 03 D5)
            machine_bytes = [0x1F, 0x20, 0x03, 0xD5]
            asm_code = ["nop"]
        else:
            # x86_64 NOP: 90
            # Ideally we match instruction length (e.g. 2-7 bytes). 
            # For demo simplicity, we patch 1 byte NOP, assuming we can resume? 
            # DANGEROUS on x86 without length disassembly.
            # But assume 1 byte patch is safer than RET.
            machine_bytes = [0x90, 0x90, 0x90] # 3 NOPs to be safe? No, might overwrite next.
            # Safe bet: 1 NOP, let the rest be interpreted? 
            # Actually, standard way is converting faulting instr to NOPs. 
            machine_bytes = [0x90]
            asm_code = ["nop"]

    elif patch_type == "div_zero_guard":
        # Placeholder for more complex logic
        if is_arm:
            machine_bytes = [0xC0, 0x03, 0x5F, 0xD6] # Just RET for demo
            asm_code = ["ret"]
        else:
            machine_bytes = [0xC3] 
            asm_code = ["ret"]
    
    return {
        "patch_type": patch_type,
        "target_address": rip, 
        "asm_instructions": asm_code,
        "machine_bytes": machine_bytes
    }

def assemble(asm_str, is_arm):
    return [] # Deprecated

if __name__ == "__main__":
    if len(sys.argv) > 2:
        # The inputs are file paths
        with open(sys.argv[1], 'r') as f:
            classification = json.load(f)
        with open(sys.argv[2], 'r') as f:
            context = json.load(f)
            
        patch = synthesize_patch(classification, context)
        print(json.dumps(patch))
