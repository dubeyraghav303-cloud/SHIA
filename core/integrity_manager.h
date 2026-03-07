#ifndef INTEGRITY_MANAGER_H
#define INTEGRITY_MANAGER_H

#include <string>

class IntegrityManager {
public:
    static std::string ComputeSHA256(const std::string& data);
};

#endif // INTEGRITY_MANAGER_H
