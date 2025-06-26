/**
 * @file main_hmi_demo.cpp
 * @brief AUTOSAR C++ with WebSocket HMI Demo Application
 * 
 * This application demonstrates a complete AUTOSAR system with WebSocket bridge
 * enabling real-time communication with web-based HMI frontends:
 * - Engine and Body Control Software Components
 * - Runtime Environment managing SWC communication
 * - CAN bus simulation
 * - WebSocket bridge for HMI integration
 * - Real-time bidirectional data exchange
 * 
 * Educational focus: Full automotive software stack with modern HMI integration
 */

#include "autosar_types.hpp"
#include "can_interface.hpp"
#include "engine_swc.hpp"
#include "body_swc.hpp"
#include "rte.hpp"
#include "simple_http_server.hpp"

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

void printSystemStatus(const std::shared_ptr<EngineSWC>& engine_swc,
                      const std::shared_ptr<BodyControlSWC>& body_swc,
                      const std::shared_ptr<VirtualCANBus>& can_bus,
                      const std::shared_ptr<SimpleHTTPServer>& http_server) {
    auto engine_data = engine_swc->getCurrentEngineData();
    auto door_status = body_swc->getCurrentDoorStatus();
    auto light_status = body_swc->getCurrentLightControl();
    
    std::cout << "\n=== Current System Status ===" << std::endl;
    std::cout << "Engine: RPM=" << engine_data.rpm 
              << ", Temp=" << static_cast<int>(engine_data.temperature) << "°C"
              << ", Throttle=" << static_cast<int>(engine_data.throttle_position) << "%" << std::endl;
    
    int open_doors = (door_status.front_left_open ? 1 : 0) +
                    (door_status.front_right_open ? 1 : 0) +
                    (door_status.rear_left_open ? 1 : 0) +
                    (door_status.rear_right_open ? 1 : 0);
    
    std::cout << "Body: " << open_doors << " doors open"
              << ", Headlights=" << (light_status.headlights_on ? "ON" : "OFF")
              << ", Turn signals=" << (light_status.turn_signal_left || light_status.turn_signal_right ? "ACTIVE" : "OFF") << std::endl;
    
    std::cout << "CAN: " << can_bus->getMessageCount() << " messages in buffer" << std::endl;
    std::cout << "HTTP Server: " << (http_server->isRunning() ? "Running" : "Stopped") << std::endl;
    std::cout << "==============================\n" << std::endl;
}

