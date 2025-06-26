#include <gtest/gtest.h>
#include "rte.hpp"
#include "engine_swc.hpp"
#include "body_swc.hpp"
#include "can_interface.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace autosar;

class RTETestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        rte = std::make_unique<RTE>();
        can_interface = std::make_shared<CANInterface>();
        
        // Create test SWCs
        engine_swc = std::make_shared<EngineSWC>(can_interface);
        body_swc = std::make_shared<BodyControlSWC>(can_interface);
        
        // Initialize SWCs
        engine_swc->initialize();
        body_swc->initialize();
    }
    
    void TearDown() override {
        if (rte && rte->isRunning()) {
            rte->stop();
        }
        rte.reset();
        engine_swc.reset();
        body_swc.reset();
    }
    
    std::unique_ptr<RTE> rte;
    std::shared_ptr<CANInterface> can_interface;
    std::shared_ptr<EngineSWC> engine_swc;
    std::shared_ptr<BodyControlSWC> body_swc;
};

// Test RTE construction and basic state
TEST_F(RTETestFixture, BasicConstruction) {
    EXPECT_FALSE(rte->isRunning());
    EXPECT_EQ(rte->getSWCCount(), 0);
    EXPECT_EQ(rte->getConnectionCount(), 0);
}

// Test SWC registration
TEST_F(RTETestFixture, SWCRegistration) {
    // Test registering Engine SWC
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getSWCCount(), 1);
    
    // Test registering Body Control SWC
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getSWCCount(), 2);
    
    // Test registering null SWC
    EXPECT_EQ(rte->registerSWC(nullptr, 100), Std_ReturnType::E_NOT_OK);
    EXPECT_EQ(rte->getSWCCount(), 2);  // Should remain unchanged
}

// Test SWC unregistration
TEST_F(RTETestFixture, SWCUnregistration) {
    // Register SWCs first
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getSWCCount(), 2);
    
    // Unregister one SWC
    EXPECT_EQ(rte->unregisterSWC("EngineSWC"), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getSWCCount(), 1);
    
    // Unregister non-existent SWC
    EXPECT_EQ(rte->unregisterSWC("NonExistentSWC"), Std_ReturnType::E_NOT_OK);
    EXPECT_EQ(rte->getSWCCount(), 1);
    
    // Unregister remaining SWC
    EXPECT_EQ(rte->unregisterSWC("BodyControlSWC"), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getSWCCount(), 0);
}

// Test invalid cycle times with consteval validation
TEST_F(RTETestFixture, InvalidCycleTimes) {
    // Test compile-time validation
    static_assert(RTE::isValidCycleTime(100));   // Valid
    static_assert(!RTE::isValidCycleTime(0));    // Invalid - too small
    static_assert(!RTE::isValidCycleTime(15000)); // Invalid - too large
    
    // Valid cycle times should work
    EXPECT_EQ(rte->registerSWC(engine_swc, 100), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, 500), Std_ReturnType::E_OK);
}

// Test port connections
TEST_F(RTETestFixture, PortConnections) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Connect engine data port to body control
    EXPECT_EQ(rte->connectPorts("EngineSWC", "EngineDataPort", 
                               "BodyControlSWC", "EngineDataPort"), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(rte->getConnectionCount(), 1);
    
    // Test connecting non-existent ports
    EXPECT_EQ(rte->connectPorts("NonExistentSWC", "SomePort", 
                               "BodyControlSWC", "EngineDataPort"), 
              Std_ReturnType::E_NOT_OK);
    EXPECT_EQ(rte->getConnectionCount(), 1);  // Should remain unchanged
    
    // Test connecting non-existent destination
    EXPECT_EQ(rte->connectPorts("EngineSWC", "EngineDataPort", 
                               "NonExistentSWC", "SomePort"), 
              Std_ReturnType::E_NOT_OK);
    EXPECT_EQ(rte->getConnectionCount(), 1);
}

// Test RTE lifecycle (start/stop)
TEST_F(RTETestFixture, RTELifecycle) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Start RTE
    EXPECT_EQ(rte->start(), Std_ReturnType::E_OK);
    EXPECT_TRUE(rte->isRunning());
    
    // Let it run briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Stop RTE
    EXPECT_EQ(rte->stop(), Std_ReturnType::E_OK);
    EXPECT_FALSE(rte->isRunning());
    
    // Test starting already stopped RTE
    EXPECT_EQ(rte->start(), Std_ReturnType::E_OK);
    EXPECT_TRUE(rte->isRunning());
    
    // Test stopping again
    EXPECT_EQ(rte->stop(), Std_ReturnType::E_OK);
    EXPECT_FALSE(rte->isRunning());
}

