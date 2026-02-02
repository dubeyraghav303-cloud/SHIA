#include <iostream>
#include <unistd.h>
#include <mach/mach.h>
#include <mach/mach_error.h>
#include <mach/thread_status.h>
#include <mach/task.h>
#include <vector>
#include <sys/wait.h>
#include <cstdlib>
#include <libgen.h>   // dirname
#include <limits.h>   // PATH_MAX
#include <mach-o/dyld.h> // _NSGetExecutablePath
#include <signal.h>
#include <sstream>
#include "context_analyzer.h"
#include "patch_apply.h"
#include "fault_history.h"

// Declaration of Rust Guard
extern "C" int shcr_validate_patch(const char* json_ptr);

// Get absolute path to the directory containing the executable
std::string get_executable_dir() {
    char path[PATH_MAX];
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        // Successfully got path
        // Note: dirname modifies the string inplace or returns static buffer. 
        // Safer to copy to std::string first then use c_str if needed, but dirname works on char*
        char* dir = dirname(path);
        return std::string(dir);
    }
    return "";
}

// Helper to run shell command and get output
std::string run_python_cmd(const std::string& script_path, const std::string& arg1, const std::string& arg2 = "") {
    // Check if script exists
    if (access(script_path.c_str(), F_OK) == -1) {
        std::cerr << "[SHCR-Error] Python script not found: " << script_path << std::endl;
        return "";
    }

    std::string cmd = "python3 " + script_path + " '" + arg1 + "' '" + arg2 + "'";
    // Redirect stderr to stdout to capture errors
    cmd += " 2>&1";
    
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "[SHCR-Error] popen failed for cmd: " << cmd << std::endl;
        return "";
    }
    
    char buffer[1024];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        result += buffer;
    }
    
    int return_code = pclose(pipe);
    if (return_code != 0) {
         std::cerr << "[SHCR-Error] Python exited with code " << return_code << ". Output:\n" << result << std::endl;
    }
    return result;
}

// Minimal Mach Exception Message Structure
struct exception_msg {
    mach_msg_header_t Head;
    mach_msg_body_t msgh_body;
    mach_msg_port_descriptor_t thread;
    mach_msg_port_descriptor_t task;
    NDR_record_t NDR;
    exception_type_t exception;
    mach_msg_type_number_t codeCnt;
    int64_t code[2];
    char padding[1024]; // Extra padding to be safe against larger messages / trailers
};

struct exception_reply {
    mach_msg_header_t Head;
    NDR_record_t NDR;
    kern_return_t RetCode;
};

// Architecture detection
#if defined(__arm64__)
    #define MACHINE_THREAD_STATE ARM_THREAD_STATE64
    #define MACHINE_THREAD_STATE_COUNT ARM_THREAD_STATE64_COUNT
    typedef arm_thread_state64_t machine_thread_state_t;
    #define REG_IP __pc
#elif defined(__x86_64__)
    #define MACHINE_THREAD_STATE x86_THREAD_STATE64
    #define MACHINE_THREAD_STATE_COUNT x86_THREAD_STATE64_COUNT
    typedef x86_thread_state64_t machine_thread_state_t;
    #define REG_IP __rip
#else
    #error "Unsupported Architecture"
#endif

#include "fault_history.h"

// ... (includes remain same, merged below)

