/**
 * @file body_swc.cpp
 * @brief Implementation of Body Control Software Component
 */

#include "../include/body_swc.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace autosar {

BodyControlSWC::BodyControlSWC(std::shared_ptr<CANInterface> can_interface)
    : SoftwareComponent("BodyControlSWC")
    , can_interface_(can_interface)
    , engine_data_valid_(false)
    , auto_lights_enabled_(true)
    , received_new_engine_data_(false)
    , random_generator_(std::chrono::steady_clock::now().time_since_epoch().count())
    , activity_probability_(0.0f, 1.0f)
    , door_selection_(0, NUM_DOORS - 1)
    , window_position_(0, 100)
    , cycle_count_(0) {
    
    if (!can_interface_) {
        throw std::invalid_argument("CAN interface cannot be null");
    }
    
    // Initialize all body control systems to safe/default states using C++20 designated initializers
    current_door_status_ = DoorStatus::all_closed();
    current_light_control_ = LightControl::all_off();
    current_window_position_ = WindowPosition::all_closed();
    
    last_execution_ = std::chrono::steady_clock::now();
}

Std_ReturnType BodyControlSWC::initialize() {
    log("Initializing Body Control SWC");
    
    // Create and add AUTOSAR ports
    
    // Door status sender port
    auto door_interface = std::make_shared<DoorStatusInterface>();
    auto door_port = std::make_shared<TypedPort<DoorStatus>>(
        "DoorStatusPort",
        PortDirection::SENDER,
        door_interface
    );
    addPort(door_port);
    
    // Light control receiver port
    auto light_interface = std::make_shared<LightControlInterface>();
    auto light_port = std::make_shared<TypedPort<LightControl>>(
        "LightControlPort",
        PortDirection::RECEIVER,
        light_interface
    );
    addPort(light_port);
    
    // Engine data receiver port (for auto features)
    auto engine_interface = std::make_shared<EngineDataInterface>();
    auto engine_port = std::make_shared<TypedPort<EngineData>>(
        "EngineDataPort",
        PortDirection::RECEIVER,
        engine_interface
    );
    addPort(engine_port);
    
    // Initialize ports with current data
    updatePortData();
    
    log("Body Control SWC initialized successfully");
    return Std_ReturnType::E_OK;
}

void BodyControlSWC::runnable() {
    auto current_time = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        current_time - last_execution_
    ).count();
    
    // Only execute if enough time has passed (2Hz = 500ms)
    if (elapsed >= BODY_CYCLE_TIME_MS) {
        cycle_count_++;
        
        try {
            // 1. Receive data from other SWCs via ports
            receiveEngineDataFromPort();
            
            // 2. Simulate body control activities
            simulateBodyActivity();
            
            // 3. Process auto features based on received data
            processAutoFeatures();
            
            // 4. Update AUTOSAR port data
            updatePortData();
            
            // 5. Send data via CAN (BSW layer)
            sendBodyDataViaCAN();
            
            // 6. Log status periodically
            if (cycle_count_ % 10 == 0) {  // Every 5 seconds at 2Hz
                logCurrentStatus();
            }
            
        } catch (const std::exception& e) {
            log("Error in runnable: " + std::string(e.what()));
        }
        
        last_execution_ = current_time;
    }
}

