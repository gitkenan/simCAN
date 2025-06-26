#include <gtest/gtest.h>
#include "body_swc.hpp"
#include "can_interface.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace autosar;

class BodySWCTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        can_interface = std::make_shared<CANInterface>();
        body_swc = std::make_unique<BodyControlSWC>(can_interface);
    }
    
    void TearDown() override {
        if (body_swc) {
            body_swc->stop();
        }
        body_swc.reset();
    }
    
    std::shared_ptr<CANInterface> can_interface;
    std::unique_ptr<BodyControlSWC> body_swc;
};

// Test BodyControlSWC construction and basic properties
TEST_F(BodySWCTestFixture, BasicConstruction) {
    EXPECT_EQ(body_swc->getName(), "BodyControlSWC");
    EXPECT_EQ(body_swc->getState(), SWCState::STOPPED);
    EXPECT_EQ(body_swc->getCycleCount(), 0);
    
    // Test that CAN interface is properly stored
    EXPECT_NE(can_interface, nullptr);
}

// Test initialization
TEST_F(BodySWCTestFixture, Initialization) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Verify ports were created during initialization
    auto door_port = body_swc->getPort("DoorStatusPort");
    auto light_port = body_swc->getPort("LightControlPort");
    auto engine_port = body_swc->getPort("EngineDataPort");
    
    EXPECT_NE(door_port, nullptr);
    EXPECT_NE(light_port, nullptr);
    EXPECT_NE(engine_port, nullptr);
    
    EXPECT_EQ(door_port->getName(), "DoorStatusPort");
    EXPECT_EQ(door_port->getDirection(), PortDirection::SENDER);
    
    EXPECT_EQ(light_port->getName(), "LightControlPort");
    EXPECT_EQ(light_port->getDirection(), PortDirection::RECEIVER);
    
    EXPECT_EQ(engine_port->getName(), "EngineDataPort");
    EXPECT_EQ(engine_port->getDirection(), PortDirection::RECEIVER);
    
    // Check that we have the correct number of ports
    const auto& ports = body_swc->getPorts();
    EXPECT_EQ(ports.size(), 3);  // Door, Light, Engine ports
}

// Test null CAN interface handling
TEST_F(BodySWCTestFixture, NullCANInterface) {
    EXPECT_THROW({
        auto bad_body = std::make_unique<BodyControlSWC>(nullptr);
    }, std::invalid_argument);
}

// Test initial door status
TEST_F(BodySWCTestFixture, InitialDoorStatus) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    const auto& door_status = body_swc->getCurrentDoorStatus();
    
    // All doors should initially be closed
    EXPECT_FALSE(door_status.front_left_open);
    EXPECT_FALSE(door_status.front_right_open);
    EXPECT_FALSE(door_status.rear_left_open);
    EXPECT_FALSE(door_status.rear_right_open);
}

// Test initial light control
TEST_F(BodySWCTestFixture, InitialLightControl) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    const auto& light_control = body_swc->getCurrentLightControl();
    
    // All lights should initially be off
    EXPECT_FALSE(light_control.headlights_on);
    EXPECT_FALSE(light_control.turn_signal_left);
    EXPECT_FALSE(light_control.turn_signal_right);
}

// Test initial window positions
TEST_F(BodySWCTestFixture, InitialWindowPositions) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    const auto& window_positions = body_swc->getCurrentWindowPosition();
    
    // All windows should initially be closed
    EXPECT_EQ(window_positions.front_left, 0);
    EXPECT_EQ(window_positions.front_right, 0);
    EXPECT_EQ(window_positions.rear_left, 0);
    EXPECT_EQ(window_positions.rear_right, 0);
}

// Test door control functionality
TEST_F(BodySWCTestFixture, DoorControl) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test opening front left door
    body_swc->setDoorOpen(0, true);  // Front left = index 0
    const auto& status_after_open = body_swc->getCurrentDoorStatus();
    EXPECT_TRUE(status_after_open.front_left_open);
    EXPECT_FALSE(status_after_open.front_right_open);
    EXPECT_FALSE(status_after_open.rear_left_open);
    EXPECT_FALSE(status_after_open.rear_right_open);
    
    // Test opening rear right door
    body_swc->setDoorOpen(3, true);  // Rear right = index 3
    const auto& status_multiple = body_swc->getCurrentDoorStatus();
    EXPECT_TRUE(status_multiple.front_left_open);
    EXPECT_FALSE(status_multiple.front_right_open);
    EXPECT_FALSE(status_multiple.rear_left_open);
    EXPECT_TRUE(status_multiple.rear_right_open);
    
    // Test closing doors
    body_swc->setDoorOpen(0, false);
    body_swc->setDoorOpen(3, false);
    const auto& status_closed = body_swc->getCurrentDoorStatus();
    EXPECT_FALSE(status_closed.front_left_open);
    EXPECT_FALSE(status_closed.front_right_open);
    EXPECT_FALSE(status_closed.rear_left_open);
    EXPECT_FALSE(status_closed.rear_right_open);
}

