#include <gtest/gtest.h>
#include "swc_base.hpp"
#include <memory>
#include <thread>
#include <chrono>

using namespace autosar;

class TestSWC : public SoftwareComponent {
public:
    TestSWC(const std::string& name) : SoftwareComponent(name), runnable_count_(0) {}
    
    Std_ReturnType initialize() override {
        return Std_ReturnType::E_OK;
    }
    
    void runnable() override {
        runnable_count_++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    int getRunnableCount() const { return runnable_count_; }
    
private:
    std::atomic<int> runnable_count_;
};

class SWCBaseTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_swc = std::make_unique<TestSWC>("TestSWC");
        engine_interface = std::make_shared<EngineDataInterface>();
        door_interface = std::make_shared<DoorStatusInterface>();
        light_interface = std::make_shared<LightControlInterface>();
    }
    
    void TearDown() override {
        test_swc.reset();
    }
    
    std::unique_ptr<TestSWC> test_swc;
    std::shared_ptr<EngineDataInterface> engine_interface;
    std::shared_ptr<DoorStatusInterface> door_interface;
    std::shared_ptr<LightControlInterface> light_interface;
};

// Test Interface class
TEST_F(SWCBaseTest, InterfaceBasics) {
    EXPECT_EQ(engine_interface->getName(), "EngineDataInterface");
    EXPECT_EQ(engine_interface->getType(), InterfaceType::SENDER_RECEIVER);
    
    EXPECT_EQ(door_interface->getName(), "DoorStatusInterface");
    EXPECT_EQ(door_interface->getType(), InterfaceType::SENDER_RECEIVER);
    
    EXPECT_EQ(light_interface->getName(), "LightControlInterface");
    EXPECT_EQ(light_interface->getType(), InterfaceType::SENDER_RECEIVER);
}

// Test TypedPort with EngineData
TEST_F(SWCBaseTest, TypedPortEngineData) {
    auto port = std::make_shared<TypedPort<EngineData>>(
        "EnginePort", PortDirection::SENDER, engine_interface
    );
    
    EXPECT_EQ(port->getName(), "EnginePort");
    EXPECT_EQ(port->getDirection(), PortDirection::SENDER);
    EXPECT_EQ(port->getInterface(), engine_interface);
    EXPECT_FALSE(port->isValid());
    
    // Test C++20 consteval functions
    EXPECT_EQ(port->getDataSize(), sizeof(EngineData));
    EXPECT_TRUE(port->isCANCompatible());
    
    // Test writing data
    EngineData test_data{.rpm = 1500, .temperature = 85, .throttle_position = 20};
    port->write(test_data);
    EXPECT_TRUE(port->isValid());
    
    // Test reading data
    EngineData read_data;
    EXPECT_TRUE(port->read(read_data));
    EXPECT_EQ(read_data.rpm, 1500);
    EXPECT_EQ(read_data.temperature, 85);
    EXPECT_EQ(read_data.throttle_position, 20);
}

// Test TypedPort with DoorStatus
TEST_F(SWCBaseTest, TypedPortDoorStatus) {
    auto port = std::make_shared<TypedPort<DoorStatus>>(
        "DoorPort", PortDirection::RECEIVER, door_interface
    );
    
    EXPECT_EQ(port->getName(), "DoorPort");
    EXPECT_EQ(port->getDirection(), PortDirection::RECEIVER);
    EXPECT_FALSE(port->isValid());
    
    // Test writing with designated initializers
    DoorStatus door_data{
        .front_left_open = true,
        .front_right_open = false,
        .rear_left_open = true,
        .rear_right_open = false
    };
    port->write(door_data);
    EXPECT_TRUE(port->isValid());
    
    // Test reading
    DoorStatus read_doors;
    EXPECT_TRUE(port->read(read_doors));
    EXPECT_TRUE(read_doors.front_left_open);
    EXPECT_FALSE(read_doors.front_right_open);
    EXPECT_TRUE(read_doors.rear_left_open);
    EXPECT_FALSE(read_doors.rear_right_open);
}

// Test TypedPort polymorphic interface
TEST_F(SWCBaseTest, TypedPortPolymorphic) {
    auto port = std::make_shared<TypedPort<LightControl>>(
        "LightPort", PortDirection::SENDER, light_interface
    );
    
    // Test polymorphic access through base Port class
    Port* base_port = port.get();
    
    LightControl light_data{
        .headlights_on = true,
        .turn_signal_left = false,
        .turn_signal_right = true
    };
    
    // Test setData/getData through base interface
    base_port->setData(&light_data);
    
    LightControl read_light;
    EXPECT_TRUE(base_port->getData(&read_light));
    EXPECT_TRUE(read_light.headlights_on);
    EXPECT_FALSE(read_light.turn_signal_left);
    EXPECT_TRUE(read_light.turn_signal_right);
}

