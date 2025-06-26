/**
 * @file rte.hpp
 * @brief Runtime Environment (RTE) - AUTOSAR C++ Implementation
 * 
 * The RTE is the heart of AUTOSAR architecture, providing:
 * - Communication routing between Software Components
 * - Data consistency and synchronization
 * - Scheduling coordination
 * - Port connection management
 * 
 * This simplified RTE demonstrates the core concepts while remaining
 * educational and comprehensible.
 */

#ifndef RTE_HPP
#define RTE_HPP

#include "swc_base.hpp"
#include "autosar_types.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <functional>

namespace autosar {

// Forward declarations
class SoftwareComponent;

/**
 * @brief Port connection information
 * 
 * Represents a connection between sender and receiver ports
 */
struct PortConnection {
    std::string sender_swc_name;
    std::string sender_port_name;
    std::string receiver_swc_name;
    std::string receiver_port_name;
    std::shared_ptr<Interface> interface;
    
    PortConnection(const std::string& sender_swc, const std::string& sender_port,
                   const std::string& receiver_swc, const std::string& receiver_port,
                   std::shared_ptr<Interface> iface)
        : sender_swc_name(sender_swc), sender_port_name(sender_port)
        , receiver_swc_name(receiver_swc), receiver_port_name(receiver_port)
        , interface(iface) {}
};

/**
 * @brief SWC scheduling information
 */
struct SWCScheduleInfo {
    std::string swc_name;
    uint32 cycle_time_ms;
    std::chrono::steady_clock::time_point last_execution;
    
    SWCScheduleInfo(const std::string& name, uint32 cycle_ms)
        : swc_name(name), cycle_time_ms(cycle_ms), last_execution(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Runtime Environment (RTE)
 * 
 * The RTE provides the communication infrastructure for AUTOSAR Software Components.
 * It handles:
 * - Port-to-port data routing
 * - Data consistency and thread safety
 * - Basic scheduling coordination
 * - SWC lifecycle management
 * 
 * In production AUTOSAR, the RTE is typically generated automatically from
 * component descriptions. This implementation provides the core functionality
 * in a simplified, educational form.
 */
class RTE {
public:
    /**
     * @brief Constructor
     */
    RTE();
    
    /**
     * @brief Destructor
     */
    ~RTE();
    
    // SWC lifecycle management
    Std_ReturnType registerSWC(std::shared_ptr<SoftwareComponent> swc, uint32 cycle_time_ms);
    Std_ReturnType unregisterSWC(const std::string& swc_name);
    
    // Port connection management
    Std_ReturnType connectPorts(const std::string& sender_swc, const std::string& sender_port,
                                const std::string& receiver_swc, const std::string& receiver_port);
    
    // RTE lifecycle
    Std_ReturnType start();
    Std_ReturnType stop();
    
    // Data exchange functions (called by SWCs through generated stubs)
    template<typename T>
    Std_ReturnType writeData(const std::string& swc_name, const std::string& port_name, const T& data);
    
    template<typename T>
    Std_ReturnType readData(const std::string& swc_name, const std::string& port_name, T& data);
    
    // Status and debugging
    bool isRunning() const { return is_running_; }
    size_t getSWCCount() const { return swcs_.size(); }
    size_t getConnectionCount() const { return connections_.size(); }
    void printConnections() const;
    void printSWCStatus() const;
    
private:
    // SWC management
    std::unordered_map<std::string, std::shared_ptr<SoftwareComponent>> swcs_;
    std::vector<SWCScheduleInfo> schedule_info_;
    
    // Port connections
    std::vector<PortConnection> connections_;
    
    // Thread synchronization
    mutable std::mutex rte_mutex_;
    std::atomic<bool> is_running_;
    std::unique_ptr<std::thread> scheduler_thread_;
    std::atomic<bool> should_run_;
    
    // Statistics
    std::atomic<uint64> data_exchanges_;
    std::atomic<uint64> scheduler_cycles_;
    
    // Private methods
    void schedulerLoop();
    void executeSWCIfReady(const SWCScheduleInfo& info, std::shared_ptr<SoftwareComponent> swc);
    std::shared_ptr<SoftwareComponent> findSWC(const std::string& name);
    std::shared_ptr<Port> findPort(const std::string& swc_name, const std::string& port_name);
    void routeDataBetweenPorts();
    void logRTEActivity(const std::string& message) const;
    
    // Data routing helpers
    void routeSpecificConnection(const PortConnection& connection);
    template<typename T>
    bool routeTypedData(std::shared_ptr<Port> sender_port, std::shared_ptr<Port> receiver_port);
    
    // Configuration constants
    static constexpr uint32 RTE_SCHEDULER_CYCLE_MS = 5;  // 200Hz scheduler
    static constexpr uint64 STATISTICS_LOG_INTERVAL = 1000;  // Log every 1000 cycles
};

// Template implementations (must be in header for C++)

template<typename T>
Std_ReturnType RTE::writeData(const std::string& swc_name, const std::string& port_name, const T& data) {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    auto port = findPort(swc_name, port_name);
    if (!port) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (port->getDirection() != PortDirection::SENDER) {
        logRTEActivity("Error: Attempted to write to non-sender port " + swc_name + ":" + port_name);
        return Std_ReturnType::E_NOT_OK;
    }
    
    port->setData(&data);
    data_exchanges_++;
    
    return Std_ReturnType::E_OK;
}

template<typename T>
Std_ReturnType RTE::readData(const std::string& swc_name, const std::string& port_name, T& data) {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    auto port = findPort(swc_name, port_name);
    if (!port) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (port->getDirection() != PortDirection::RECEIVER) {
        logRTEActivity("Error: Attempted to read from non-receiver port " + swc_name + ":" + port_name);
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (port->getData(&data)) {
        data_exchanges_++;
        return Std_ReturnType::E_OK;
    }
    
    return Std_ReturnType::E_NOT_OK;
}

template<typename T>
bool RTE::routeTypedData(std::shared_ptr<Port> sender_port, std::shared_ptr<Port> receiver_port) {
    // This is a simplified implementation - in production AUTOSAR,
    // the RTE generator would create type-specific routing functions
    
    auto typed_sender = std::dynamic_pointer_cast<TypedPort<T>>(sender_port);
    auto typed_receiver = std::dynamic_pointer_cast<TypedPort<T>>(receiver_port);
    
    if (typed_sender && typed_receiver) {
        T data;
        if (typed_sender->read(data)) {
            typed_receiver->write(data);
            return true;
        }
    }
    
    return false;
}

} // namespace autosar

#endif // RTE_HPP