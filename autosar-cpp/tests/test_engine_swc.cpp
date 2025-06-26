#include <gtest/gtest.h>
#include "engine_swc.hpp"
#include "can_interface.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace autosar;

class EngineTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        can_interface = std::make_shared<CANInterface>();
        engine_swc = std::make_unique<EngineSWC>(can_interface);
    }
    
    void TearDown() override {
        if (engine_swc) {
            engine_swc->stop();
        }
        engine_swc.reset();
    }
    
    std::shared_ptr<CANInterface> can_interface;
    std::unique_ptr<EngineSWC> engine_swc;
};

// Test EngineSWC construction and basic properties
TEST_F(EngineTestFixture, BasicConstruction) {
    EXPECT_EQ(engine_swc->getName(), "EngineSWC");
    EXPECT_EQ(engine_swc->getState(), SWCState::STOPPED);
    EXPECT_EQ(engine_swc->getCycleCount(), 0);
    
    // Test that CAN interface is properly stored
    EXPECT_NE(can_interface, nullptr);
}

// Test initialization
TEST_F(EngineTestFixture, Initialization) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Verify ports were created during initialization
    auto engine_port = engine_swc->getPort("EngineDataPort");
    EXPECT_NE(engine_port, nullptr);
    EXPECT_EQ(engine_port->getName(), "EngineDataPort");
    EXPECT_EQ(engine_port->getDirection(), PortDirection::SENDER);
    
    // Check that we have the correct number of ports
    const auto& ports = engine_swc->getPorts();
    EXPECT_EQ(ports.size(), 1);  // Only EngineDataPort
}

// Test null CAN interface handling
TEST_F(EngineTestFixture, NullCANInterface) {
    EXPECT_THROW({
        auto bad_engine = std::make_unique<EngineSWC>(nullptr);
    }, std::invalid_argument);
}

// Test engine data reading
TEST_F(EngineTestFixture, EngineDataReading) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Get initial engine data
    const auto& initial_data = engine_swc->getCurrentEngineData();
    EXPECT_GE(initial_data.rpm, 800);  // Should be at idle or higher
    EXPECT_GE(initial_data.temperature, 80);  // Reasonable temperature
    EXPECT_GE(initial_data.throttle_position, 0);  // Non-negative throttle
    EXPECT_LE(initial_data.throttle_position, 100);  // Max 100%
}

// Test engine data writing through port
TEST_F(EngineTestFixture, EngineDataWriting) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Create test engine data
    EngineData test_data{
        .rpm = 2500,
        .temperature = 95,
        .throttle_position = 50
    };
    
    // Write data through the port interface
    EXPECT_EQ(engine_swc->writeEngineData(test_data), Std_ReturnType::E_OK);
    
    // Verify data can be read back
    EngineData read_data;
    EXPECT_EQ(engine_swc->readEngineData(read_data), Std_ReturnType::E_OK);
    EXPECT_EQ(read_data.rpm, 2500);
    EXPECT_EQ(read_data.temperature, 95);
    EXPECT_EQ(read_data.throttle_position, 50);
}

// Test manual throttle control
TEST_F(EngineTestFixture, ManualThrottleControl) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test setting throttle position
    engine_swc->setThrottlePosition(75);
    
    // Get current data and verify throttle was updated
    const auto& current_data = engine_swc->getCurrentEngineData();
    EXPECT_EQ(current_data.throttle_position, 75);
    
    // Test boundary conditions
    engine_swc->setThrottlePosition(150);  // Should be clamped to 100
    const auto& clamped_data = engine_swc->getCurrentEngineData();
    EXPECT_EQ(clamped_data.throttle_position, 100);
}

// Test target RPM setting
TEST_F(EngineTestFixture, TargetRPMControl) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Set target RPM
    engine_swc->setTargetRPM(3000);
    
    // The actual RPM should move toward target over time
    // We can't test this directly without running the simulation,
    // but we can verify the target was accepted
    const auto& current_data = engine_swc->getCurrentEngineData();
    EXPECT_GE(current_data.rpm, 800);  // Should still be reasonable
}

// Test engine simulation behavior
TEST_F(EngineTestFixture, EngineSimulation) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Get initial state
    const auto& initial_data = engine_swc->getCurrentEngineData();
    uint16 initial_rpm = initial_data.rpm;
    
    // Start the engine SWC
    engine_swc->start();
    
    // Let it run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    // Stop the engine
    engine_swc->stop();
    
    // Verify that runnable executed
    EXPECT_GT(engine_swc->getCycleCount(), 0);
    
    // Check that engine data might have changed due to simulation
    const auto& final_data = engine_swc->getCurrentEngineData();
    // RPM might have changed due to throttle simulation
    EXPECT_GE(final_data.rpm, 800);  // Should stay within reasonable bounds
    EXPECT_LE(final_data.rpm, 6000);
}