// Test data exchange through RTE
TEST_F(RTETestFixture, DataExchange) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Test writing data through RTE
    EngineData test_data{.rpm = 2500, .temperature = 95, .throttle_position = 50};
    EXPECT_EQ(rte->writeData("EngineSWC", "EngineDataPort", test_data), 
              Std_ReturnType::E_OK);
    
    // Test reading data through RTE
    EngineData read_data;
    EXPECT_EQ(rte->readData("EngineSWC", "EngineDataPort", read_data), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(read_data.rpm, 2500);
    EXPECT_EQ(read_data.temperature, 95);
    EXPECT_EQ(read_data.throttle_position, 50);
    
    // Test reading from non-existent port
    EXPECT_EQ(rte->readData("NonExistentSWC", "SomePort", read_data), 
              Std_ReturnType::E_NOT_OK);
    
    // Test writing to non-existent port
    EXPECT_EQ(rte->writeData("NonExistentSWC", "SomePort", test_data), 
              Std_ReturnType::E_NOT_OK);
}

// Test data routing between connected ports
TEST_F(RTETestFixture, DataRouting) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Connect ports
    EXPECT_EQ(rte->connectPorts("EngineSWC", "EngineDataPort", 
                               "BodyControlSWC", "EngineDataPort"), 
              Std_ReturnType::E_OK);
    
    // Start RTE for data routing
    EXPECT_EQ(rte->start(), Std_ReturnType::E_OK);
    
    // Write data to engine SWC
    EngineData engine_data{.rpm = 3000, .temperature = 98, .throttle_position = 75};
    EXPECT_EQ(rte->writeData("EngineSWC", "EngineDataPort", engine_data), 
              Std_ReturnType::E_OK);
    
    // Allow some time for routing
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Data should be routed to body control SWC
    EngineData routed_data;
    EXPECT_EQ(rte->readData("BodyControlSWC", "EngineDataPort", routed_data), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(routed_data.rpm, 3000);
    EXPECT_EQ(routed_data.temperature, 98);
    EXPECT_EQ(routed_data.throttle_position, 75);
    
    rte->stop();
}

// Test C++20 concepts in data exchange
TEST_F(RTETestFixture, ConceptValidation) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Test with different AUTOSAR data types
    EngineData engine_data{.rpm = 1500, .temperature = 88, .throttle_position = 30};
    DoorStatus door_data{.front_left_open = true, .front_right_open = false, 
                        .rear_left_open = false, .rear_right_open = true};
    LightControl light_data{.headlights_on = true, .turn_signal_left = false, 
                           .turn_signal_right = true};
    
    // All should work with C++20 concepts
    EXPECT_EQ(rte->writeData("EngineSWC", "EngineDataPort", engine_data), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(rte->writeData("BodyControlSWC", "DoorStatusPort", door_data), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(rte->writeData("BodyControlSWC", "LightControlPort", light_data), 
              Std_ReturnType::E_OK);
    
    // Verify reads work
    EngineData read_engine;
    DoorStatus read_doors;
    LightControl read_lights;
    
    EXPECT_EQ(rte->readData("EngineSWC", "EngineDataPort", read_engine), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(rte->readData("BodyControlSWC", "DoorStatusPort", read_doors), 
              Std_ReturnType::E_OK);
    EXPECT_EQ(rte->readData("BodyControlSWC", "LightControlPort", read_lights), 
              Std_ReturnType::E_OK);
}

// Test scheduler functionality
TEST_F(RTETestFixture, SchedulerFunctionality) {
    // Register SWCs with different cycle times
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);  // 100ms
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);     // 500ms
    
    // Start RTE
    EXPECT_EQ(rte->start(), Std_ReturnType::E_OK);
    
    // Let it run for 1 second
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    
    // Stop RTE
    EXPECT_EQ(rte->stop(), Std_ReturnType::E_OK);
    
    // Engine should have run more frequently than body control
    uint32 engine_cycles = engine_swc->getCycleCount();
    uint32 body_cycles = body_swc->getCycleCount();
    
    EXPECT_GT(engine_cycles, 0);
    EXPECT_GT(body_cycles, 0);
    
    // Engine runs at 10Hz (100ms), Body at 2Hz (500ms)
    // So engine should run approximately 5 times more frequently
    EXPECT_GT(engine_cycles, body_cycles);
}

