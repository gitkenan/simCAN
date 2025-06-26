#include <gtest/gtest.h>
#include "autosar_types.hpp"

using namespace autosar;

class AutosarTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test basic AUTOSAR platform types
TEST_F(AutosarTypesTest, PlatformTypesSizes) {
    EXPECT_EQ(sizeof(uint8), 1);
    EXPECT_EQ(sizeof(uint16), 2);
    EXPECT_EQ(sizeof(uint32), 4);
    EXPECT_EQ(sizeof(uint64), 8);
    EXPECT_EQ(sizeof(sint8), 1);
    EXPECT_EQ(sizeof(sint16), 2);
    EXPECT_EQ(sizeof(sint32), 4);
    EXPECT_EQ(sizeof(float32), 4);
    EXPECT_EQ(sizeof(float64), 8);
    EXPECT_EQ(sizeof(boolean), 1);
}

// Test return types
TEST_F(AutosarTypesTest, ReturnTypes) {
    EXPECT_EQ(static_cast<uint8>(Std_ReturnType::E_OK), 0x00);
    EXPECT_EQ(static_cast<uint8>(Std_ReturnType::E_NOT_OK), 0x01);
}

// Test enums
TEST_F(AutosarTypesTest, Enums) {
    PortDirection sender = PortDirection::SENDER;
    PortDirection receiver = PortDirection::RECEIVER;
    EXPECT_NE(sender, receiver);
    
    InterfaceType sr = InterfaceType::SENDER_RECEIVER;
    InterfaceType cs = InterfaceType::CLIENT_SERVER;
    EXPECT_NE(sr, cs);
    
    SWCState stopped = SWCState::STOPPED;
    SWCState running = SWCState::RUNNING;
    SWCState error = SWCState::ERROR;
    EXPECT_NE(stopped, running);
    EXPECT_NE(running, error);
}

// Test C++20 concepts
TEST_F(AutosarTypesTest, ConceptValidation) {
    // These should compile because they meet AutosarDataType requirements
    static_assert(AutosarDataType<uint32>);
    static_assert(AutosarDataType<EngineData>);
    static_assert(AutosarDataType<DoorStatus>);
    static_assert(AutosarDataType<LightControl>);
    
    // Test numeric concepts
    static_assert(AutosarNumericType<uint16>);
    static_assert(AutosarNumericType<float32>);
    static_assert(!AutosarNumericType<EngineData>);
    
    // Test struct concepts
    static_assert(AutosarStructType<EngineData>);
    static_assert(!AutosarStructType<uint32>);
}

// Test consteval timing functions
TEST_F(AutosarTypesTest, TimingConstants) {
    EXPECT_EQ(get_engine_cycle_time(), 100);
    EXPECT_EQ(get_body_cycle_time(), 500);
    EXPECT_EQ(get_rte_cycle_time(), 10);
    
    EXPECT_EQ(ENGINE_CYCLE_TIME_MS, 100);
    EXPECT_EQ(BODY_CYCLE_TIME_MS, 500);
    EXPECT_EQ(RTE_CYCLE_TIME_MS, 10);
}

// Test CAN IDs
TEST_F(AutosarTypesTest, CANIds) {
    EXPECT_EQ(can_ids::ENGINE_RPM, 0x110);
    EXPECT_EQ(can_ids::ENGINE_TEMP, 0x111);
    EXPECT_EQ(can_ids::DOOR_STATUS, 0x220);
    EXPECT_EQ(can_ids::LIGHT_STATUS, 0x230);
}

// Test EngineData structure
TEST_F(AutosarTypesTest, EngineDataStructure) {
    // Test default construction
    EngineData default_data;
    EXPECT_EQ(default_data.rpm, 800);
    EXPECT_EQ(default_data.temperature, 90);
    EXPECT_EQ(default_data.throttle_position, 0);
    
    // Test designated initializer support
    auto idle_data = EngineData::idle();
    EXPECT_EQ(idle_data.rpm, 800);
    EXPECT_EQ(idle_data.temperature, 90);
    EXPECT_EQ(idle_data.throttle_position, 0);
    
    auto running_data = EngineData::running(2000);
    EXPECT_EQ(running_data.rpm, 2000);
    EXPECT_EQ(running_data.temperature, 95);
    EXPECT_EQ(running_data.throttle_position, 25);
    
    // Test size constraint for CAN compatibility
    EXPECT_LE(sizeof(EngineData), 8);
}

