/**
 * @file main_demo.cpp
 * @brief AUTOSAR C++ Learning Demo Application
 * 
 * This application demonstrates a complete AUTOSAR system with:
 * - Engine and Body Control Software Components
 * - Runtime Environment managing SWC communication
 * - CAN bus simulation
 * - Inter-component data flow
 * 
 * Educational focus: See AUTOSAR concepts working together in practice
 */

#include "autosar_types.hpp"
#include "can_interface.hpp"
#include "engine_swc.hpp"
#include "body_swc.hpp"
#include "rte.hpp"

#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <csignal>
#include <atomic>

using namespace autosar;

// Global flag for graceful shutdown
std::atomic<bool> running(true);

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down gracefully..." << std::endl;
    running = false;
}

int main() {
    std::cout << "=== AUTOSAR C++ Learning Demo ===" << std::endl;
    std::cout << "Demonstrating automotive software architecture with:" << std::endl;
    std::cout << "- Engine Software Component (10Hz)" << std::endl;
    std::cout << "- Body Control Software Component (2Hz)" << std::endl;
    std::cout << "- Runtime Environment managing communication" << std::endl;
    std::cout << "- CAN bus simulation" << std::endl;
    std::cout << "\nPress Ctrl+C to stop...\n" << std::endl;
    
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // 1. Create CAN bus infrastructure
        auto can_bus = std::make_shared<VirtualCANBus>();
        auto can_interface = std::make_shared<CANInterface>(can_bus);
        
        std::cout << "[DEMO] Created CAN bus infrastructure" << std::endl;
        
        // 2. Create Software Components
        auto engine_swc = std::make_shared<EngineSWC>(can_interface);
        auto body_swc = std::make_shared<BodyControlSWC>(can_interface);
        
        std::cout << "[DEMO] Created Software Components" << std::endl;
        
        // 3. Create and configure Runtime Environment
        RTE rte;
        
        // Register SWCs with their timing requirements
        if (rte.registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS) != Std_ReturnType::E_OK) {
            std::cerr << "Failed to register Engine SWC" << std::endl;
            return 1;
        }
        
        if (rte.registerSWC(body_swc, BODY_CYCLE_TIME_MS) != Std_ReturnType::E_OK) {
            std::cerr << "Failed to register Body Control SWC" << std::endl;
            return 1;
        }
        
        std::cout << "[DEMO] Registered SWCs with RTE" << std::endl;
        
        // 4. Print system configuration
        rte.printSWCStatus();
        
        // 5. Start the AUTOSAR system (this will initialize SWCs and create ports)
        if (rte.start() != Std_ReturnType::E_OK) {
            std::cerr << "Failed to start RTE" << std::endl;
            return 1;
        }
        
        std::cout << "\n[DEMO] AUTOSAR system started successfully!" << std::endl;
        
        // 6. Connect ports for inter-SWC communication (now that SWCs are initialized)
        // Engine SWC sends engine data to Body Control SWC for auto features
        if (rte.connectPorts("EngineSWC", "EngineDataPort", 
                            "BodyControlSWC", "EngineDataPort") != Std_ReturnType::E_OK) {
            std::cerr << "Failed to connect Engine->Body ports" << std::endl;
            return 1;
        }
        
        std::cout << "[DEMO] Connected SWC ports" << std::endl;
        
        // 7. Print final system configuration
        rte.printConnections();
        rte.printSWCStatus();
        
        std::cout << "Monitoring system activity (press Ctrl+C to stop)...\n" << std::endl;
        
        // 8. Run system and demonstrate features
        auto start_time = std::chrono::steady_clock::now();
        int demo_cycle = 0;
        
        while (running) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time
            ).count();
            
            // Demonstrate external interactions every 10 seconds
            if (elapsed_seconds > 0 && elapsed_seconds % 10 == 0 && demo_cycle != elapsed_seconds / 10) {
                demo_cycle = elapsed_seconds / 10;
                
                std::cout << "\n[DEMO] Demonstrating external interactions..." << std::endl;
                
                // Simulate door operations
                if (demo_cycle % 2 == 1) {
                    body_swc->setDoorOpen(0, true);  // Open front left door
                    std::cout << "[DEMO] Opened front left door via external command" << std::endl;
                } else {
                    body_swc->setDoorOpen(0, false); // Close front left door
                    std::cout << "[DEMO] Closed front left door via external command" << std::endl;
                }
                
                // Simulate light control using C++20 designated initializers
                LightControl lights{
                    .headlights_on = (demo_cycle % 3 == 0),
                    .turn_signal_left = (demo_cycle % 4 == 0),
                    .turn_signal_right = false
                };
                body_swc->setLightControl(lights);
                std::cout << "[DEMO] Updated light control via external command" << std::endl;
                
                // Print current system status
                std::cout << "\n[DEMO] Current System Status:" << std::endl;
                auto engine_data = engine_swc->getCurrentEngineData();
                auto door_status = body_swc->getCurrentDoorStatus();
                auto light_status = body_swc->getCurrentLightControl();
                
                std::cout << "  Engine: RPM=" << engine_data.rpm 
                         << ", Temp=" << static_cast<int>(engine_data.temperature) << "°C"
                         << ", Throttle=" << static_cast<int>(engine_data.throttle_position) << "%" << std::endl;
                
                int open_doors = (door_status.front_left_open ? 1 : 0) +
                               (door_status.front_right_open ? 1 : 0) +
                               (door_status.rear_left_open ? 1 : 0) +
                               (door_status.rear_right_open ? 1 : 0);
                
                std::cout << "  Body: " << open_doors << " doors open"
                         << ", Headlights=" << (light_status.headlights_on ? "ON" : "OFF")
                         << ", Turn signals=" << (light_status.turn_signal_left || light_status.turn_signal_right ? "ACTIVE" : "OFF") << std::endl;
                
                std::cout << "  CAN: " << can_bus->getMessageCount() << " messages in buffer" << std::endl;
                std::cout << std::endl;
            }
            
            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 9. Graceful shutdown
        std::cout << "\n[DEMO] Stopping AUTOSAR system..." << std::endl;
        
        if (rte.stop() != Std_ReturnType::E_OK) {
            std::cerr << "Warning: RTE stop returned error" << std::endl;
        }
        
        // Print final statistics
        rte.printSWCStatus();
        
        std::cout << "\n[DEMO] System stopped successfully" << std::endl;
        std::cout << "Final CAN buffer size: " << can_bus->getMessageCount() << " messages" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== AUTOSAR Demo Complete ===" << std::endl;
    std::cout << "This demonstration showed:" << std::endl;
    std::cout << "✓ AUTOSAR Software Components with proper lifecycle management" << std::endl;
    std::cout << "✓ Runtime Environment handling inter-component communication" << std::endl;
    std::cout << "✓ Port-based data exchange with type safety" << std::endl;
    std::cout << "✓ CAN bus simulation with realistic automotive protocols" << std::endl;
    std::cout << "✓ Realistic timing constraints and scheduling" << std::endl;
    std::cout << "✓ Professional C++ patterns for automotive development" << std::endl;
    
    return 0;
}