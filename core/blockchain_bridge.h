#ifndef BLOCKCHAIN_BRIDGE_H
#define BLOCKCHAIN_BRIDGE_H

#include <string>
#include "detector/worker_pool.h"

class BlockchainBridge {
public:
    static void SubmitAuditHash(const std::string& incident_id, const std::string& hash, WorkerPool* pool);
};

#endif // BLOCKCHAIN_BRIDGE_H