Std_ReturnType BodyControlSWC::writeDoorStatus(const DoorStatus& status) {
    auto port = std::dynamic_pointer_cast<TypedPort<DoorStatus>>(getPort("DoorStatusPort"));
    if (port) {
        port->write(status);
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType BodyControlSWC::readDoorStatus(DoorStatus& status) const {
    auto port = std::dynamic_pointer_cast<TypedPort<DoorStatus>>(getPort("DoorStatusPort"));
    if (port && port->read(status)) {
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType BodyControlSWC::readLightControl(LightControl& control) const {
    auto port = std::dynamic_pointer_cast<TypedPort<LightControl>>(getPort("LightControlPort"));
    if (port && port->read(control)) {
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType BodyControlSWC::readEngineData(EngineData& data) const {
    auto port = std::dynamic_pointer_cast<TypedPort<EngineData>>(getPort("EngineDataPort"));
    if (port && port->read(data)) {
        return Std_ReturnType::E_OK;
    }
    return Std_ReturnType::E_NOT_OK;
}

void BodyControlSWC::setLightControl(const LightControl& control) {
    current_light_control_ = control;
    log("Light control updated via external command");
}

void BodyControlSWC::setDoorOpen(uint8 door_index, boolean is_open) {
    boolean* door_ptr = getDoorStatusPointer(door_index);
    if (door_ptr) {
        *door_ptr = is_open;
        std::string status = is_open ? "OPENED" : "CLOSED";
        log(std::string(getDoorName(door_index)) + " door " + status + " via external command");
    }
}

void BodyControlSWC::setWindowPosition(uint8 window_index, uint8 position) {
    uint8* window_ptr = getWindowPositionPointer(window_index);
    if (window_ptr) {
        *window_ptr = std::min(position, static_cast<uint8>(100));
        log(std::string(getWindowName(window_index)) + " window set to " + 
            std::to_string(position) + "% via external command");
    }
}

void BodyControlSWC::simulateBodyActivity() {
    simulateDoorActivity();
    simulateWindowActivity();
}

void BodyControlSWC::processAutoFeatures() {
    if (engine_data_valid_ && received_new_engine_data_) {
        processAutoLights();
        received_new_engine_data_ = false;
    }
}

void BodyControlSWC::sendBodyDataViaCAN() {
    // Send door status via CAN
    Std_ReturnType door_result = can_interface_->sendDoorStatus(current_door_status_);
    if (door_result != Std_ReturnType::E_OK) {
        log("Warning: Failed to send door status via CAN");
    }
    
    // Send light status via CAN
    Std_ReturnType light_result = can_interface_->sendLightStatus(current_light_control_);
    if (light_result != Std_ReturnType::E_OK) {
        log("Warning: Failed to send light status via CAN");
    }
}

void BodyControlSWC::updatePortData() {
    // Update door status port
    writeDoorStatus(current_door_status_);
    
    // Read any new light control commands
    LightControl new_light_control;
    if (readLightControl(new_light_control) == Std_ReturnType::E_OK) {
        current_light_control_ = new_light_control;
    }
}

void BodyControlSWC::receiveEngineDataFromPort() {
    EngineData new_engine_data;
    if (readEngineData(new_engine_data) == Std_ReturnType::E_OK) {
        received_engine_data_ = new_engine_data;
        engine_data_valid_ = true;
        received_new_engine_data_ = true;
    }
}

void BodyControlSWC::simulateDoorActivity() {
    if (activity_probability_(random_generator_) < DOOR_ACTIVITY_CHANCE) {
        uint8 door_index = static_cast<uint8>(door_selection_(random_generator_));
        boolean* door_ptr = getDoorStatusPointer(door_index);
        
        if (door_ptr) {
            *door_ptr = !(*door_ptr);
            std::string status = *door_ptr ? "OPENED" : "CLOSED";
            log(std::string(getDoorName(door_index)) + " door " + status + " (simulation)");
        }
    }
}

void BodyControlSWC::simulateWindowActivity() {
    if (activity_probability_(random_generator_) < WINDOW_ACTIVITY_CHANCE) {
        uint8 window_index = static_cast<uint8>(door_selection_(random_generator_)); // Reuse door selection
        uint8* window_ptr = getWindowPositionPointer(window_index);
        
        if (window_ptr) {
            uint8 new_position = static_cast<uint8>(window_position_(random_generator_));
            *window_ptr = new_position;
            log(std::string(getWindowName(window_index)) + " window moved to " + 
                std::to_string(new_position) + "% (simulation)");
        }
    }
}

void BodyControlSWC::processAutoLights() {
    if (!auto_lights_enabled_) {
        return;
    }
    
    boolean should_be_on = received_engine_data_.rpm > AUTO_LIGHT_RPM_THRESHOLD;
    
    if (current_light_control_.headlights_on != should_be_on) {
        current_light_control_.headlights_on = should_be_on;
        std::string status = should_be_on ? "ON" : "OFF";
        log("Auto headlights: " + status + " (RPM: " + std::to_string(received_engine_data_.rpm) + ")");
    }
}

boolean* BodyControlSWC::getDoorStatusPointer(uint8 door_index) {
    switch (door_index) {
        case 0: return &current_door_status_.front_left_open;
        case 1: return &current_door_status_.front_right_open;
        case 2: return &current_door_status_.rear_left_open;
        case 3: return &current_door_status_.rear_right_open;
        default: return nullptr;
    }
}

uint8* BodyControlSWC::getWindowPositionPointer(uint8 window_index) {
    switch (window_index) {
        case 0: return &current_window_position_.front_left;
        case 1: return &current_window_position_.front_right;
        case 2: return &current_window_position_.rear_left;
        case 3: return &current_window_position_.rear_right;
        default: return nullptr;
    }
}

const char* BodyControlSWC::getDoorName(uint8 door_index) const {
    switch (door_index) {
        case 0: return "Front left";
        case 1: return "Front right";
        case 2: return "Rear left";
        case 3: return "Rear right";
        default: return "Unknown";
    }
}

const char* BodyControlSWC::getWindowName(uint8 window_index) const {
    switch (window_index) {
        case 0: return "Front left";
        case 1: return "Front right";
        case 2: return "Rear left";
        case 3: return "Rear right";
        default: return "Unknown";
    }
}

void BodyControlSWC::logCurrentStatus() {
    std::stringstream ss;
    
    // Count open doors
    int open_doors = 0;
    if (current_door_status_.front_left_open) open_doors++;
    if (current_door_status_.front_right_open) open_doors++;
    if (current_door_status_.rear_left_open) open_doors++;
    if (current_door_status_.rear_right_open) open_doors++;
    
    // Count active lights
    int active_lights = 0;
    if (current_light_control_.headlights_on) active_lights++;
    if (current_light_control_.turn_signal_left) active_lights++;
    if (current_light_control_.turn_signal_right) active_lights++;
    
    ss << "Body Status - Open doors: " << open_doors
       << ", Active lights: " << active_lights
       << ", Engine RPM: " << (engine_data_valid_ ? std::to_string(received_engine_data_.rpm) : "N/A")
       << " (Cycle: " << cycle_count_ << ")";
    
    log(ss.str());
}

} // namespace autosar