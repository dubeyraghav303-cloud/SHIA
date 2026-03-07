#ifndef AI_BRIDGE_H
#define AI_BRIDGE_H

#include <string>

class AIBridge {
public:
    static std::string GetFallbackFix(const std::string& error_log, const std::string& file_context);
    static bool IsSafeCommand(const std::string& command);

private:
   static std::string ExtractJsonString(const std::string& json_response, const std::string& key);
   static std::string EscapeJsonString(const std::string& input);
};

#endif // AI_BRIDGE_H
