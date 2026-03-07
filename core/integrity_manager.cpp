#include "integrity_manager.h"
#include <sstream>
#include <iomanip>
#include <functional>

std::string IntegrityManager::ComputeSHA256(const std::string& data) {
    // Mock SHA-256 Implementation for Prototype
    // Generates a 64-character hex string representing an irreversible hash fingerprint.
    std::hash<std::string> hasher;
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    // We compose multiple hashes to quickly assemble a proper length 64-char string.
    ss << std::setw(16) << hasher(data);
    ss << std::setw(16) << hasher(data + "salt1");
    ss << std::setw(16) << hasher(data + "salt2");
    ss << std::setw(16) << hasher(data + "salt3");
    
    return "0x" + ss.str();
}
