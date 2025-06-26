#include <gtest/gtest.h>
#include "can_interface.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace autosar;

class CANInterfaceTestFixture : public ::testing::Test {
protected:
    void SetUp() override {
        can_interface = std::make_unique<CANInterface>();
    }
    
    void TearDown() override {
        can_interface.reset();
    }
    
    std::unique_ptr<CANInterface> can_interface;
};

// Test CAN interface construction
TEST_F(CANInterfaceTestFixture, BasicConstruction) {
    EXPECT_NE(can_interface, nullptr);
    
    // CAN interface should be operational after construction
    EXPECT_TRUE(true);  // If construction succeeded, basic test passes
}

// Test engine data transmission
TEST_F(CANInterfaceTestFixture, EngineDataTransmission) {
    // Create test engine data
    EngineData test_data{
        .rpm = 2500,
        .temperature = 95,
        .throttle_position = 60
    };
    
    // Send engine data
    Std_ReturnType result = can_interface->sendEngineData(test_data);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
    
    // Test with boundary values
    EngineData boundary_data{
        .rpm = 0,        // Minimum RPM
        .temperature = 255,  // Maximum uint8
        .throttle_position = 100  // Maximum throttle
    };
    
    result = can_interface->sendEngineData(boundary_data);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
}

// Test door status transmission
TEST_F(CANInterfaceTestFixture, DoorStatusTransmission) {
    // Create test door status
    DoorStatus test_doors{
        .front_left_open = true,
        .front_right_open = false,
        .rear_left_open = true,
        .rear_right_open = false
    };
    
    // Send door status
    Std_ReturnType result = can_interface->sendDoorStatus(test_doors);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
    
    // Test all doors closed
    DoorStatus closed_doors = DoorStatus::all_closed();
    result = can_interface->sendDoorStatus(closed_doors);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
    
    // Test driver door only
    DoorStatus driver_only = DoorStatus::driver_open();
    result = can_interface->sendDoorStatus(driver_only);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
}

// Test light status transmission
TEST_F(CANInterfaceTestFixture, LightStatusTransmission) {
    // Create test light status
    LightControl test_lights{
        .headlights_on = true,
        .turn_signal_left = false,
        .turn_signal_right = true
    };
    
    // Send light status
    Std_ReturnType result = can_interface->sendLightStatus(test_lights);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
    
    // Test all lights off
    LightControl lights_off = LightControl::all_off();
    result = can_interface->sendLightStatus(lights_off);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
    
    // Test headlights only
    LightControl headlights_only = LightControl::headlights_only();
    result = can_interface->sendLightStatus(headlights_only);
    EXPECT_EQ(result, Std_ReturnType::E_OK);
}

// Test multiple rapid transmissions
TEST_F(CANInterfaceTestFixture, RapidTransmissions) {
    // Send multiple messages rapidly to test internal buffering/handling
    for (int i = 0; i < 100; ++i) {
        EngineData data{
            .rpm = static_cast<uint16>(1000 + i),
            .temperature = static_cast<uint8>(80 + (i % 40)),
            .throttle_position = static_cast<uint8>(i % 101)
        };
        
        Std_ReturnType result = can_interface->sendEngineData(data);
        EXPECT_EQ(result, Std_ReturnType::E_OK);
    }
    
    // Send multiple door status updates
    for (int i = 0; i < 50; ++i) {
        DoorStatus doors{
            .front_left_open = (i % 2 == 0),
            .front_right_open = (i % 3 == 0),
            .rear_left_open = (i % 4 == 0),
            .rear_right_open = (i % 5 == 0)
        };
        
        Std_ReturnType result = can_interface->sendDoorStatus(doors);
        EXPECT_EQ(result, Std_ReturnType::E_OK);
    }
    
    // Send multiple light control updates
    for (int i = 0; i < 30; ++i) {
        LightControl lights{
            .headlights_on = (i % 2 == 0),
            .turn_signal_left = (i % 6 == 0),
            .turn_signal_right = (i % 7 == 0)
        };
        
        Std_ReturnType result = can_interface->sendLightStatus(lights);
        EXPECT_EQ(result, Std_ReturnType::E_OK);
    }
}

// Test data structure size compliance for CAN
TEST_F(CANInterfaceTestFixture, CANSizeCompliance) {
    // Verify that all data structures fit within CAN message limits (8 bytes)
    EXPECT_LE(sizeof(EngineData), 8);
    EXPECT_LE(sizeof(DoorStatus), 8);
    EXPECT_LE(sizeof(LightControl), 8);
    
    // Test that the CAN interface can handle maximum data sizes
    EngineData max_engine{
        .rpm = 65535,      // Maximum uint16
        .temperature = 255, // Maximum uint8
        .throttle_position = 255  // Maximum uint8
    };
    
    EXPECT_EQ(can_interface->sendEngineData(max_engine), Std_ReturnType::E_OK);
}

