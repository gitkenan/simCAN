# Automotive CAN & AUTOSAR Learning Project

A comprehensive automotive software development project demonstrating **AUTOSAR architecture**, **CAN communication protocols**, and **modern C++ embedded systems programming**. This project showcases production-ready automotive software patterns suitable for ECU development and embedded systems roles.

## 🎯 Project Highlights

### [**AUTOSAR C++ Implementation**](./autosar-cpp/) ⭐ **Main Project**
- **Production-style AUTOSAR architecture** with Software Components (SWCs), Runtime Environment (RTE), and port-based communication
- **Modern C++17** with automotive best practices, smart pointers, and thread-safe programming
- **Realistic automotive features**: Engine control, body control, auto-lighting, CAN messaging
- **Professional build system** with CMake, comprehensive documentation, and demo application
- **Industry-relevant skills**: Directly applicable to automotive ECU development positions

### Key Technical Demonstrations
- **Software Components**: Engine SWC (10Hz), Body Control SWC (2Hz) with proper AUTOSAR lifecycle
- **Runtime Environment**: Thread-safe data routing, scheduling, port connection management  
- **CAN Communication**: Automotive message formats, encoding/decoding, realistic protocol simulation
- **Inter-component Communication**: Type-safe data exchange between ECUs via RTE ports
- **Real-time Scheduling**: Deterministic execution patterns matching production automotive systems

## 🚀 Quick Start

```bash
# Navigate to the main AUTOSAR implementation
cd autosar-cpp

# Build and run the demo
./build.sh
cd build
./autosar_demo
```

This launches a complete AUTOSAR system demonstrating:
- Engine and body control ECU simulation
- Real-time component scheduling
- Inter-component data flow for auto features
- CAN message transmission with automotive protocols
- External interaction simulation (doors, lights)

## 📊 What You'll See

```
=== AUTOSAR C++ Learning Demo ===
[DEMO] Created CAN bus infrastructure
[DEMO] Created Software Components  
[DEMO] Registered SWCs with RTE
[DEMO] Connected SWC ports
[RTE] Connected EngineSWC:EngineDataPort -> BodyControlSWC:EngineDataPort
[DEMO] AUTOSAR system started successfully!

[EngineSWC] Engine Status - RPM: 1245, Temp: 92°C, Throttle: 15%
[BodyControlSWC] Auto headlights: ON (RPM: 1245)  
[CAN] TX ID:0x110 Data:04 dd 5c 0f
[RTE] Scheduler cycle 1000, Data exchanges: 156
```

## 🏗️ Architecture Overview

```
🚗 Vehicle ECUs                     📡 Communication
┌─────────────────┐                ┌─────────────────┐
│   Engine ECU    │◄──────────────►│   CAN Network   │
│                 │                │                 │
│ • RPM Control   │                │ • Message IDs   │
│ • Temperature   │                │ • Data Encoding │
│ • Throttle      │                │ • Bus Arbitr.   │
└─────────────────┘                └─────────────────┘
         ▲                                   ▲
         │                                   │
🔄 AUTOSAR Runtime Environment (RTE)         │
         │                                   │
         ▼                                   ▼
┌─────────────────┐                ┌─────────────────┐
│ Body Control    │◄──────────────►│  Other ECUs     │
│                 │                │                 │  
│ • Door Status   │                │ • Gateway       │
│ • Lighting      │                │ • Diagnostics   │
│ • Auto Features │                │ • Infotainment  │
└─────────────────┘                └─────────────────┘
```

## 🎓 Learning Progression

This project demonstrates a complete learning journey:

1. **[prototypes/](./prototypes/)** - Initial explorations with Python and simple C++
2. **[autosar-learning/](./autosar-learning/)** - Python AUTOSAR concept exploration  
3. **[autosar-cpp/](./autosar-cpp/)** - **Production-ready C++ implementation** ⭐

The final AUTOSAR C++ implementation represents industry-standard automotive software development and directly aligns with skills required for positions at automotive companies like Lear, Continental, Bosch, and others.

## 🛠️ Technical Skills Demonstrated

### AUTOSAR Architecture
- ✅ Software Component design and implementation
- ✅ Runtime Environment with port-based communication
- ✅ Interface definitions and type safety
- ✅ Component lifecycle management and scheduling

### Automotive C++ Programming  
- ✅ Modern C++17 with automotive coding standards
- ✅ Thread-safe programming and resource management
- ✅ Template metaprogramming for type-safe interfaces
- ✅ RAII and smart pointer usage for embedded safety

### Embedded Systems
- ✅ Real-time scheduling and timing constraints
- ✅ Memory management for resource-constrained systems
- ✅ Deterministic behavior and error handling
- ✅ Professional build systems and documentation

### Automotive Domain Knowledge
- ✅ CAN bus protocols and message encoding
- ✅ ECU communication patterns and network topology
- ✅ Realistic automotive feature implementation
- ✅ Industry-standard development practices

## 🎯 Career Relevance

This project directly demonstrates competencies required for automotive software engineering roles:

- **ECU Software Development**: AUTOSAR components, real-time systems, C++ proficiency
- **Automotive Protocols**: CAN communication, message formats, network programming
- **Embedded Systems**: Resource management, timing constraints, safety-critical patterns
- **Software Architecture**: Component-based design, interface specifications, system integration

Perfect preparation for positions requiring AUTOSAR knowledge, embedded C++ development, and automotive domain expertise.

## 📚 Documentation

- **[AUTOSAR Implementation Guide](./autosar-cpp/README.md)** - Complete technical documentation
- **[Architecture Deep-dive](./autosar-cpp/docs/autosar-cpp-learning-blog.md)** - Educational blog post
- **[API Documentation](./autosar-cpp/include/)** - Comprehensive code documentation
- **[Build Instructions](./autosar-cpp/build.sh)** - Professional development workflow

---

*This project showcases the software architecture powering modern vehicles, from engine management to advanced driver assistance systems. The AUTOSAR C++ implementation demonstrates production-ready skills for automotive embedded development.*