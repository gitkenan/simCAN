/**
 * @file swc_base.hpp
 * @brief Base classes for AUTOSAR Software Components
 * 
 * Defines the fundamental AUTOSAR SWC concepts:
 * - Software Component base class
 * - Port definitions
 * - Interface specifications
 * - Runnable entities
 */

#ifndef SWC_BASE_HPP
#define SWC_BASE_HPP

#include "autosar_types.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <thread>
#include <atomic>
#include <iostream>

namespace autosar {

// Forward declarations
class RTE;
class Port;

/**
 * @brief Interface definition for AUTOSAR communication
 * 
 * In AUTOSAR, interfaces define the contract for communication between SWCs.
 * They specify what data elements are exchanged and how.
 */
class Interface {
public:
    Interface(const std::string& name, InterfaceType type) 
        : name_(name), type_(type) {}
    
    virtual ~Interface() = default;
    
    const std::string& getName() const { return name_; }
    InterfaceType getType() const { return type_; }
    
protected:
    std::string name_;
    InterfaceType type_;
};

/**
 * @brief AUTOSAR Port - connection point for SWC communication
 * 
 * Ports are the communication endpoints of Software Components.
 * They are typed by interfaces and have a direction (sender/receiver).
 */
class Port {
public:
    Port(const std::string& name, PortDirection direction, 
         std::shared_ptr<Interface> interface)
        : name_(name), direction_(direction), interface_(interface) {}
    
    virtual ~Port() = default;
    
    const std::string& getName() const { return name_; }
    PortDirection getDirection() const { return direction_; }
    std::shared_ptr<Interface> getInterface() const { return interface_; }
    
    // Port data access (simplified - in real AUTOSAR this is handled by RTE)
    virtual void setData(const void* data) = 0;
    virtual bool getData(void* data) const = 0;
    
protected:
    std::string name_;
    PortDirection direction_;
    std::shared_ptr<Interface> interface_;
};

/**
 * @brief Typed port template for specific data types
 */
template<typename T>
class TypedPort : public Port {
public:
    TypedPort(const std::string& name, PortDirection direction,
              std::shared_ptr<Interface> interface)
        : Port(name, direction, interface), has_data_(false) {}
    
    void setData(const void* data) override {
        if (data) {
            data_ = *static_cast<const T*>(data);
            has_data_ = true;
        }
    }
    
    bool getData(void* data) const override {
        if (has_data_ && data) {
            *static_cast<T*>(data) = data_;
            return true;
        }
        return false;
    }
    
    // Type-safe accessors
    void write(const T& data) {
        data_ = data;
        has_data_ = true;
    }
    
    bool read(T& data) const {
        if (has_data_) {
            data = data_;
            return true;
        }
        return false;
    }
    
    bool isValid() const { return has_data_; }
    
private:
    T data_;
    std::atomic<bool> has_data_;
};

/**
 * @brief Base class for AUTOSAR Software Components
 * 
 * Software Components are the main building blocks of AUTOSAR applications.
 * They encapsulate functionality and communicate through ports.
 */
class SoftwareComponent {
public:
    explicit SoftwareComponent(const std::string& name) 
        : name_(name), state_(SWCState::STOPPED), rte_(nullptr) {}
    
    virtual ~SoftwareComponent() {
        stop();
    }
    
    // Basic SWC lifecycle
    virtual Std_ReturnType initialize() = 0;
    virtual void start();
    virtual void stop();
    
    // Port management
    void addPort(std::shared_ptr<Port> port);
    std::shared_ptr<Port> getPort(const std::string& name) const;
    const std::vector<std::shared_ptr<Port>>& getPorts() const { return ports_; }
    
    // RTE connection
    void setRTE(RTE* rte) { rte_ = rte; }
    
    // Getters
    const std::string& getName() const { return name_; }
    SWCState getState() const { return state_; }
    
    // Main execution function - to be implemented by derived classes
    virtual void runnable() = 0;
    
protected:
    std::string name_;
    std::atomic<SWCState> state_;
    std::vector<std::shared_ptr<Port>> ports_;
    std::unordered_map<std::string, std::shared_ptr<Port>> port_map_;
    RTE* rte_;
    
    std::unique_ptr<std::thread> execution_thread_;
    std::atomic<bool> should_run_;
    
    // Logging helper
    void log(const std::string& message) const {
        std::cout << "[" << name_ << "] " << message << std::endl;
    }
    
private:
    void executionLoop();
};

/**
 * @brief Common AUTOSAR Interface Definitions
 * 
 * These interfaces are shared between multiple SWCs
 */

/**
 * @brief Engine Data Interface - defines contract for engine communication
 */
class EngineDataInterface : public Interface {
public:
    EngineDataInterface() : Interface("EngineDataInterface", InterfaceType::SENDER_RECEIVER) {}
    ~EngineDataInterface() override = default;
};

/**
 * @brief Door Status Interface - defines contract for door communication
 */
class DoorStatusInterface : public Interface {
public:
    DoorStatusInterface() : Interface("DoorStatusInterface", InterfaceType::SENDER_RECEIVER) {}
    ~DoorStatusInterface() override = default;
};

/**
 * @brief Light Control Interface - defines contract for lighting system
 */
class LightControlInterface : public Interface {
public:
    LightControlInterface() : Interface("LightControlInterface", InterfaceType::SENDER_RECEIVER) {}
    ~LightControlInterface() override = default;
};

} // namespace autosar

#endif // SWC_BASE_HPP