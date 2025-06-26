# AUTOSAR C++ Test Suite

This directory contains a comprehensive test suite for the AUTOSAR C++ implementation, covering all major components and integration scenarios.

## Test Coverage

### Unit Tests
- **AUTOSAR Types** (`test_autosar_types.cpp`) - Platform types, C++20 concepts, designated initializers
- **Software Component Base** (`test_swc_base.cpp`) - Base classes, typed ports, lifecycle management
- **Engine SWC** (`test_engine_swc.cpp`) - Engine control functionality, realistic automotive behavior
- **Body Control SWC** (`test_body_swc.cpp`) - Door/window/light control, automotive features
- **Runtime Environment** (`test_rte.cpp`) - Communication orchestration, scheduling, data routing
- **CAN Interface** (`test_can_interface.cpp`) - Vehicle network protocols, message encoding

### Integration Tests
- **Complete System** (`test_integration.cpp`) - End-to-end automotive scenarios, performance testing

## Running Tests

### Quick Test Run
```bash
cd autosar-cpp
./test_runner.sh
```

### Manual Build and Test
```bash
cd autosar-cpp
mkdir build_test && cd build_test
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
./autosar_tests
```

### Running Specific Test Categories
```bash
# Run only AUTOSAR types tests
./autosar_tests --gtest_filter="AutosarTypesTest.*"

# Run only Engine SWC tests
./autosar_tests --gtest_filter="EngineTestFixture.*"

# Run only integration tests
./autosar_tests --gtest_filter="IntegrationTestFixture.*"
```

## Test Framework

The test suite uses **Google Test (gtest)** framework with the following features:
- Automatic test discovery
- Parallel test execution
- XML test result output
- Detailed failure reporting
- Memory leak detection (in debug builds)

## Test Categories Explained

### Platform and Type Safety Tests
These tests verify that the AUTOSAR platform types meet automotive requirements:
- Size constraints for CAN message compatibility
- C++20 concept validation for type safety
- Designated initializer functionality
- Compile-time validation with consteval functions

### Component Lifecycle Tests  
Software Component tests cover the complete AUTOSAR lifecycle:
- Initialization and port configuration
- Runnable execution and timing
- Port-based communication
- Thread safety and concurrent access
- Error handling and fault tolerance

### Real-Time System Tests
Runtime Environment tests validate the coordination layer:
- SWC registration and scheduling
- Port connection and data routing
- Multi-threaded execution safety
- Performance under load
- Deterministic timing behavior

### Automotive Protocol Tests
CAN interface tests ensure protocol compliance:
- Message encoding and size validation
- Automotive message ID ranges
- High-frequency transmission handling
- Thread-safe concurrent access
- Performance benchmarking

### System Integration Tests
Integration tests simulate complete automotive scenarios:
- Vehicle startup and shutdown sequences
- Driver interaction scenarios (doors, lights, engine)
- Automatic feature coordination (auto headlights)
- Concurrent system operation
- Error recovery and fault tolerance
- Complete drive cycle simulation

## Performance Benchmarks

The test suite includes performance validation:
- **Message Throughput**: >10,000 CAN messages/second
- **Component Execution**: Engine SWC at 10Hz, Body at 2Hz
- **System Latency**: Data routing <10ms
- **Memory Usage**: All structures ≤8 bytes (CAN compatible)
- **Concurrent Safety**: Multi-threaded operation without data races

## Automotive Compliance

Tests verify compliance with automotive standards:
- AUTOSAR architecture patterns
- CAN protocol message formatting
- Real-time deterministic behavior
- Safety-critical error handling
- Production-ready code patterns

## Continuous Integration

The test suite is designed for CI/CD integration:
- Zero external dependencies (downloads Google Test automatically)
- XML output for build system integration
- Clear pass/fail status codes
- Comprehensive coverage reporting
- Cross-platform compatibility (Linux, Windows, macOS)

## Test Data and Scenarios

Tests use realistic automotive data:
- Engine RPM ranges: 600-7000 RPM
- Temperature ranges: 70-120°C
- Throttle positions: 0-100%
- CAN message IDs in automotive ranges (0x110-0x230)
- Timing constraints matching real vehicle systems

## Debugging Test Failures

For test debugging:
```bash
# Run with verbose output
./autosar_tests --gtest_output=xml:results.xml --gtest_print_time=1

# Run single test with full output
./autosar_tests --gtest_filter="SpecificTest.*" --gtest_break_on_failure

# Debug with GDB
gdb ./autosar_tests
(gdb) run --gtest_filter="FailingTest.*"
```

This comprehensive test suite ensures the AUTOSAR C++ implementation meets automotive industry standards for safety, reliability, and performance.