/**
 * @file engine_swc.cpp
 * @brief Implementation of Engine Software Component
 */

#include "../include/engine_swc.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace autosar {

EngineSWC::EngineSWC(std::shared_ptr<CANInterface> can_interface)
    : SoftwareComponent("EngineSWC")
    , can_interface_(can_interface)
    , random_generator_(std::chrono::steady_clock::now().time_since_epoch().count())
    , rpm_variation_(-50, 100)
    , temp_variation_(-0.5f, 1.0f)
    , cycle_count_(0) {
    
    if (!can_interface_) {
        throw std::invalid_argument("CAN interface cannot be null");
    }
    
    // Initialize engine data to realistic idle values using C++20 designated initializers
    current_engine_data_ = EngineData{
        .rpm = MIN_RPM,
        .temperature = 90,
        .throttle_position = 0
    };
    
    last_execution_ = std::chrono::steady_clock::now();
}

Std_ReturnType EngineSWC::initialize() {
    log("Initializing Engine SWC");
    
    // Create and add AUTOSAR ports
    auto engine_interface = std::make_shared<EngineDataInterface>();
    auto engine_port = std::make_shared<TypedPort<EngineData>>(
        "EngineDataPort", 
        PortDirection::SENDER, 
        engine_interface
    );
    
    addPort(engine_port);
    
    // Initialize port with current data
    updatePortData();
    
    log("Engine SWC initialized successfully");
    return Std_ReturnType::E_OK;
}

void EngineSWC::runnable() {
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - last_execution_
    ).count();
    
    // Only execute if enough time has passed (10Hz = 100ms)
    if (elapsed >= ENGINE_CYCLE_TIME_MS) {
        cycle_count_++;
        
        try {
            // 1. Simulate engine parameter changes
            simulateEngineParameters();
            
            // 2. Update AUTOSAR port data
            updatePortData();
            
            // 3. Send data via CAN (BSW layer)
            sendEngineDataViaCAN();
            
            // 4. Log status periodically
            if (cycle_count_ % 20 == 0) {  // Every 2 seconds at 10Hz
                std::stringstream ss;
                ss << "Engine Status - RPM: " << std::setw(4) << current_engine_data_.rpm
                   << ", Temp: " << std::setw(3) << static_cast<int>(current_engine_data_.temperature) << "°C"
                   << ", Throttle: " << std::setw(3) << static_cast<int>(current_engine_data_.throttle_position) << "%"
                   << " (Cycle: " << cycle_count_ << ")";
                log(ss.str());
            }
            
        } catch (const std::exception& e) {
            log("Error in runnable: " + std::string(e.what()));
        }
        
        last_execution_ = current_time;
    }
}

Std_ReturnType EngineSWC::writeEngineData(const EngineData& data) {
    auto port = std::dynamic_pointer_cast<TypedPort<EngineData>>(getPort("EngineDataPort"));
    if (port) {
        port->write(data);
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType EngineSWC::readEngineData(EngineData& data) const {
    auto port = std::dynamic_pointer_cast<TypedPort<EngineData>>(getPort("EngineDataPort"));
    if (port && port->read(data)) {
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

void EngineSWC::simulateEngineParameters() {
    // Simulate RPM changes (idle to redline behavior)
    current_engine_data_.rpm = simulateRPMChange();
    
    // Simulate temperature changes (thermal dynamics)
    current_engine_data_.temperature = simulateTemperatureChange();
    
    // Simulate throttle position (driver input)
    current_engine_data_.throttle_position = simulateThrottlePosition();
}

void EngineSWC::sendEngineDataViaCAN() {
    // Send via CAN bus (this represents the AUTOSAR BSW COM module)
    Std_ReturnType result = can_interface_->sendEngineData(current_engine_data_);
    
    if (result != Std_ReturnType::E_OK) {
        log("Warning: Failed to send engine data via CAN");
    }
}

void EngineSWC::updatePortData() {
    // Update the AUTOSAR port with current data
    // In a real system, this would be handled automatically by the RTE
    writeEngineData(current_engine_data_);
}

uint16 EngineSWC::simulateRPMChange() {
    // Realistic RPM simulation
    int rpm_change = rpm_variation_(random_generator_);
    int new_rpm = static_cast<int>(current_engine_data_.rpm) + rpm_change;
    
    // Apply realistic constraints
    new_rpm = std::max(static_cast<int>(MIN_RPM), new_rpm);
    new_rpm = std::min(static_cast<int>(MAX_RPM), new_rpm);
    
    // Add some engine load simulation
    if (current_engine_data_.throttle_position > 50) {
        // Under load, RPM tends to increase more
        new_rpm += 20;
    } else if (current_engine_data_.throttle_position == 0) {
        // No throttle, drift towards idle
        int idle_difference = static_cast<int>(MIN_RPM) - new_rpm;
        new_rpm += idle_difference / 10;  // Gradual return to idle
    }
    
    return static_cast<uint16>(std::clamp(new_rpm, static_cast<int>(MIN_RPM), static_cast<int>(MAX_RPM)));
}

uint8 EngineSWC::simulateTemperatureChange() {
    // Engine temperature simulation
    float temp_change = temp_variation_(random_generator_);
    float new_temp = static_cast<float>(current_engine_data_.temperature) + temp_change;
    
    // Temperature depends on RPM (higher RPM = more heat)
    if (current_engine_data_.rpm > 3000) {
        new_temp += 0.1f;  // Heating up under high RPM
    } else if (current_engine_data_.rpm < 1500) {
        new_temp -= 0.05f;  // Cooling down at low RPM
    }
    
    // Constrain to realistic range
    new_temp = std::clamp(new_temp, static_cast<float>(MIN_TEMP), static_cast<float>(MAX_TEMP));
    
    return static_cast<uint8>(new_temp);
}

uint8 EngineSWC::simulateThrottlePosition() {
    // Simple throttle simulation (in real system, this comes from pedal sensor)
    static uint32 throttle_counter = 0;
    throttle_counter++;
    
    // Change throttle position occasionally for demo
    if (throttle_counter % 50 == 0) {  // Every 5 seconds at 10Hz
        std::uniform_int_distribution<int> throttle_dist(0, 100);
        return static_cast<uint8>(throttle_dist(random_generator_));
    }
    
    return current_engine_data_.throttle_position;
}

} // namespace autosar