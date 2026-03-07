#ifndef RECOVERY_CONTROLLER_H
#define RECOVERY_CONTROLLER_H

#include <string>

class RecoveryController {
public:
    static bool ExecuteAction(const std::string& action, const std::string& service_name, const std::string& error_log, const std::string& file_context, bool enable_ai_repair);

private:
   static bool RestartService(const std::string& service_name);
   static bool FlushCache(const std::string& service_name);
   static bool CleanTmp();
};

#endif // RECOVERY_CONTROLLER_H
