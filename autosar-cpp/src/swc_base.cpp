/**
 * @file swc_base.cpp
 * @brief Implementation of base AUTOSAR SWC functionality
 */

#include "../include/swc_base.hpp"
#include <chrono>
#include <thread>

namespace autosar {

void SoftwareComponent::start() {
    if (state_ == SWCState::RUNNING) {
        log("Already running");
        return;
    }
    
    log("Starting Software Component");
    should_run_ = true;
    state_ = SWCState::RUNNING;
    
    // Start execution thread
    execution_thread_ = std::make_unique<std::thread>(&SoftwareComponent::executionLoop, this);
}

void SoftwareComponent::stop() {
    if (state_ == SWCState::STOPPED) {
        return;
    }
    
    log("Stopping Software Component");
    should_run_ = false;
    state_ = SWCState::STOPPED;
    
    // Wait for execution thread to finish
    if (execution_thread_ && execution_thread_->joinable()) {
        execution_thread_->join();
    }
    execution_thread_.reset();
}

void SoftwareComponent::addPort(std::shared_ptr<Port> port) {
    ports_.push_back(port);
    port_map_[port->getName()] = port;
    
    std::string direction = (port->getDirection() == PortDirection::SENDER) ? "Sender" : "Receiver";
    log("Added " + direction + " port: " + port->getName());
}

std::shared_ptr<Port> SoftwareComponent::getPort(const std::string& name) const {
    auto it = port_map_.find(name);
    return (it != port_map_.end()) ? it->second : nullptr;
}

void SoftwareComponent::executionLoop() {
    log("Execution thread started");
    
    while (should_run_ && state_ == SWCState::RUNNING) {
        try {
            // Call the SWC's main runnable function
            runnable();
            
            // Small delay to prevent excessive CPU usage
            // In real AUTOSAR, this would be handled by the OS scheduler
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            
        } catch (const std::exception& e) {
            log("Exception in runnable: " + std::string(e.what()));
            state_ = SWCState::ERROR;
            break;
        }
    }
    
    log("Execution thread finished");
}

} // namespace autosar