int main() {
    std::cout << "=== AUTOSAR C++ with HTTP HMI Demo ===" << std::endl;
    std::cout << "Demonstrating automotive software architecture with:" << std::endl;
    std::cout << "- Engine Software Component (10Hz)" << std::endl;
    std::cout << "- Body Control Software Component (2Hz)" << std::endl;
    std::cout << "- Runtime Environment managing communication" << std::endl;
    std::cout << "- CAN bus simulation" << std::endl;
    std::cout << "- HTTP API server for HMI integration" << std::endl;
    std::cout << "\nHTTP server will start on port 8080" << std::endl;
    std::cout << "Frontend can access: http://localhost:8080/api/vehicle" << std::endl;
    std::cout << "\nPress Ctrl+C to stop...\n" << std::endl;
    
    // Set up signal handler for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    
    try {
        // 1. Create CAN bus infrastructure
        auto can_bus = std::make_shared<VirtualCANBus>();
        auto can_interface = std::make_shared<CANInterface>(can_bus);
        
        std::cout << "[HMI_DEMO] Created CAN bus infrastructure" << std::endl;
        
        // 2. Create Software Components
        auto engine_swc = std::make_shared<EngineSWC>(can_interface);
        auto body_swc = std::make_shared<BodyControlSWC>(can_interface);
        
        std::cout << "[HMI_DEMO] Created Software Components" << std::endl;
        
        // 3. Create and configure Runtime Environment
        auto rte = std::make_shared<RTE>();
        
        // Register SWCs with their timing requirements
        if (rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS) != Std_ReturnType::E_OK) {
            std::cerr << "Failed to register Engine SWC" << std::endl;
            return 1;
        }
        
        if (rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS) != Std_ReturnType::E_OK) {
            std::cerr << "Failed to register Body Control SWC" << std::endl;
            return 1;
        }
        
        std::cout << "[HMI_DEMO] Registered SWCs with RTE" << std::endl;
        
        // 4. Create and initialize HTTP server
        auto http_server = std::make_shared<SimpleHTTPServer>(8080);
        
        if (http_server->initialize(engine_swc, body_swc, rte) != Std_ReturnType::E_OK) {
            std::cerr << "Failed to initialize HTTP server" << std::endl;
            return 1;
        }
        
        std::cout << "[HMI_DEMO] Initialized HTTP server" << std::endl;
        
        // 5. Start the AUTOSAR system
        if (rte->start() != Std_ReturnType::E_OK) {
            std::cerr << "Failed to start RTE" << std::endl;
            return 1;
        }
        
        std::cout << "[HMI_DEMO] AUTOSAR system started successfully!" << std::endl;
        
        // 6. Connect ports for inter-SWC communication
        if (rte->connectPorts("EngineSWC", "EngineDataPort", 
                             "BodyControlSWC", "EngineDataPort") != Std_ReturnType::E_OK) {
            std::cerr << "Failed to connect Engine->Body ports" << std::endl;
            return 1;
        }
        
        std::cout << "[HMI_DEMO] Connected SWC ports" << std::endl;
        
        // 7. Start HTTP server
        if (http_server->start() != Std_ReturnType::E_OK) {
            std::cerr << "Failed to start HTTP server" << std::endl;
            return 1;
        }
        
        std::cout << "[HMI_DEMO] HTTP server started on port 8080" << std::endl;
        std::cout << "[HMI_DEMO] API available at: http://localhost:8080/api/" << std::endl;
        
        // 8. Print initial system configuration
        rte->printConnections();
        rte->printSWCStatus();
        
        std::cout << "\nMonitoring system activity and HMI connections..." << std::endl;
        std::cout << "Open your web browser to see the vehicle dashboard!" << std::endl;
        std::cout << "(Press Ctrl+C to stop)\n" << std::endl;
        
        // 9. Run system with enhanced monitoring
        auto start_time = std::chrono::steady_clock::now();
        int status_cycle = 0;
        int demo_cycle = 0;
        
        while (running) {
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(
                current_time - start_time
            ).count();
            
            // Print system status every 15 seconds
            if (elapsed_seconds > 0 && elapsed_seconds % 15 == 0 && status_cycle != elapsed_seconds / 15) {
                status_cycle = elapsed_seconds / 15;
                printSystemStatus(engine_swc, body_swc, can_bus, http_server);
                
                // Print HTTP server statistics
                auto stats = http_server->getServerStats();
                std::cout << "HTTP Server Stats: " << stats.dump(2) << std::endl;
            }
            
            // Demonstrate external interactions every 20 seconds
            if (elapsed_seconds > 0 && elapsed_seconds % 20 == 0 && demo_cycle != elapsed_seconds / 20) {
                demo_cycle = elapsed_seconds / 20;
                
                std::cout << "\n[HMI_DEMO] Demonstrating automated interactions..." << std::endl;
                
                // Simulate door operations
                if (demo_cycle % 2 == 1) {
                    body_swc->setDoorOpen(0, true);  // Open front left door
                    std::cout << "[HMI_DEMO] Opened front left door via system command" << std::endl;
                } else {
                    body_swc->setDoorOpen(0, false); // Close front left door
                    std::cout << "[HMI_DEMO] Closed front left door via system command" << std::endl;
                }
                
                // Simulate light control
                LightControl lights{
                    .headlights_on = (demo_cycle % 3 == 0),
                    .turn_signal_left = (demo_cycle % 4 == 0),
                    .turn_signal_right = false
                };
                body_swc->setLightControl(lights);
                std::cout << "[HMI_DEMO] Updated light control via system command" << std::endl;
                
                std::cout << "[HMI_DEMO] Changes will be reflected in connected HMI clients" << std::endl;
            }
            
            // Sleep for a short time to avoid busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 10. Graceful shutdown
        std::cout << "\n[HMI_DEMO] Stopping system components..." << std::endl;
        
        // Stop HTTP server first
        if (http_server->stop() != Std_ReturnType::E_OK) {
            std::cerr << "Warning: HTTP server stop returned error" << std::endl;
        }
        
        // Stop RTE
        if (rte->stop() != Std_ReturnType::E_OK) {
            std::cerr << "Warning: RTE stop returned error" << std::endl;
        }
        
        // Print final statistics
        std::cout << "\n[HMI_DEMO] Final System Statistics:" << std::endl;
        rte->printSWCStatus();
        
        auto final_stats = http_server->getServerStats();
        std::cout << "Final HTTP Server Stats: " << final_stats.dump(2) << std::endl;
        
        std::cout << "Final CAN buffer size: " << can_bus->getMessageCount() << " messages" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception in main: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\n=== AUTOSAR HMI Demo Complete ===" << std::endl;
    std::cout << "This demonstration showed:" << std::endl;
    std::cout << "✓ AUTOSAR Software Components with proper lifecycle management" << std::endl;
    std::cout << "✓ Runtime Environment handling inter-component communication" << std::endl;
    std::cout << "✓ Port-based data exchange with type safety" << std::endl;
    std::cout << "✓ CAN bus simulation with realistic automotive protocols" << std::endl;
    std::cout << "✓ HTTP REST API for HMI integration" << std::endl;
    std::cout << "✓ JSON-based communication between automotive systems and web UI" << std::endl;
    std::cout << "✓ Professional C++ patterns for automotive development" << std::endl;
    
    return 0;
}