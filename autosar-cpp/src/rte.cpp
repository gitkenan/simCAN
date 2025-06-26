/**
 * @file rte.cpp
 * @brief Implementation of Runtime Environment (RTE)
 */

#include "../include/rte.hpp"
#include "../include/engine_swc.hpp"
#include "../include/body_swc.hpp"
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace autosar {

RTE::RTE() 
    : is_running_(false)
    , should_run_(false)
    , data_exchanges_(0)
    , scheduler_cycles_(0) {
    logRTEActivity("RTE created");
}

RTE::~RTE() {
    stop();
    logRTEActivity("RTE destroyed");
}

Std_ReturnType RTE::registerSWC(std::shared_ptr<SoftwareComponent> swc, uint32 cycle_time_ms) {
    if (!swc) {
        logRTEActivity("Error: Cannot register null SWC");
        return Std_ReturnType::E_NOT_OK;
    }
    
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    const std::string& swc_name = swc->getName();
    
    // Check if SWC already registered
    if (swcs_.find(swc_name) != swcs_.end()) {
        logRTEActivity("Warning: SWC " + swc_name + " already registered");
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Register SWC
    swcs_[swc_name] = swc;
    schedule_info_.emplace_back(swc_name, cycle_time_ms);
    
    // Set RTE reference in SWC
    swc->setRTE(this);
    
    logRTEActivity("Registered SWC: " + swc_name + " (cycle: " + std::to_string(cycle_time_ms) + "ms)");
    return Std_ReturnType::E_OK;
}

Std_ReturnType RTE::unregisterSWC(const std::string& swc_name) {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    auto swc_it = swcs_.find(swc_name);
    if (swc_it == swcs_.end()) {
        logRTEActivity("Warning: SWC " + swc_name + " not found for unregistration");
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Stop SWC if running
    swc_it->second->stop();
    
    // Remove from schedule
    schedule_info_.erase(
        std::remove_if(schedule_info_.begin(), schedule_info_.end(),
                      [&swc_name](const SWCScheduleInfo& info) {
                          return info.swc_name == swc_name;
                      }),
        schedule_info_.end()
    );
    
    // Remove from SWC map
    swcs_.erase(swc_it);
    
    logRTEActivity("Unregistered SWC: " + swc_name);
    return Std_ReturnType::E_OK;
}

Std_ReturnType RTE::connectPorts(const std::string& sender_swc, const std::string& sender_port,
                                 const std::string& receiver_swc, const std::string& receiver_port) {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    // Find sender port
    auto sender_port_ptr = findPort(sender_swc, sender_port);
    if (!sender_port_ptr) {
        logRTEActivity("Error: Sender port not found: " + sender_swc + ":" + sender_port);
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Find receiver port
    auto receiver_port_ptr = findPort(receiver_swc, receiver_port);
    if (!receiver_port_ptr) {
        logRTEActivity("Error: Receiver port not found: " + receiver_swc + ":" + receiver_port);
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Validate port directions
    if (sender_port_ptr->getDirection() != PortDirection::SENDER) {
        logRTEActivity("Error: Port " + sender_swc + ":" + sender_port + " is not a sender port");
        return Std_ReturnType::E_NOT_OK;
    }
    
    if (receiver_port_ptr->getDirection() != PortDirection::RECEIVER) {
        logRTEActivity("Error: Port " + receiver_swc + ":" + receiver_port + " is not a receiver port");
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Check interface compatibility
    auto sender_interface = sender_port_ptr->getInterface();
    auto receiver_interface = receiver_port_ptr->getInterface();
    
    if (sender_interface->getName() != receiver_interface->getName()) {
        logRTEActivity("Error: Interface mismatch between " + sender_swc + ":" + sender_port + 
                      " and " + receiver_swc + ":" + receiver_port);
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Create connection
    connections_.emplace_back(sender_swc, sender_port, receiver_swc, receiver_port, sender_interface);
    
    logRTEActivity("Connected " + sender_swc + ":" + sender_port + " -> " + 
                  receiver_swc + ":" + receiver_port + " (Interface: " + sender_interface->getName() + ")");
    
    return Std_ReturnType::E_OK;
}

Std_ReturnType RTE::start() {
    if (is_running_) {
        logRTEActivity("Warning: RTE already running");
        return Std_ReturnType::E_NOT_OK;
    }
    
    logRTEActivity("Starting RTE with " + std::to_string(swcs_.size()) + " SWCs and " + 
                  std::to_string(connections_.size()) + " connections");
    
    // Initialize all SWCs
    for (auto& [name, swc] : swcs_) {
        Std_ReturnType result = swc->initialize();
        if (result != Std_ReturnType::E_OK) {
            logRTEActivity("Error: Failed to initialize SWC " + name);
            return result;
        }
    }
    
    // Start scheduler
    is_running_ = true;
    should_run_ = true;
    scheduler_thread_ = std::make_unique<std::thread>(&RTE::schedulerLoop, this);
    
    // Start all SWCs
    for (auto& [name, swc] : swcs_) {
        swc->start();
    }
    
    logRTEActivity("RTE started successfully");
    return Std_ReturnType::E_OK;
}

Std_ReturnType RTE::stop() {
    if (!is_running_) {
        return Std_ReturnType::E_OK;
    }
    
    logRTEActivity("Stopping RTE...");
    
    // Stop all SWCs first
    for (auto& [name, swc] : swcs_) {
        swc->stop();
    }
    
    // Stop scheduler
    should_run_ = false;
    is_running_ = false;
    
    if (scheduler_thread_ && scheduler_thread_->joinable()) {
        scheduler_thread_->join();
    }
    scheduler_thread_.reset();
    
    logRTEActivity("RTE stopped. Statistics: " + std::to_string(data_exchanges_) + 
                  " data exchanges, " + std::to_string(scheduler_cycles_) + " scheduler cycles");
    
    return Std_ReturnType::E_OK;
}

void RTE::printConnections() const {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    std::cout << "\n=== RTE Port Connections ===" << std::endl;
    for (const auto& conn : connections_) {
        std::cout << conn.sender_swc_name << ":" << conn.sender_port_name 
                  << " -> " << conn.receiver_swc_name << ":" << conn.receiver_port_name
                  << " (Interface: " << conn.interface->getName() << ")" << std::endl;
    }
    std::cout << "Total connections: " << connections_.size() << std::endl;
}

void RTE::printSWCStatus() const {
    std::lock_guard<std::mutex> lock(rte_mutex_);
    
    std::cout << "\n=== SWC Status ===" << std::endl;
    for (const auto& [name, swc] : swcs_) {
        std::string state_str;
        switch (swc->getState()) {
            case SWCState::STOPPED: state_str = "STOPPED"; break;
            case SWCState::RUNNING: state_str = "RUNNING"; break;
            case SWCState::ERROR:   state_str = "ERROR"; break;
        }
        
        std::cout << name << ": " << state_str << " (" << swc->getPorts().size() << " ports)" << std::endl;
    }
}

void RTE::schedulerLoop() {
    logRTEActivity("RTE scheduler started");
    
    while (should_run_) {
        try {
            scheduler_cycles_++;
            
            // Execute SWCs based on their timing requirements
            auto current_time = std::chrono::steady_clock::now();
            
            for (auto& info : schedule_info_) {
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    current_time - info.last_execution
                ).count();
                
                if (elapsed >= info.cycle_time_ms) {
                    auto swc = findSWC(info.swc_name);
                    if (swc) {
                        executeSWCIfReady(info, swc);
                        info.last_execution = current_time;
                    }
                }
            }
            
            // Route data between connected ports
            routeDataBetweenPorts();
            
            // Log statistics periodically
            if (scheduler_cycles_ % STATISTICS_LOG_INTERVAL == 0) {
                logRTEActivity("Scheduler cycle " + std::to_string(scheduler_cycles_) + 
                              ", Data exchanges: " + std::to_string(data_exchanges_));
            }
            
            // Small delay to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(RTE_SCHEDULER_CYCLE_MS));
            
        } catch (const std::exception& e) {
            logRTEActivity("Exception in scheduler: " + std::string(e.what()));
        }
    }
    
    logRTEActivity("RTE scheduler stopped");
}

void RTE::executeSWCIfReady(const SWCScheduleInfo& info, std::shared_ptr<SoftwareComponent> swc) {
    if (swc->getState() == SWCState::RUNNING) {
        // In this simplified implementation, we let SWCs manage their own execution
        // In production AUTOSAR, the RTE would directly call runnable entities
        // based on precise scheduling requirements
    }
}

std::shared_ptr<SoftwareComponent> RTE::findSWC(const std::string& name) {
    auto it = swcs_.find(name);
    return (it != swcs_.end()) ? it->second : nullptr;
}

std::shared_ptr<Port> RTE::findPort(const std::string& swc_name, const std::string& port_name) {
    auto swc = findSWC(swc_name);
    return swc ? swc->getPort(port_name) : nullptr;
}

void RTE::routeDataBetweenPorts() {
    // Route data for all connections
    for (const auto& connection : connections_) {
        routeSpecificConnection(connection);
    }
}

void RTE::routeSpecificConnection(const PortConnection& connection) {
    auto sender_port = findPort(connection.sender_swc_name, connection.sender_port_name);
    auto receiver_port = findPort(connection.receiver_swc_name, connection.receiver_port_name);
    
    if (!sender_port || !receiver_port) {
        return;
    }
    
    // Try to route data based on known interface types
    const std::string& interface_name = connection.interface->getName();
    
    if (interface_name == "EngineDataInterface") {
        if (routeTypedData<EngineData>(sender_port, receiver_port)) {
            data_exchanges_++;
        }
    } else if (interface_name == "DoorStatusInterface") {
        if (routeTypedData<DoorStatus>(sender_port, receiver_port)) {
            data_exchanges_++;
        }
    } else if (interface_name == "LightControlInterface") {
        if (routeTypedData<LightControl>(sender_port, receiver_port)) {
            data_exchanges_++;
        }
    }
}

void RTE::logRTEActivity(const std::string& message) const {
    std::cout << "[RTE] " << message << std::endl;
}

} // namespace autosar