// Test window control functionality
TEST_F(BodySWCTestFixture, WindowControl) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test setting window positions
    body_swc->setWindowPosition(0, 50);  // Front left = index 0, 50% open
    body_swc->setWindowPosition(1, 75);  // Front right = index 1, 75% open
    
    const auto& positions = body_swc->getCurrentWindowPosition();
    EXPECT_EQ(positions.front_left, 50);
    EXPECT_EQ(positions.front_right, 75);
    EXPECT_EQ(positions.rear_left, 0);   // Unchanged
    EXPECT_EQ(positions.rear_right, 0);  // Unchanged
    
    // Test boundary conditions
    body_swc->setWindowPosition(2, 150);  // Should be clamped to 100
    const auto& clamped_positions = body_swc->getCurrentWindowPosition();
    EXPECT_EQ(clamped_positions.rear_left, 100);
}

// Test light control functionality
TEST_F(BodySWCTestFixture, LightControl) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Create light control data
    LightControl test_lights{
        .headlights_on = true,
        .turn_signal_left = false,
        .turn_signal_right = true
    };
    
    body_swc->setLightControl(test_lights);
    const auto& current_lights = body_swc->getCurrentLightControl();
    
    EXPECT_TRUE(current_lights.headlights_on);
    EXPECT_FALSE(current_lights.turn_signal_left);
    EXPECT_TRUE(current_lights.turn_signal_right);
}

// Test door status port communication
TEST_F(BodySWCTestFixture, DoorStatusPort) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Set some door states
    body_swc->setDoorOpen(0, true);  // Front left
    body_swc->setDoorOpen(2, true);  // Rear left
    
    // Write to port
    const auto& current_status = body_swc->getCurrentDoorStatus();
    EXPECT_EQ(body_swc->writeDoorStatus(current_status), Std_ReturnType::E_OK);
    
    // Read from port
    DoorStatus read_status;
    EXPECT_EQ(body_swc->readDoorStatus(read_status), Std_ReturnType::E_OK);
    
    EXPECT_TRUE(read_status.front_left_open);
    EXPECT_FALSE(read_status.front_right_open);
    EXPECT_TRUE(read_status.rear_left_open);
    EXPECT_FALSE(read_status.rear_right_open);
}

// Test light control port communication
TEST_F(BodySWCTestFixture, LightControlPort) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Create and set light control
    LightControl test_lights{
        .headlights_on = true,
        .turn_signal_left = true,
        .turn_signal_right = false
    };
    
    body_swc->setLightControl(test_lights);
    
    // Read from port
    LightControl read_lights;
    EXPECT_EQ(body_swc->readLightControl(read_lights), Std_ReturnType::E_OK);
    
    EXPECT_TRUE(read_lights.headlights_on);
    EXPECT_TRUE(read_lights.turn_signal_left);
    EXPECT_FALSE(read_lights.turn_signal_right);
}

// Test engine data port communication
TEST_F(BodySWCTestFixture, EngineDataPort) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Initially, engine data should not be available
    EngineData engine_data;
    EXPECT_EQ(body_swc->readEngineData(engine_data), Std_ReturnType::E_NOT_OK);
}

// Test runnable execution
TEST_F(BodySWCTestFixture, RunnableExecution) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    uint32 initial_cycle_count = body_swc->getCycleCount();
    
    // Execute runnable directly
    body_swc->runnable();
    
    // Verify cycle count increased
    EXPECT_EQ(body_swc->getCycleCount(), initial_cycle_count + 1);
}