// Main Supervisor Logic
int main(int argc, char* argv[]) {
    // Determine Paths
    std::string exe_dir = get_executable_dir();

    // Production-first core resolution
    std::string core_prod = exe_dir + "/../Resources/core";
    std::string core_dev  = exe_dir + "/../../../../core";

    std::string core_path;
    if (access(core_prod.c_str(), F_OK) == 0) {
        core_path = core_prod;
    } else if (access(core_dev.c_str(), F_OK) == 0) {
        core_path = core_dev;
    } else {
        std::cerr << "[SHCR-Fatal] core directory not found (neither Resources nor dev path)." << std::endl;
        return 1;
    }

    
    // Debug Paths
    std::cout << "[SHCR-Debug] Executable Dir: " << exe_dir << std::endl;
    std::cout << "[SHCR-Debug] Core Path: " << core_path << std::endl;

    if (argc < 2) {
        std::cerr << "Usage: shcr_runtime <target_executable>" << std::endl;
        return 1;
    }

    const char* target_path = argv[1];
    pid_t pid = fork();
    
    FaultHistory history;

    if (pid == 0) {
        // Child
        raise(SIGSTOP); // Suspend self so parent can attach
        execv(target_path, &argv[1]);
        perror("execv failed");
        exit(1);
    } else {
        // Parent
        std::cout << "[SHCR] Hosted " << target_path << " (PID: " << pid << ")" << std::endl;
        
        // Wait for child to stop
        int status;
        waitpid(pid, &status, WUNTRACED);
        
        if (!WIFSTOPPED(status)) {
             std::cerr << "[SHCR] Child process failed to start or didn't stop." << std::endl;
        }

        mach_port_t task;
        kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
        if (kr != KERN_SUCCESS) {
            std::cerr << "[SHCR] Failed to get task port (kr=" << kr << "). sudo might be required." << std::endl;
        }

        // Allocate Exception Port
        mach_port_t exception_port;
        mach_port_allocate(mach_task_self(), MACH_PORT_RIGHT_RECEIVE, &exception_port);
        mach_port_insert_right(mach_task_self(), exception_port, exception_port, MACH_MSG_TYPE_MAKE_SEND);

        // Set Exception Port on Child Task
        kr = task_set_exception_ports(
            task,
            EXC_MASK_BAD_ACCESS | EXC_MASK_ARITHMETIC | EXC_MASK_BAD_INSTRUCTION,
            exception_port,
            EXCEPTION_DEFAULT | MACH_EXCEPTION_CODES,
            MACHINE_THREAD_STATE 
        );
        if (kr != KERN_SUCCESS) {
             std::cerr << "[SHCR] Failed to set exception ports: " << kr << std::endl;
        }
        
        // Resume Child
        kill(pid, SIGCONT);

        std::cout << "[SHCR] Monitoring via Mach Exceptions..." << std::endl;

        // Message Loop
        while (true) {
            exception_msg msg;
            msg.Head.msgh_local_port = exception_port;
            msg.Head.msgh_size = sizeof(msg);
            
            kr = mach_msg(&msg.Head, MACH_RCV_MSG, 0, sizeof(msg), exception_port, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
            if (kr != KERN_SUCCESS) {
                if (kr == MACH_RCV_INTERRUPTED) break; 
                std::cerr << "[SHCR] mach_msg failed/broke loop with code: " << kr << " (" << mach_error_string(kr) << ")" << std::endl;
                break;
            }

            std::cout << "[SHCR] Exception Intercepted!" << std::endl;

            // 1. Analyze
            // Need to get thread state from the thread port in msg
            mach_port_t crash_thread = msg.thread.name;
            machine_thread_state_t state;
            mach_msg_type_number_t count = MACHINE_THREAD_STATE_COUNT;
            
            kr = thread_get_state(crash_thread, MACHINE_THREAD_STATE, (thread_state_t)&state, &count);
            if (kr != KERN_SUCCESS) {
                std::cerr << "[SHCR] Failed to get thread state" << std::endl;
            }

            // Check History & Progress
            // We need thread ID. mach_port_t is similar but actual ID is better? 
            // Using port name as ID is fine for unique mapping in this context.
            uint64_t tid = (uint64_t)crash_thread; 
            
            if (history.CheckEscalation(tid, state.REG_IP, state)) {
                std::cerr << "[SHCR] Escalation triggered. Killing process." << std::endl;
                kill(pid, SIGKILL);
                break;
            }

            int signal_code = 0;
            if (msg.exception == EXC_BAD_ACCESS) signal_code = 11;
            else if (msg.exception == EXC_ARITHMETIC) signal_code = 8;
            else if (msg.exception == EXC_BAD_INSTRUCTION) signal_code = 4;

            auto ctx = ContextAnalyzer::Analyze(pid, signal_code, msg.code[1], state.REG_IP);
            std::string ctx_json = ContextAnalyzer::SerializeContext(ctx);

            std::cout << "[SHCR] Context: " << ctx_json << std::endl;

            // 2. Classify (Python)
            std::string ctx_file = "/tmp/shcr_ctx.json";
            FILE* f = fopen(ctx_file.c_str(), "w");
            if (f) {
                fwrite(ctx_json.c_str(), 1, ctx_json.length(), f);
                fclose(f);
            }
            
            std::string classifier_script = core_path + "/patch_engine/classifier.py"; 
            std::string class_json = run_python_cmd(classifier_script, ctx_file);
            
            // Check if classification failed
            if (class_json.empty()) {
                std::cerr << "[SHCR] Classification failed (empty output). Killing child." << std::endl;
                kill(pid, SIGKILL);
                break;
            }
            std::cout << "[SHCR] Classification: " << class_json << std::endl;
            
            // Check for abort action
            if (class_json.find("\"action\": \"abort\"") != std::string::npos) {
                 std::cerr << "[SHCR] Classifier returned ABORT. Terminating child." << std::endl;
                 kill(pid, SIGKILL);
                 break;
            }

            // 3. Synthesize (Python)
            std::string synth_script = core_path + "/patch_engine/synthesizer.py";
            
            std::string class_file = "/tmp/shcr_class.json";
            f = fopen(class_file.c_str(), "w");
            if (f) {
                fwrite(class_json.c_str(), 1, class_json.length(), f);
                fclose(f);
            }
            
            std::string patch_json_str = run_python_cmd(synth_script, class_file, ctx_file);
             std::cout << "[SHCR] Patch Candidate: " << patch_json_str << std::endl;

            // 4. Validate (Rust)
            int safety = shcr_validate_patch(patch_json_str.c_str());
            if (safety == 0) {
                 std::cerr << "[SHCR] PATCH REJECTED BY RUST GUARD (or invalid JSON). Terminating." << std::endl;
                 kill(pid, SIGKILL);
                 break;
            }
            std::cout << "[SHCR] Rust Guard: APPROVED." << std::endl;
            
            // 5. Apply
            std::vector<uint8_t> patch_bytes;
            size_t bytes_pos = patch_json_str.find("\"machine_bytes\": [");
            if (bytes_pos != std::string::npos) {
                std::string array_content = patch_json_str.substr(bytes_pos + 18);
                size_t end_pos = array_content.find("]");
                array_content = array_content.substr(0, end_pos);
                std::stringstream ss_bytes(array_content);
                std::string segment;
                while(std::getline(ss_bytes, segment, ',')) {
                    patch_bytes.push_back(std::stoi(segment));
                }
            }
            
            if (!PatchTransformer::ApplyPatch(pid, state.REG_IP, patch_bytes)) {
                std::cerr << "[SHCR] Failed to apply patch. Terminating child to prevent infinite loop." << std::endl;
                kill(pid, SIGKILL);
                break;
            }

            // 6. Resume
            exception_reply reply;
            reply.Head.msgh_bits = MACH_MSGH_BITS(MACH_MSG_TYPE_MOVE_SEND_ONCE, 0);
            reply.Head.msgh_remote_port = msg.Head.msgh_remote_port; 
            reply.Head.msgh_local_port = MACH_PORT_NULL;
            reply.Head.msgh_id = msg.Head.msgh_id + 100;
            reply.NDR = msg.NDR;
            reply.RetCode = KERN_SUCCESS; // Handled!

            mach_msg(&reply.Head, MACH_SEND_MSG, sizeof(reply), 0, MACH_PORT_NULL, MACH_MSG_TIMEOUT_NONE, MACH_PORT_NULL);
        }
    }
    return 0;
}