// Test DoorStatus structure
TEST_F(AutosarTypesTest, DoorStatusStructure) {
    // Test default construction
    DoorStatus default_doors;
    EXPECT_FALSE(default_doors.front_left_open);
    EXPECT_FALSE(default_doors.front_right_open);
    EXPECT_FALSE(default_doors.rear_left_open);
    EXPECT_FALSE(default_doors.rear_right_open);
    
    // Test designated initializer support
    auto all_closed = DoorStatus::all_closed();
    EXPECT_FALSE(all_closed.front_left_open);
    EXPECT_FALSE(all_closed.front_right_open);
    EXPECT_FALSE(all_closed.rear_left_open);
    EXPECT_FALSE(all_closed.rear_right_open);
    
    auto driver_open = DoorStatus::driver_open();
    EXPECT_TRUE(driver_open.front_left_open);
    EXPECT_FALSE(driver_open.front_right_open);
    EXPECT_FALSE(driver_open.rear_left_open);
    EXPECT_FALSE(driver_open.rear_right_open);
    
    // Test size constraint for CAN compatibility
    EXPECT_LE(sizeof(DoorStatus), 8);
}

// Test LightControl structure
TEST_F(AutosarTypesTest, LightControlStructure) {
    // Test default construction
    LightControl default_lights;
    EXPECT_FALSE(default_lights.headlights_on);
    EXPECT_FALSE(default_lights.turn_signal_left);
    EXPECT_FALSE(default_lights.turn_signal_right);
    
    // Test designated initializer support
    auto all_off = LightControl::all_off();
    EXPECT_FALSE(all_off.headlights_on);
    EXPECT_FALSE(all_off.turn_signal_left);
    EXPECT_FALSE(all_off.turn_signal_right);
    
    auto headlights_only = LightControl::headlights_only();
    EXPECT_TRUE(headlights_only.headlights_on);
    EXPECT_FALSE(headlights_only.turn_signal_left);
    EXPECT_FALSE(headlights_only.turn_signal_right);
    
    // Test size constraint for CAN compatibility
    EXPECT_LE(sizeof(LightControl), 8);
}

// Test C++20 designated initializers
TEST_F(AutosarTypesTest, DesignatedInitializers) {
    // Test custom engine data with designated initializers
    EngineData custom_engine{
        .rpm = 1500,
        .temperature = 88,
        .throttle_position = 15
    };
    EXPECT_EQ(custom_engine.rpm, 1500);
    EXPECT_EQ(custom_engine.temperature, 88);
    EXPECT_EQ(custom_engine.throttle_position, 15);
    
    // Test partial initialization (remaining fields get defaults)
    EngineData partial_engine{
        .rpm = 3000
    };
    EXPECT_EQ(partial_engine.rpm, 3000);
    EXPECT_EQ(partial_engine.temperature, 90);  // Default value
    EXPECT_EQ(partial_engine.throttle_position, 0);  // Default value
}

// Test data structure properties for automotive compliance
TEST_F(AutosarTypesTest, AutomotiveCompliance) {
    // Test that all data structures are trivially copyable
    static_assert(std::is_trivially_copyable_v<EngineData>);
    static_assert(std::is_trivially_copyable_v<DoorStatus>);
    static_assert(std::is_trivially_copyable_v<LightControl>);
    
    // Test that all data structures have standard layout
    static_assert(std::is_standard_layout_v<EngineData>);
    static_assert(std::is_standard_layout_v<DoorStatus>);
    static_assert(std::is_standard_layout_v<LightControl>);
    
    // Test that all data structures fit in CAN message (8 bytes)
    static_assert(sizeof(EngineData) <= 8);
    static_assert(sizeof(DoorStatus) <= 8);
    static_assert(sizeof(LightControl) <= 8);
}