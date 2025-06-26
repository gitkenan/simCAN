/**
 * @file can_interface.hpp
 * @brief CAN communication interface for AUTOSAR SWCs
 * 
 * Provides a simple CAN bus interface that integrates with AUTOSAR SWCs.
 * In a real system, this would interface with AUTOSAR BSW CAN modules.
 */

#ifndef CAN_INTERFACE_HPP
#define CAN_INTERFACE_HPP

#include "autosar_types.hpp"
#include <vector>
#include <mutex>
#include <cstring>
#include <memory>

namespace autosar {

/**
 * @brief CAN message structure
 */
struct CANMessage {
    uint32 id;
    uint8 data[8];
    uint8 length;
    
    CANMessage() : id(0), length(0) {
        std::memset(data, 0, sizeof(data));
    }
    
    CANMessage(uint32 msg_id, const std::vector<uint8>& msg_data) 
        : id(msg_id), length(std::min(static_cast<size_t>(8), msg_data.size())) {
        std::memset(data, 0, sizeof(data));
        std::copy(msg_data.begin(), msg_data.begin() + length, data);
    }
    
    CANMessage(uint32 msg_id, const uint8* msg_data, uint8 msg_length)
        : id(msg_id), length(std::min(static_cast<uint8>(8), msg_length)) {
        std::memset(data, 0, sizeof(data));
        if (msg_data) {
            std::memcpy(data, msg_data, length);
        }
    }
};

/**
 * @brief Virtual CAN Bus for simulation
 * 
 * Simulates a CAN bus network for educational purposes.
 * In production AUTOSAR, this would be replaced by actual CAN drivers.
 */
class VirtualCANBus {
public:
    VirtualCANBus() = default;
    ~VirtualCANBus() = default;
    
    // Send a CAN message
    Std_ReturnType send(const CANMessage& message);
    
    // Receive a CAN message (non-blocking)
    Std_ReturnType receive(CANMessage& message);
    
    // Check if messages are available
    bool hasMessages() const;
    
    // Get number of messages in buffer
    size_t getMessageCount() const;
    
    // Clear all messages (for testing)
    void clear();
    
private:
    mutable std::mutex buffer_mutex_;
    std::vector<CANMessage> message_buffer_;
    
    // Maximum buffer size to prevent memory issues
    static constexpr size_t MAX_BUFFER_SIZE = 1000;
};

/**
 * @brief CAN interface for SWCs
 * 
 * Provides high-level CAN communication functions for SWCs.
 * Handles message encoding/decoding and bus access.
 */
class CANInterface {
public:
    explicit CANInterface(std::shared_ptr<VirtualCANBus> can_bus);
    ~CANInterface() = default;
    
    // High-level send functions
    Std_ReturnType sendEngineData(const EngineData& data);
    Std_ReturnType sendDoorStatus(const DoorStatus& status);
    Std_ReturnType sendLightStatus(const LightControl& lights);
    
    // High-level receive functions  
    Std_ReturnType receiveEngineData(EngineData& data);
    Std_ReturnType receiveDoorStatus(DoorStatus& status);
    Std_ReturnType receiveLightStatus(LightControl& lights);
    
    // Generic send/receive
    Std_ReturnType sendMessage(const CANMessage& message);
    Std_ReturnType receiveMessage(CANMessage& message);
    
private:
    std::shared_ptr<VirtualCANBus> can_bus_;
    
    // Message encoding helpers
    CANMessage encodeEngineData(const EngineData& data);
    CANMessage encodeDoorStatus(const DoorStatus& status);
    CANMessage encodeLightStatus(const LightControl& lights);
    
    // Message decoding helpers
    Std_ReturnType decodeEngineData(const CANMessage& message, EngineData& data);
    Std_ReturnType decodeDoorStatus(const CANMessage& message, DoorStatus& status);
    Std_ReturnType decodeLightStatus(const CANMessage& message, LightControl& lights);
};

} // namespace autosar

#endif // CAN_INTERFACE_HPP