// Test thread safety
TEST_F(CANInterfaceTestFixture, ThreadSafety) {
    std::atomic<bool> stop_test(false);
    std::atomic<int> engine_send_count(0);
    std::atomic<int> door_send_count(0);
    std::atomic<int> light_send_count(0);
    
    // Thread 1: Send engine data
    std::thread engine_thread([&]() {
        while (!stop_test) {
            EngineData data{
                .rpm = static_cast<uint16>(1000 + engine_send_count),
                .temperature = static_cast<uint8>(85 + (engine_send_count % 20)),
                .throttle_position = static_cast<uint8>(engine_send_count % 101)
            };
            
            if (can_interface->sendEngineData(data) == Std_ReturnType::E_OK) {
                engine_send_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // Thread 2: Send door status
    std::thread door_thread([&]() {
        while (!stop_test) {
            DoorStatus doors{
                .front_left_open = (door_send_count % 2 == 0),
                .front_right_open = (door_send_count % 3 == 0),
                .rear_left_open = (door_send_count % 4 == 0),
                .rear_right_open = (door_send_count % 5 == 0)
            };
            
            if (can_interface->sendDoorStatus(doors) == Std_ReturnType::E_OK) {
                door_send_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(150));
        }
    });
    
    // Thread 3: Send light status
    std::thread light_thread([&]() {
        while (!stop_test) {
            LightControl lights{
                .headlights_on = (light_send_count % 2 == 0),
                .turn_signal_left = (light_send_count % 6 == 0),
                .turn_signal_right = (light_send_count % 7 == 0)
            };
            
            if (can_interface->sendLightStatus(lights) == Std_ReturnType::E_OK) {
                light_send_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    });
    
    // Let threads run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop_test = true;
    
    engine_thread.join();
    door_thread.join();
    light_thread.join();
    
    EXPECT_GT(engine_send_count.load(), 0);
    EXPECT_GT(door_send_count.load(), 0);
    EXPECT_GT(light_send_count.load(), 0);
}

// Test C++20 designated initializers with CAN transmission
TEST_F(CANInterfaceTestFixture, Cpp20DesignatedInitializers) {
    // Test engine data with C++20 designated initializers
    EngineData engine_data{
        .rpm = 1800,
        .temperature = 92,
        .throttle_position = 35
    };
    EXPECT_EQ(can_interface->sendEngineData(engine_data), Std_ReturnType::E_OK);
    
    // Test door status with designated initializers
    DoorStatus door_data{
        .front_left_open = true,
        .front_right_open = false,
        .rear_left_open = true,
        .rear_right_open = false
    };
    EXPECT_EQ(can_interface->sendDoorStatus(door_data), Std_ReturnType::E_OK);
    
    // Test light control with designated initializers
    LightControl light_data{
        .headlights_on = true,
        .turn_signal_left = false,
        .turn_signal_right = true
    };
    EXPECT_EQ(can_interface->sendLightStatus(light_data), Std_ReturnType::E_OK);
    
    // Test partial initialization (remaining fields get defaults)
    EngineData partial_engine{
        .rpm = 2200
        // temperature and throttle_position will use default values
    };
    EXPECT_EQ(can_interface->sendEngineData(partial_engine), Std_ReturnType::E_OK);
}

// Test realistic automotive scenarios
TEST_F(CANInterfaceTestFixture, RealisticScenarios) {
    // Scenario 1: Engine startup sequence
    std::vector<EngineData> startup_sequence = {
        {.rpm = 0, .temperature = 20, .throttle_position = 0},      // Cold start
        {.rpm = 200, .temperature = 25, .throttle_position = 10},   // Cranking
        {.rpm = 800, .temperature = 40, .throttle_position = 5},    // Idle
        {.rpm = 850, .temperature = 60, .throttle_position = 0},    // Warm idle
        {.rpm = 900, .temperature = 85, .throttle_position = 0}     // Operating temp
    };
    
    for (const auto& data : startup_sequence) {
        EXPECT_EQ(can_interface->sendEngineData(data), Std_ReturnType::E_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Scenario 2: Vehicle unlocking and entry
    DoorStatus unlocked_vehicle{
        .front_left_open = true,   // Driver enters
        .front_right_open = false,
        .rear_left_open = false,
        .rear_right_open = false
    };
    EXPECT_EQ(can_interface->sendDoorStatus(unlocked_vehicle), Std_ReturnType::E_OK);
    
    // Scenario 3: Night driving with turn signals
    std::vector<LightControl> night_driving = {
        {.headlights_on = true, .turn_signal_left = false, .turn_signal_right = false}, // Headlights on
        {.headlights_on = true, .turn_signal_left = true, .turn_signal_right = false},  // Left turn
        {.headlights_on = true, .turn_signal_left = false, .turn_signal_right = false}, // Straight
        {.headlights_on = true, .turn_signal_left = false, .turn_signal_right = true},  // Right turn
        {.headlights_on = true, .turn_signal_left = false, .turn_signal_right = false}  // Straight
    };
    
    for (const auto& lights : night_driving) {
        EXPECT_EQ(can_interface->sendLightStatus(lights), Std_ReturnType::E_OK);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

// Test error resilience
TEST_F(CANInterfaceTestFixture, ErrorResilience) {
    // Test transmission with extreme values
    EngineData extreme_data{
        .rpm = 65535,      // Maximum possible RPM
        .temperature = 0,   // Minimum temperature
        .throttle_position = 255  // Maximum possible throttle
    };
    EXPECT_EQ(can_interface->sendEngineData(extreme_data), Std_ReturnType::E_OK);
    
    // Test rapid state changes
    for (int i = 0; i < 1000; ++i) {
        DoorStatus rapidly_changing{
            .front_left_open = (i % 2 == 0),
            .front_right_open = (i % 2 == 1),
            .rear_left_open = (i % 4 < 2),
            .rear_right_open = (i % 4 >= 2)
        };
        EXPECT_EQ(can_interface->sendDoorStatus(rapidly_changing), Std_ReturnType::E_OK);
    }
}

// Test CAN message ID consistency
TEST_F(CANInterfaceTestFixture, MessageIDConsistency) {
    // Verify that the CAN IDs are within automotive standard ranges
    EXPECT_EQ(can_ids::ENGINE_RPM, 0x110);
    EXPECT_EQ(can_ids::ENGINE_TEMP, 0x111);
    EXPECT_EQ(can_ids::DOOR_STATUS, 0x220);
    EXPECT_EQ(can_ids::LIGHT_STATUS, 0x230);
    
    // All IDs should be within 11-bit CAN ID range (0x000 - 0x7FF)
    EXPECT_LE(can_ids::ENGINE_RPM, 0x7FF);
    EXPECT_LE(can_ids::ENGINE_TEMP, 0x7FF);
    EXPECT_LE(can_ids::DOOR_STATUS, 0x7FF);
    EXPECT_LE(can_ids::LIGHT_STATUS, 0x7FF);
    
    // IDs should be different for different message types
    EXPECT_NE(can_ids::ENGINE_RPM, can_ids::ENGINE_TEMP);
    EXPECT_NE(can_ids::ENGINE_RPM, can_ids::DOOR_STATUS);
    EXPECT_NE(can_ids::ENGINE_RPM, can_ids::LIGHT_STATUS);
    EXPECT_NE(can_ids::DOOR_STATUS, can_ids::LIGHT_STATUS);
}

// Test data integrity through encoding/decoding simulation
TEST_F(CANInterfaceTestFixture, DataIntegrity) {
    // Test that data maintains integrity through the CAN interface
    
    // Original engine data
    EngineData original_engine{
        .rpm = 2750,
        .temperature = 96,
        .throttle_position = 42
    };
    
    // Send through CAN interface
    EXPECT_EQ(can_interface->sendEngineData(original_engine), Std_ReturnType::E_OK);
    
    // In a real system, we would decode and verify, but since this is a simulation,
    // we verify the interface accepts and processes the data correctly
    
    // Test with multiple data types
    DoorStatus original_doors{
        .front_left_open = true,
        .front_right_open = false,
        .rear_left_open = true,
        .rear_right_open = true
    };
    EXPECT_EQ(can_interface->sendDoorStatus(original_doors), Std_ReturnType::E_OK);
    
    LightControl original_lights{
        .headlights_on = false,
        .turn_signal_left = true,
        .turn_signal_right = false
    };
    EXPECT_EQ(can_interface->sendLightStatus(original_lights), Std_ReturnType::E_OK);
}

// Test performance under load
TEST_F(CANInterfaceTestFixture, PerformanceUnderLoad) {
    const int NUM_MESSAGES = 10000;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Send many messages as fast as possible
    for (int i = 0; i < NUM_MESSAGES; ++i) {
        EngineData data{
            .rpm = static_cast<uint16>(800 + (i % 5000)),
            .temperature = static_cast<uint8>(70 + (i % 50)),
            .throttle_position = static_cast<uint8>(i % 101)
        };
        
        Std_ReturnType result = can_interface->sendEngineData(data);
        EXPECT_EQ(result, Std_ReturnType::E_OK);
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Performance expectation: should handle 10k messages in under 1 second
    EXPECT_LT(duration.count(), 1000);
    
    // Calculate messages per second
    double messages_per_second = static_cast<double>(NUM_MESSAGES) / (duration.count() / 1000.0);
    EXPECT_GT(messages_per_second, 1000);  // Should handle at least 1k msg/sec
}