// Test error handling and fault tolerance
TEST_F(RTETestFixture, ErrorHandling) {
    // Test operations on unregistered SWCs
    EngineData data{.rpm = 1000, .temperature = 90, .throttle_position = 25};
    EXPECT_EQ(rte->writeData("UnregisteredSWC", "SomePort", data), 
              Std_ReturnType::E_NOT_OK);
    
    EXPECT_EQ(rte->readData("UnregisteredSWC", "SomePort", data), 
              Std_ReturnType::E_NOT_OK);
    
    // Register SWC and test operations on non-existent ports
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    EXPECT_EQ(rte->writeData("EngineSWC", "NonExistentPort", data), 
              Std_ReturnType::E_NOT_OK);
    
    EXPECT_EQ(rte->readData("EngineSWC", "NonExistentPort", data), 
              Std_ReturnType::E_NOT_OK);
}

// Test thread safety
TEST_F(RTETestFixture, ThreadSafety) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Start RTE
    EXPECT_EQ(rte->start(), Std_ReturnType::E_OK);
    
    std::atomic<bool> stop_test(false);
    std::atomic<int> write_count(0);
    std::atomic<int> read_count(0);
    
    // Writer thread
    std::thread writer([&]() {
        while (!stop_test) {
            EngineData data{
                .rpm = static_cast<uint16>(1000 + write_count),
                .temperature = static_cast<uint8>(85 + (write_count % 20)),
                .throttle_position = static_cast<uint8>(write_count % 101)
            };
            if (rte->writeData("EngineSWC", "EngineDataPort", data) == Std_ReturnType::E_OK) {
                write_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    });
    
    // Reader thread
    std::thread reader([&]() {
        while (!stop_test) {
            EngineData data;
            if (rte->readData("EngineSWC", "EngineDataPort", data) == Std_ReturnType::E_OK) {
                read_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(700));
        }
    });
    
    // Let threads run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop_test = true;
    
    writer.join();
    reader.join();
    
    rte->stop();
    
    EXPECT_GT(write_count.load(), 0);
    EXPECT_GT(read_count.load(), 0);
}

// Test RTE status and debugging functions
TEST_F(RTETestFixture, StatusAndDebugging) {
    // Register SWCs
    EXPECT_EQ(rte->registerSWC(engine_swc, ENGINE_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    EXPECT_EQ(rte->registerSWC(body_swc, BODY_CYCLE_TIME_MS), Std_ReturnType::E_OK);
    
    // Connect ports
    EXPECT_EQ(rte->connectPorts("EngineSWC", "EngineDataPort", 
                               "BodyControlSWC", "EngineDataPort"), 
              Std_ReturnType::E_OK);
    
    // Verify counts
    EXPECT_EQ(rte->getSWCCount(), 2);
    EXPECT_EQ(rte->getConnectionCount(), 1);
    
    // Test debugging functions (these should not crash)
    rte->printConnections();
    rte->printSWCStatus();
    
    // These are mainly for visual verification during development
    EXPECT_TRUE(true);  // If we get here without crashing, debugging functions work
}

// Test consteval compile-time validation
TEST_F(RTETestFixture, CompileTimeValidation) {
    // Test that consteval functions work at compile time
    static_assert(RTE::isValidCycleTime(100));
    static_assert(RTE::isValidCycleTime(1));
    static_assert(RTE::isValidCycleTime(10000));
    static_assert(!RTE::isValidCycleTime(0));
    static_assert(!RTE::isValidCycleTime(15000));
    
    // If compilation succeeds, consteval validation works
    EXPECT_TRUE(RTE::isValidCycleTime(ENGINE_CYCLE_TIME_MS));
    EXPECT_TRUE(RTE::isValidCycleTime(BODY_CYCLE_TIME_MS));
    EXPECT_TRUE(RTE::isValidCycleTime(RTE_CYCLE_TIME_MS));
}