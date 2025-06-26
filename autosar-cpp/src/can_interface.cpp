/**
 * @file can_interface.cpp
 * @brief Implementation of CAN communication interface
 */

#include "../include/can_interface.hpp"
#include <iostream>
#include <iomanip>

namespace autosar {

// VirtualCANBus implementation
Std_ReturnType VirtualCANBus::send(const CANMessage& message) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    // Prevent buffer overflow
    if (message_buffer_.size() >= MAX_BUFFER_SIZE) {
        message_buffer_.erase(message_buffer_.begin());
    }
    
    message_buffer_.push_back(message);
    
    // Debug output (can be disabled in production)
    std::cout << "[CAN] TX ID:0x" << std::hex << std::setw(3) << std::setfill('0') 
              << message.id << " Data:";
    for (int i = 0; i < message.length; ++i) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(message.data[i]) << " ";
    }
    std::cout << std::dec << std::endl;
    
    return Std_ReturnType::E_OK;
}

Std_ReturnType VirtualCANBus::receive(CANMessage& message) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    if (message_buffer_.empty()) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    message = message_buffer_.front();
    message_buffer_.erase(message_buffer_.begin());
    
    return Std_ReturnType::E_OK;
}

bool VirtualCANBus::hasMessages() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return !message_buffer_.empty();
}

size_t VirtualCANBus::getMessageCount() const {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    return message_buffer_.size();
}

void VirtualCANBus::clear() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    message_buffer_.clear();
}

// CANInterface implementation
CANInterface::CANInterface(std::shared_ptr<VirtualCANBus> can_bus) 
    : can_bus_(can_bus) {
    if (!can_bus_) {
        throw std::invalid_argument("CAN bus cannot be null");
    }
}

Std_ReturnType CANInterface::sendEngineData(const EngineData& data) {
    CANMessage message = encodeEngineData(data);
    return can_bus_->send(message);
}

Std_ReturnType CANInterface::sendDoorStatus(const DoorStatus& status) {
    CANMessage message = encodeDoorStatus(status);
    return can_bus_->send(message);
}

Std_ReturnType CANInterface::sendLightStatus(const LightControl& lights) {
    CANMessage message = encodeLightStatus(lights);
    return can_bus_->send(message);
}

Std_ReturnType CANInterface::receiveEngineData(EngineData& data) {
    CANMessage message;
    
    // Look for engine data messages in the bus
    while (can_bus_->hasMessages()) {
        if (can_bus_->receive(message) == Std_ReturnType::E_OK) {
            if (message.id == can_ids::ENGINE_RPM) {
                return decodeEngineData(message, data);
            }
        }
    }
    
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType CANInterface::receiveDoorStatus(DoorStatus& status) {
    CANMessage message;
    
    while (can_bus_->hasMessages()) {
        if (can_bus_->receive(message) == Std_ReturnType::E_OK) {
            if (message.id == can_ids::DOOR_STATUS) {
                return decodeDoorStatus(message, status);
            }
        }
    }
    
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType CANInterface::receiveLightStatus(LightControl& lights) {
    CANMessage message;
    
    while (can_bus_->hasMessages()) {
        if (can_bus_->receive(message) == Std_ReturnType::E_OK) {
            if (message.id == can_ids::LIGHT_STATUS) {
                return decodeLightStatus(message, lights);
            }
        }
    }
    
    return Std_ReturnType::E_NOT_OK;
}

Std_ReturnType CANInterface::sendMessage(const CANMessage& message) {
    return can_bus_->send(message);
}

Std_ReturnType CANInterface::receiveMessage(CANMessage& message) {
    return can_bus_->receive(message);
}

// Message encoding implementations
CANMessage CANInterface::encodeEngineData(const EngineData& data) {
    CANMessage message;
    message.id = can_ids::ENGINE_RPM;
    message.length = 4;
    
    // Encode RPM as 16-bit big-endian
    message.data[0] = static_cast<uint8>((data.rpm >> 8) & 0xFF);
    message.data[1] = static_cast<uint8>(data.rpm & 0xFF);
    
    // Temperature and throttle as single bytes
    message.data[2] = data.temperature;
    message.data[3] = data.throttle_position;
    
    return message;
}

CANMessage CANInterface::encodeDoorStatus(const DoorStatus& status) {
    CANMessage message;
    message.id = can_ids::DOOR_STATUS;
    message.length = 1;
    
    uint8 door_bits = 0;
    if (status.front_left_open)  door_bits |= 0x01;
    if (status.front_right_open) door_bits |= 0x02;
    if (status.rear_left_open)   door_bits |= 0x04;
    if (status.rear_right_open)  door_bits |= 0x08;
    
    message.data[0] = door_bits;
    
    return message;
}

CANMessage CANInterface::encodeLightStatus(const LightControl& lights) {
    CANMessage message;
    message.id = can_ids::LIGHT_STATUS;
    message.length = 1;
    
    uint8 light_bits = 0;
    if (lights.headlights_on)      light_bits |= 0x01;
    if (lights.turn_signal_left)   light_bits |= 0x02;
    if (lights.turn_signal_right)  light_bits |= 0x04;
    
    message.data[0] = light_bits;
    
    return message;
}

// Message decoding implementations
Std_ReturnType CANInterface::decodeEngineData(const CANMessage& message, EngineData& data) {
    if (message.length < 4) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    // Decode 16-bit big-endian RPM
    data.rpm = (static_cast<uint16>(message.data[0]) << 8) | message.data[1];
    data.temperature = message.data[2];
    data.throttle_position = message.data[3];
    
    return Std_ReturnType::E_OK;
}

Std_ReturnType CANInterface::decodeDoorStatus(const CANMessage& message, DoorStatus& status) {
    if (message.length < 1) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    uint8 door_bits = message.data[0];
    status.front_left_open  = (door_bits & 0x01) != 0;
    status.front_right_open = (door_bits & 0x02) != 0;
    status.rear_left_open   = (door_bits & 0x04) != 0;
    status.rear_right_open  = (door_bits & 0x08) != 0;
    
    return Std_ReturnType::E_OK;
}

Std_ReturnType CANInterface::decodeLightStatus(const CANMessage& message, LightControl& lights) {
    if (message.length < 1) {
        return Std_ReturnType::E_NOT_OK;
    }
    
    uint8 light_bits = message.data[0];
    lights.headlights_on      = (light_bits & 0x01) != 0;
    lights.turn_signal_left   = (light_bits & 0x02) != 0;
    lights.turn_signal_right  = (light_bits & 0x04) != 0;
    
    return Std_ReturnType::E_OK;
}

} // namespace autosar