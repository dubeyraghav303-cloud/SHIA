#ifndef TELEMETRY_BRIDGE_H
#define TELEMETRY_BRIDGE_H

#include <string>

class TelemetryBridge {
public:
    static void Initialize(const std::string& supabase_url, const std::string& supabase_key, const std::string& session_id);
    static bool SendIncident(const std::string& timestamp, 
                             const std::string& service_name, 
                             const std::string& error_log, 
                             const std::string& action, 
                             bool ai_patch, 
                             const std::string& status);

private:
   static std::string ExtractJsonString(const std::string& json_response, const std::string& key);
   static std::string EscapeJsonString(const std::string& input);
   
   static std::string SUPABASE_URL;
   static std::string SUPABASE_KEY;
   static std::string SESSION_ID;
};

#endif // TELEMETRY_BRIDGE_H
