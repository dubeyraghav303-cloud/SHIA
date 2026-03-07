#include "telemetry_bridge.h"
#include <iostream>
#include <sstream>
#include <curl/curl.h>

std::string TelemetryBridge::SUPABASE_URL = "";
std::string TelemetryBridge::SUPABASE_KEY = "";
std::string TelemetryBridge::SESSION_ID = "";

// libcurl write callback
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string TelemetryBridge::EscapeJsonString(const std::string& input) {
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

void TelemetryBridge::Initialize(const std::string& supabase_url, const std::string& supabase_key, const std::string& session_id) {
    SUPABASE_URL = supabase_url;
    SUPABASE_KEY = supabase_key;
    SESSION_ID = session_id;

    if (SUPABASE_URL.empty() || SUPABASE_KEY.empty()) {
        std::cerr << "\n[SHIA-Telemetry] ⚠️  WARNING: Supabase URL or Key is missing from environment!" << std::endl;
        std::cerr << "[SHIA-Telemetry] -> Telemetry pipeline is disabled. Database will not be updated.\n" << std::endl;
    }
}

bool TelemetryBridge::SendIncident(const std::string& timestamp, 
                                   const std::string& service_name, 
                                   const std::string& error_log, 
                                   const std::string& action, 
                                   bool ai_patch, 
                                   const std::string& status) {
    
    if (SUPABASE_URL.empty() || SUPABASE_KEY.empty()) {
        std::cerr << "[SHIA-Telemetry] Supabase URL or Key not set, skipping telemetry export." << std::endl;
        return false;
    }

    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "[SHIA-Telemetry] Failed to initialize libcurl" << std::endl;
        return false;
    }

    std::string json_data = "{";
    json_data += "\"session_id\": \"" + EscapeJsonString(SESSION_ID) + "\",";
    json_data += "\"timestamp\": \"" + EscapeJsonString(timestamp) + "\",";
    json_data += "\"service\": \"" + EscapeJsonString(service_name) + "\",";
    json_data += "\"error\": \"" + EscapeJsonString(error_log) + "\",";
    json_data += "\"action\": \"" + EscapeJsonString(action) + "\",";
    json_data += "\"ai_patch\": " + std::string(ai_patch ? "true" : "false") + ",";
    json_data += "\"status\": \"" + EscapeJsonString(status) + "\"";
    json_data += "}";

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("apikey: " + SUPABASE_KEY).c_str());
    headers = curl_slist_append(headers, ("Authorization: Bearer " + SUPABASE_KEY).c_str());
    headers = curl_slist_append(headers, "Prefer: return=representation"); // Returns inserted row

    std::string response_string;
    std::string url = SUPABASE_URL + "/rest/v1/incidents";

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    
    // Ignore SSL verification for demo if local certificates fail
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "[SHIA-Telemetry] HTTP Request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    if (http_code >= 200 && http_code < 300) {
        std::cout << "[SHIA-Telemetry] Successfully synced incident (" << status << ") to dashboard." << std::endl;
        return true;
    } else {
        std::cerr << "\n[SHIA-Telemetry] ❌ SUPABASE POST FAILED: HTTP " << http_code << std::endl;
        if (http_code == 401 || http_code == 403) {
            std::cerr << " -> ERROR: Unauthorized. Check if your SUPABASE_KEY (Anon/Service Role) is correct and active." << std::endl;
        } else if (http_code == 404) {
            std::cerr << " -> ERROR: Not Found. Check if your SUPABASE_URL is correct and the 'incidents' table exists via schema.sql." << std::endl;
        }
        std::cerr << " -> RAW RESPONSE: " << response_string << std::endl;
        std::cerr << "------------------------------------------\n";
        return false;
    }
}
