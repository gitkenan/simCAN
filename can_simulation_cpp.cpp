#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <vector>
#include <string>
#include <iomanip>
#include <sstream>
#include <cstring>

// Simplified CAN message structure
struct CANMessage {
    uint32_t id;
    uint8_t data[8];
    uint8_t length;
    
    CANMessage(uint32_t msg_id, const std::vector<uint8_t>& msg_data) 
        : id(msg_id), length(std::min((size_t)8, msg_data.size())) {
        std::memset(data, 0, 8);
        std::copy(msg_data.begin(), msg_data.begin() + length, data);
    }
};

// Virtual CAN Bus implementation
class VirtualCANBus {
private:
    std::vector<CANMessage> message_buffer;
    mutable std::mutex buffer_mutex;
    
public:
    void send(const CANMessage& msg) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        message_buffer.push_back(msg);
    }
    
    bool receive(CANMessage& msg) {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        if (!message_buffer.empty()) {
            msg = message_buffer.front();
            message_buffer.erase(message_buffer.begin());
            return true;
        }
        return false;
    }
};

// Engine ECU simulation
class EngineECU {
private:
    VirtualCANBus* bus;
    uint16_t engine_rpm;
    uint8_t throttle_pos;
    bool running;
    std::mt19937 rng;
    
public:
    EngineECU(VirtualCANBus* can_bus) 
        : bus(can_bus), engine_rpm(800), throttle_pos(0), running(true), rng(std::random_device{}()) {}
    
    void simulate() {
        std::cout << "[ENGINE ECU] Starting engine control unit simulation" << std::endl;
        
        while (running) {
            try {
                // Update engine RPM
                std::uniform_int_distribution<int> rpm_change(-50, 100);
                engine_rpm = std::max(800, std::min(6000, engine_rpm + rpm_change(rng)));
                
                // Create RPM message (ID: 0x110)
                std::vector<uint8_t> rpm_data = {
                    static_cast<uint8_t>(engine_rpm >> 8),  // High byte
                    static_cast<uint8_t>(engine_rpm & 0xFF), // Low byte
                    throttle_pos,
                    0
                };
                
                CANMessage rpm_msg(0x110, rpm_data);
                bus->send(rpm_msg);
                
                // Create vehicle speed message (ID: 0x120)
                uint16_t speed = std::max(0, (engine_rpm - 800) / 50);
                std::vector<uint8_t> speed_data = {
                    static_cast<uint8_t>(speed >> 8),
                    static_cast<uint8_t>(speed & 0xFF),
                    0, 0
                };
                
                CANMessage speed_msg(0x120, speed_data);
                bus->send(speed_msg);
                
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
                
                std::cout << "[ENGINE ECU] " << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                         << "." << std::setfill('0') << std::setw(3) << ms.count()
                         << " - Engine RPM: " << engine_rpm 
                         << ", Speed: " << speed << " km/h" << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << "[ENGINE ECU] Error: " << e.what() << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz
        }
    }
    
    void stop() { running = false; }
};

// Body Control Module simulation
class BodyControlModule {
private:
    VirtualCANBus* bus;
    uint8_t door_status;
    bool running;
    std::mt19937 rng;
    
public:
    BodyControlModule(VirtualCANBus* can_bus) 
        : bus(can_bus), door_status(0), running(true), rng(std::random_device{}()) {}
    
