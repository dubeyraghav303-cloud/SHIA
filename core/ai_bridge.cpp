#include "ai_bridge.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <curl/curl.h>

const std::string GEMINI_API_KEY = "AIzaSyDZ9tn2wM1n1graMCcLhDEdUMEhYDoKgWM";
const std::string GEMINI_API_URL = "https://generativelanguage.googleapis.com/v1beta/models/gemini-flash-latest:generateContent?key=" + GEMINI_API_KEY;

// libcurl write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string AIBridge::EscapeJsonString(const std::string& input) {
    std::string output;
    for (char c : input) {
        if (c == '"') output += "\\\"";
        else if (c == '\\') output += "\\\\";
        else if (c == '\b') output += "\\b";
        else if (c == '\f') output += "\\f";
        else if (c == '\n') output += "\\n";
        else if (c == '\r') output += "\\r";
        else if (c == '\t') output += "\\t";
        else output += c;
    }
    return output;
}

// Very basic JSON string extraction to avoid importing full json libs
std::string AIBridge::ExtractJsonString(const std::string& json_response, const std::string& key) {
    std::string search_key = "\"" + key + "\": \"";
    size_t start_pos = json_response.find(search_key);
    if (start_pos == std::string::npos) {
        // Fallback for Gemini's structure, search for the 'text' field deep inside
        search_key = "\"text\": \"";
        start_pos = json_response.find(search_key);
        if (start_pos == std::string::npos) return "";
    }
    
    start_pos += search_key.length();
    size_t end_pos = start_pos;
    bool in_escape = false;
    
    while (end_pos < json_response.length()) {
        if (json_response[end_pos] == '\\' && !in_escape) {
            in_escape = true;
        } else if (json_response[end_pos] == '"' && !in_escape) {
            break; 
        } else {
            in_escape = false;
        }
        end_pos++;
    }
    
    std::string raw = json_response.substr(start_pos, end_pos - start_pos);
    
    // Clean up basic escapes like \n
    std::string cleaned;
    for (size_t i = 0; i < raw.length(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.length()) {
            if (raw[i+1] == 'n') { cleaned += '\n'; i++; }
            else if (raw[i+1] == '"') { cleaned += '"'; i++; }
            else if (raw[i+1] == '\\') { cleaned += '\\'; i++; }
            else { cleaned += raw[i]; }
        } else {
            cleaned += raw[i];
        }
    }
    
    // Trim leading/trailing whitespace
    size_t first = cleaned.find_first_not_of(" \n\r\t");
    if (first == std::string::npos) return "";
    size_t last = cleaned.find_last_not_of(" \n\r\t");
    return cleaned.substr(first, (last - first + 1));
}

bool AIBridge::IsSafeCommand(const std::string& command) {
    if (command.empty()) return false;
    
    std::vector<std::string> dangerous_substrings = {
        "rm -rf", "shred", "> /dev/sda", "mkfs", "dd if=", "wipefs"
    };

    for (const auto& danger : dangerous_substrings) {
        if (command.find(danger) != std::string::npos) {
            std::cerr << "[SHIA-AI-Bridge] 🚨 SAFETY VIOLATION DETECTED: Command contains '" << danger << "'" << std::endl;
            return false;
        }
    }
    return true;
}

std::string AIBridge::GetFallbackFix(const std::string& error_log, const std::string& file_context) {
    std::cout << "[SHIA-AI-Bridge] Requesting fallback fix from Gemini API..." << std::endl;
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[SHIA-AI-Bridge] Failed to initialize libcurl" << std::endl;
        return "";
    }

    std::string prompt = "Act as a senior SRE. I have an error in my infrastructure: " + error_log + 
                         "\\nThe relevant file content is: " + file_context + 
                         "\\nProvide ONLY the corrected line(s) of code or the specific command to fix this. "
                         "No explanations. No markdown blocks. Just the raw fix.";
                         
    std::string json_data = "{"
                            "\"contents\": [{"
                            "  \"parts\":[{\"text\": \"" + EscapeJsonString(prompt) + "\"}]"
                            "}]"
                            "}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    std::string response_string;

    curl_easy_setopt(curl, CURLOPT_URL, GEMINI_API_URL.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    // Ignore SSL verification for demo simplicity if certificates are missing locally
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "[SHIA-AI-Bridge] curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        return "";
    }

    std::cout << "[SHIA-AI-Bridge] Raw Response: " << response_string << std::endl;
    
    std::string extracted_fix = ExtractJsonString(response_string, "text");
    
    if (extracted_fix.empty()) {
        std::cerr << "[SHIA-AI-Bridge] Failed to parse fix from response." << std::endl;
        return "";
    }

    if (!IsSafeCommand(extracted_fix)) {
        std::cerr << "[SHIA-AI-Bridge] Fix rejected by Safe Filter: " << extracted_fix << std::endl;
        return "";
    }

    return extracted_fix;
}