// Test C++20 concepts compilation
TEST_F(SWCBaseTest, ConceptsCompileTime) {
    // These should compile successfully due to concepts
    auto engine_port = std::make_shared<TypedPort<EngineData>>(
        "EnginePort", PortDirection::SENDER, engine_interface
    );
    
    auto door_port = std::make_shared<TypedPort<DoorStatus>>(
        "DoorPort", PortDirection::RECEIVER, door_interface
    );
    
    auto light_port = std::make_shared<TypedPort<LightControl>>(
        "LightPort", PortDirection::SENDER, light_interface
    );
    
    // Test that concepts prevent invalid usage at compile time
    static_assert(AutosarDataType<EngineData>);
    static_assert(AutosarDataType<DoorStatus>);
    static_assert(AutosarDataType<LightControl>);
    
    EXPECT_TRUE(engine_port != nullptr);
    EXPECT_TRUE(door_port != nullptr);
    EXPECT_TRUE(light_port != nullptr);
}

// Test Software Component lifecycle
TEST_F(SWCBaseTest, SWCLifecycle) {
    EXPECT_EQ(test_swc->getName(), "TestSWC");
    EXPECT_EQ(test_swc->getState(), SWCState::STOPPED);
    
    // Test initialization
    EXPECT_EQ(test_swc->initialize(), Std_ReturnType::E_OK);
    
    // Test starting
    test_swc->start();
    EXPECT_EQ(test_swc->getState(), SWCState::RUNNING);
    
    // Let it run for a bit
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Test stopping
    test_swc->stop();
    EXPECT_EQ(test_swc->getState(), SWCState::STOPPED);
    
    // Verify runnable was called
    EXPECT_GT(test_swc->getRunnableCount(), 0);
}

// Test port management in SWC
TEST_F(SWCBaseTest, SWCPortManagement) {
    // Create ports
    auto engine_port = std::make_shared<TypedPort<EngineData>>(
        "EnginePort", PortDirection::SENDER, engine_interface
    );
    auto door_port = std::make_shared<TypedPort<DoorStatus>>(
        "DoorPort", PortDirection::RECEIVER, door_interface
    );
    
    // Add ports to SWC
    test_swc->addPort(engine_port);
    test_swc->addPort(door_port);
    
    // Test port retrieval
    auto retrieved_engine = test_swc->getPort("EnginePort");
    auto retrieved_door = test_swc->getPort("DoorPort");
    auto non_existent = test_swc->getPort("NonExistent");
    
    EXPECT_EQ(retrieved_engine, engine_port);
    EXPECT_EQ(retrieved_door, door_port);
    EXPECT_EQ(non_existent, nullptr);
    
    // Test getting all ports
    const auto& all_ports = test_swc->getPorts();
    EXPECT_EQ(all_ports.size(), 2);
}

// Test thread safety of TypedPort
TEST_F(SWCBaseTest, TypedPortThreadSafety) {
    auto port = std::make_shared<TypedPort<EngineData>>(
        "ThreadTestPort", PortDirection::SENDER, engine_interface
    );
    
    std::atomic<bool> stop_flag(false);
    std::atomic<int> write_count(0);
    std::atomic<int> read_count(0);
    
    // Writer thread
    std::thread writer([&]() {
        while (!stop_flag) {
            EngineData data{
                .rpm = static_cast<uint16>(1000 + write_count),
                .temperature = 90,
                .throttle_position = 25
            };
            port->write(data);
            write_count++;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    });
    
    // Reader thread
    std::thread reader([&]() {
        while (!stop_flag) {
            EngineData data;
            if (port->read(data)) {
                read_count++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(150));
        }
    });
    
    // Let threads run for a short time
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    stop_flag = true;
    
    writer.join();
    reader.join();
    
    EXPECT_GT(write_count.load(), 0);
    EXPECT_GT(read_count.load(), 0);
    EXPECT_TRUE(port->isValid());
}

// Test error conditions
TEST_F(SWCBaseTest, ErrorConditions) {
    auto port = std::make_shared<TypedPort<EngineData>>(
        "ErrorTestPort", PortDirection::SENDER, engine_interface
    );
    
    // Test reading from uninitialized port
    EngineData data;
    EXPECT_FALSE(port->read(data));
    EXPECT_FALSE(port->isValid());
    
    // Test null pointer handling in polymorphic interface
    EXPECT_FALSE(port->getData(nullptr));
    port->setData(nullptr);  // Should handle gracefully
    EXPECT_FALSE(port->isValid());
}

// Test C++20 requires clauses functionality
TEST_F(SWCBaseTest, RequiresClauses) {
    auto port = std::make_shared<TypedPort<EngineData>>(
        "RequiresTestPort", PortDirection::SENDER, engine_interface
    );
    
    // These should compile due to requires clauses
    EngineData engine_data{.rpm = 1200, .temperature = 88, .throttle_position = 15};
    port->write(engine_data);
    
    EngineData read_data;
    EXPECT_TRUE(port->read(read_data));
    
    // Verify the requires clauses work with AutosarDataType concept
    static_assert(requires(TypedPort<EngineData> p, EngineData d) {
        p.write(d);
        p.read(d);
    });
}