    void simulate() {
        std::cout << "[BCM] Starting Body Control Module simulation" << std::endl;
        
        while (running) {
            try {
                // Randomly change door status (5% chance)
                std::uniform_real_distribution<float> chance(0.0, 1.0);
                if (chance(rng) < 0.05) {
                    std::uniform_int_distribution<int> door_select(0, 3);
                    int door = door_select(rng);
                    door_status ^= (1 << door); // Toggle door bit
                }
                
                // Create door status message (ID: 0x200)
                std::vector<uint8_t> door_data = {door_status, 0, 0, 0, 0, 0, 0, 0};
                CANMessage door_msg(0x200, door_data);
                bus->send(door_msg);
                
                if (door_status != 0) {
                    std::vector<std::string> door_names = {"FL", "FR", "RL", "RR"};
                    std::string open_doors;
                    for (int i = 0; i < 4; ++i) {
                        if (door_status & (1 << i)) {
                            if (!open_doors.empty()) open_doors += ", ";
                            open_doors += door_names[i];
                        }
                    }
                    
                    auto now = std::chrono::system_clock::now();
                    auto time_t = std::chrono::system_clock::to_time_t(now);
                    
                    std::cout << "[BCM] " << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                             << " - Doors open: " << open_doors << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cout << "[BCM] Error: " << e.what() << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 2Hz
        }
    }
    
    void stop() { running = false; }
};

// CAN Message Logger
class CANLogger {
private:
    VirtualCANBus* bus;
    bool running;
    
    std::string decode_message(const CANMessage& msg) {
        std::stringstream ss;
        
        switch (msg.id) {
            case 0x110: // Engine RPM
                if (msg.length >= 3) {
                    uint16_t rpm = (msg.data[0] << 8) | msg.data[1];
                    uint8_t throttle = msg.data[2];
                    ss << "Engine RPM: " << rpm << ", Throttle: " << (int)throttle << "%";
                }
                break;
                
            case 0x120: // Vehicle Speed
                if (msg.length >= 2) {
                    uint16_t speed = (msg.data[0] << 8) | msg.data[1];
                    ss << "Vehicle Speed: " << speed << " km/h";
                }
                break;
                
            case 0x200: // Door Status
                if (msg.length >= 1) {
                    uint8_t doors = msg.data[0];
                    std::vector<std::string> door_names = {"FL", "FR", "RL", "RR"};
                    std::string open_doors;
                    
                    for (int i = 0; i < 4; ++i) {
                        if (doors & (1 << i)) {
                            if (!open_doors.empty()) open_doors += ", ";
                            open_doors += door_names[i];
                        }
                    }
                    
                    ss << "Doors Open: " << (open_doors.empty() ? "None" : open_doors);
                }
                break;
                
            default:
                ss << "Unknown message, Raw Data: [";
                for (int i = 0; i < msg.length; ++i) {
                    if (i > 0) ss << ", ";
                    ss << (int)msg.data[i];
                }
                ss << "]";
                break;
        }
        
        return ss.str();
    }
    
    std::string get_priority_level(uint32_t msg_id) {
        if (msg_id >= 0x100 && msg_id <= 0x1FF) return "CRITICAL";
        if (msg_id >= 0x200 && msg_id <= 0x2FF) return "BODY";
        if (msg_id >= 0x300 && msg_id <= 0x3FF) return "COMFORT";
        return "UNKNOWN";
    }
    
public:
    CANLogger(VirtualCANBus* can_bus) : bus(can_bus), running(true) {}
    
    void log_messages() {
        std::cout << "[LOGGER] Starting automotive CAN message logger..." << std::endl;
        
        while (running) {
            CANMessage msg(0, {});
            
            if (bus->receive(msg)) {
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;
                
                std::string decoded = decode_message(msg);
                std::string priority = get_priority_level(msg.id);
                
                std::cout << "[LOGGER] " << std::put_time(std::localtime(&time_t), "%H:%M:%S")
                         << "." << std::setfill('0') << std::setw(3) << ms.count()
                         << " - ID:0x" << std::hex << std::setw(3) << std::setfill('0') 
                         << msg.id << std::dec << " [" << priority << "] - " 
                         << decoded << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    void stop() { running = false; }
};

int main() {
    std::cout << "================================================================================\n";
    std::cout << "Automotive CAN Bus Simulation (C++ Version)\n";
    std::cout << "Simulating Engine ECU and Body Control Module\n";
    std::cout << "================================================================================\n";
    
    try {
        // Create virtual CAN bus
        VirtualCANBus bus;
        std::cout << "[MAIN] Virtual CAN bus created" << std::endl;
        
        // Create ECU instances
        EngineECU engine_ecu(&bus);
        BodyControlModule bcm(&bus);
        CANLogger logger(&bus);
        
        // Create and start threads
        std::thread engine_thread(&EngineECU::simulate, &engine_ecu);
        std::thread bcm_thread(&BodyControlModule::simulate, &bcm);
        std::thread logger_thread(&CANLogger::log_messages, &logger);
        
        std::cout << "[MAIN] All ECUs and logger started" << std::endl;
        std::cout << "[MAIN] Press Ctrl+C to stop the simulation" << std::endl;
        std::cout << "--------------------------------------------------------------------------------\n";
        
        // Keep main thread alive (in real implementation, would handle signal)
        std::this_thread::sleep_for(std::chrono::seconds(30)); // Run for 30 seconds
        
        std::cout << "\n[MAIN] Stopping simulation..." << std::endl;
        
        // Stop all ECUs
        engine_ecu.stop();
        bcm.stop();
        logger.stop();
        
        // Wait for threads to finish
        engine_thread.join();
        bcm_thread.join();
        logger_thread.join();
        
        std::cout << "[MAIN] Simulation stopped" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "[MAIN] Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}