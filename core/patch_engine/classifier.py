import json
import os
import sys

# Load rules
RULES_PATH = os.path.join(os.path.dirname(__file__), "rules", "core_rules.json")
try:
    with open(RULES_PATH, "r") as f:
        RULES = json.load(f)
except Exception as e:
    RULES = {}

def classify_crash(context):
    """
    Context is a dictionary:
    {
        "signal": "SIGSEGV",
        "si_code": 0, # or string
        "address": 0x...,
        "rip": 0x...,
        "disassembly": "mov rax, [rbx]"
    }
    """
    sig = context.get("signal")
    addr = context.get("address", 0)
    instr = context.get("instruction", 0)
    
    # 1. Safety Check: Is it a control flow instruction?
    # ARM64 Heuristics for Branch/Call/Ret
    # RET:  1101 0110 0101 1111 0000 0011 1100 0000 -> D65F03C0 
    # B:    0001 01... 
    # BL:   1001 01...
    # BR:   1101 0110 0001 1111 ...
    # Simplified check using bitmasks on uint32
    
    is_cf = False
    
    # Check for RET (Family) (D65F03C0 is standard ret)
    if (instr & 0xFFFFFC1F) == 0xD65F0000: is_cf = True
    
    # Check for B / BL (Unconditional Branch Immediate)
    # op(1) 0 0 1 0 1 ...
    # B: 0001 01... (0x14000000) mask 0xFC000000
    if (instr & 0x7C000000) == 0x14000000: is_cf = True
    
    # Check for BR / BLR (Unconditional Branch Register)
    # 1101 0110 0001 1111 ... (0xD61F0000)
    if (instr & 0xFE000000) == 0xD6000000: is_cf = True
    
    if is_cf:
        return {
            "type": "unrecoverable",
            "action": "abort",
            "description": "Fault on Control Flow Instruction. Unsafe to patch."
        }

    # 2. Crash Logic
    if sig == "SIGSEGV":
        if addr == 0:
            return RULES["SIGSEGV"]["0x0"]
        else:
            return RULES["SIGSEGV"]["default"]
            
    if sig == "SIGFPE":
        return RULES["SIGFPE"]["integer_divide_by_zero"]
        
    if sig == "TIMEOUT":
        return RULES["TIMEOUT"]["default"]

    if sig == "SIGILL":
        return RULES["SIGILL"]["default"]

    return None

if __name__ == "__main__":
    # CLI Mode for integration
    if len(sys.argv) > 1:
        ctx_file = sys.argv[1]
        with open(ctx_file, "r") as f:
            ctx = json.load(f)
        result = classify_crash(ctx)
        print(json.dumps(result))