// Test simulation behavior over time
TEST_F(BodySWCTestFixture, SimulationBehavior) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Start the body control SWC
    body_swc->start();
    
    // Let it run for a short time to allow simulation activity
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));  // 1 second
    
    // Stop the SWC
    body_swc->stop();
    
    // Verify that it executed multiple cycles
    EXPECT_GT(body_swc->getCycleCount(), 0);
    
    // Due to random simulation, we can't predict exact door/window states,
    // but we can verify they're within valid ranges
    const auto& door_status = body_swc->getCurrentDoorStatus();
    const auto& window_positions = body_swc->getCurrentWindowPosition();
    
    // Window positions should be 0-100%
    EXPECT_GE(window_positions.front_left, 0);
    EXPECT_LE(window_positions.front_left, 100);
    EXPECT_GE(window_positions.front_right, 0);
    EXPECT_LE(window_positions.front_right, 100);
    EXPECT_GE(window_positions.rear_left, 0);
    EXPECT_LE(window_positions.rear_left, 100);
    EXPECT_GE(window_positions.rear_right, 0);
    EXPECT_LE(window_positions.rear_right, 100);
}

// Test error handling for invalid operations
TEST_F(BodySWCTestFixture, ErrorHandling) {
    // Test operations before initialization
    DoorStatus door_data;
    EXPECT_EQ(body_swc->readDoorStatus(door_data), Std_ReturnType::E_NOT_OK);
    
    LightControl light_data;
    EXPECT_EQ(body_swc->readLightControl(light_data), Std_ReturnType::E_NOT_OK);
    
    EngineData engine_data;
    EXPECT_EQ(body_swc->readEngineData(engine_data), Std_ReturnType::E_NOT_OK);
    
    // After initialization, most operations should work
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    EXPECT_EQ(body_swc->readDoorStatus(door_data), Std_ReturnType::E_OK);
    EXPECT_EQ(body_swc->readLightControl(light_data), Std_ReturnType::E_OK);
    // Engine data will still fail until actual engine data is provided
    EXPECT_EQ(body_swc->readEngineData(engine_data), Std_ReturnType::E_NOT_OK);
}

// Test invalid door/window indices
TEST_F(BodySWCTestFixture, InvalidIndices) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test invalid door indices (should be ignored gracefully)
    body_swc->setDoorOpen(255, true);  // Invalid index
    body_swc->setWindowPosition(255, 50);  // Invalid index
    
    // Verify no crash and normal operation continues
    const auto& door_status = body_swc->getCurrentDoorStatus();
    EXPECT_FALSE(door_status.front_left_open);  // Should remain unchanged
}

// Test C++20 designated initializers in data structures
TEST_F(BodySWCTestFixture, Cpp20DesignatedInitializers) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test using C++20 designated initializers
    DoorStatus test_doors{
        .front_left_open = true,
        .front_right_open = false,
        .rear_left_open = true,
        .rear_right_open = false
    };
    
    EXPECT_EQ(body_swc->writeDoorStatus(test_doors), Std_ReturnType::E_OK);
    
    DoorStatus read_doors;
    EXPECT_EQ(body_swc->readDoorStatus(read_doors), Std_ReturnType::E_OK);
    
    EXPECT_TRUE(read_doors.front_left_open);
    EXPECT_FALSE(read_doors.front_right_open);
    EXPECT_TRUE(read_doors.rear_left_open);
    EXPECT_FALSE(read_doors.rear_right_open);
}

// Test thread safety
TEST_F(BodySWCTestFixture, ThreadSafety) {
    EXPECT_EQ(body_swc->initialize(), Std_ReturnType::E_OK);
    
    std::atomic<bool> stop_test(false);
    std::atomic<int> operation_count(0);
    
    // Thread 1: Door operations
    std::thread door_thread([&]() {
        while (!stop_test) {
            body_swc->setDoorOpen(0, true);
            body_swc->setDoorOpen(0, false);
            operation_count++;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // Thread 2: Window operations
    std::thread window_thread([&]() {
        while (!stop_test) {
            body_swc->setWindowPosition(0, 50);
            body_swc->setWindowPosition(0, 0);
            operation_count++;
            std::this_thread::sleep_for(std::chrono::microseconds(150));
        }
    });
    
    // Thread 3: Port read operations
    std::thread read_thread([&]() {
        while (!stop_test) {
            DoorStatus doors;
            LightControl lights;
            body_swc->readDoorStatus(doors);
            body_swc->readLightControl(lights);
            operation_count++;
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    });
    
    // Run test for short duration
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop_test = true;
    
    door_thread.join();
    window_thread.join();
    read_thread.join();
    
    EXPECT_GT(operation_count.load(), 0);
    
    // Verify system is still in valid state
    const auto& final_doors = body_swc->getCurrentDoorStatus();
    const auto& final_windows = body_swc->getCurrentWindowPosition();
    const auto& final_lights = body_swc->getCurrentLightControl();
    
    // Just verify we can read the state without crashes
    EXPECT_GE(final_windows.front_left, 0);
    EXPECT_LE(final_windows.front_left, 100);
}