// Test runnable execution directly
TEST_F(EngineTestFixture, RunnableExecution) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    uint32 initial_cycle_count = engine_swc->getCycleCount();
    
    // Call runnable directly
    engine_swc->runnable();
    
    // Verify cycle count increased
    EXPECT_EQ(engine_swc->getCycleCount(), initial_cycle_count + 1);
}

// Test port data consistency
TEST_F(EngineTestFixture, PortDataConsistency) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Set specific engine parameters
    engine_swc->setThrottlePosition(60);
    engine_swc->setTargetRPM(2000);
    
    // Execute one cycle to update port data
    engine_swc->runnable();
    
    // Read data through port
    EngineData port_data;
    EXPECT_EQ(engine_swc->readEngineData(port_data), Std_ReturnType::E_OK);
    
    // Get data directly from SWC
    const auto& direct_data = engine_swc->getCurrentEngineData();
    
    // Port data should match internal data
    EXPECT_EQ(port_data.rpm, direct_data.rpm);
    EXPECT_EQ(port_data.temperature, direct_data.temperature);
    EXPECT_EQ(port_data.throttle_position, direct_data.throttle_position);
}

// Test error handling for invalid operations
TEST_F(EngineTestFixture, ErrorHandling) {
    // Test operations before initialization
    EngineData data;
    EXPECT_EQ(engine_swc->readEngineData(data), Std_ReturnType::E_NOT_OK);
    
    EngineData write_data{.rpm = 1500, .temperature = 90, .throttle_position = 25};
    EXPECT_EQ(engine_swc->writeEngineData(write_data), Std_ReturnType::E_NOT_OK);
    
    // After initialization, operations should work
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    EXPECT_EQ(engine_swc->readEngineData(data), Std_ReturnType::E_OK);
    EXPECT_EQ(engine_swc->writeEngineData(write_data), Std_ReturnType::E_OK);
}

// Test realistic engine parameter ranges
TEST_F(EngineTestFixture, RealisticParameterRanges) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Run simulation for a bit to see parameter evolution
    engine_swc->start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    engine_swc->stop();
    
    const auto& data = engine_swc->getCurrentEngineData();
    
    // Test realistic ranges
    EXPECT_GE(data.rpm, 600);      // Above stall speed
    EXPECT_LE(data.rpm, 7000);     // Below redline
    EXPECT_GE(data.temperature, 70);  // Above ambient
    EXPECT_LE(data.temperature, 120); // Below overheating
    EXPECT_GE(data.throttle_position, 0);   // Minimum throttle
    EXPECT_LE(data.throttle_position, 100); // Maximum throttle
}

// Test C++20 features usage
TEST_F(EngineTestFixture, Cpp20Features) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test designated initializers with engine data
    EngineData modern_data{
        .rpm = 1800,
        .temperature = 92,
        .throttle_position = 35
    };
    
    EXPECT_EQ(engine_swc->writeEngineData(modern_data), Std_ReturnType::E_OK);
    
    EngineData read_data;
    EXPECT_EQ(engine_swc->readEngineData(read_data), Std_ReturnType::E_OK);
    EXPECT_EQ(read_data.rpm, 1800);
    EXPECT_EQ(read_data.temperature, 92);
    EXPECT_EQ(read_data.throttle_position, 35);
}

// Test multithreaded safety
TEST_F(EngineTestFixture, ThreadSafety) {
    EXPECT_EQ(engine_swc->initialize(), Std_ReturnType::E_OK);
    
    std::atomic<bool> stop_test(false);
    std::atomic<int> read_success_count(0);
    std::atomic<int> write_success_count(0);
    
    // Reader thread
    std::thread reader([&]() {
        while (!stop_test) {
            EngineData data;
            if (engine_swc->readEngineData(data) == Std_ReturnType::E_OK) {
                read_success_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
    });
    
    // Writer thread
    std::thread writer([&]() {
        int counter = 0;
        while (!stop_test) {
            EngineData data{
                .rpm = static_cast<uint16>(1000 + (counter % 3000)),
                .temperature = static_cast<uint8>(85 + (counter % 20)),
                .throttle_position = static_cast<uint8>(counter % 101)
            };
            if (engine_swc->writeEngineData(data) == Std_ReturnType::E_OK) {
                write_success_count++;
            }
            counter++;
            std::this_thread::sleep_for(std::chrono::microseconds(700));
        }
    });
    
    // Run test for short duration
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop_test = true;
    
    reader.join();
    writer.join();
    
    EXPECT_GT(read_success_count.load(), 0);
    EXPECT_GT(write_success_count.load(), 0);
}