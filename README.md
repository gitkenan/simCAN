# Automotive CAN & AUTOSAR Learning Project

A comprehensive automotive software development project demonstrating AUTOSAR architecture, CAN communication protocols, and modern C++ embedded systems programming. This project showcases production-ready automotive software patterns suitable for ECU development and embedded systems roles.

## Project Highlights

### AUTOSAR C++ Implementation - Main Project
The autosar-cpp directory contains a production-style AUTOSAR architecture with Software Components (SWCs), Runtime Environment (RTE), and port-based communication. It demonstrates modern C++20 with automotive best practices, concepts for type safety, designated initializers, smart pointers, and thread-safe programming. The implementation includes realistic automotive features such as engine control, body control, auto-lighting, and CAN messaging, along with a professional build system using CMake.

### Key Technical Demonstrations
The project includes Software Components for engine control (10Hz) and body control (2Hz) with proper AUTOSAR lifecycle management. The Runtime Environment provides thread-safe data routing, scheduling, and port connection management. CAN communication is implemented with automotive message formats, encoding/decoding, and realistic protocol simulation. Inter-component communication demonstrates type-safe data exchange between ECUs via RTE ports, with real-time scheduling that matches deterministic execution patterns found in production automotive systems.

## Quick Start

```bash
# Navigate to the main AUTOSAR implementation
cd autosar-cpp

# Build and run the demo
./build.sh
cd build
./autosar_demo
```

This launches a complete AUTOSAR system demonstrating engine and body control ECU simulation, real-time component scheduling, inter-component data flow for auto features, CAN message transmission with automotive protocols, and external interaction simulation for doors and lights.

## What You'll See

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

## Architecture Overview

The system consists of multiple Vehicle ECUs communicating through a CAN Network. The Engine ECU handles RPM control, temperature monitoring, and throttle management. It communicates with other components via the CAN Network, which manages message IDs, data encoding, and bus arbitration.

The AUTOSAR Runtime Environment (RTE) serves as the central coordination layer, connecting the Engine ECU with the Body Control module and other ECUs. The Body Control ECU manages door status, lighting systems, and auto features, while other ECUs in the system include gateway modules, diagnostics, and infotainment systems.

## Learning Progression

This project demonstrates a complete learning journey. The prototypes directory contains initial explorations with Python and simple C++. The autosar-learning directory shows Python AUTOSAR concept exploration. The autosar-cpp directory contains the production-ready C++ implementation that represents the main focus of the project.

The final AUTOSAR C++ implementation represents industry-standard automotive software development and directly aligns with skills required for positions at automotive companies like Lear, Continental, Bosch, and others.

## Technical Skills Demonstrated

### AUTOSAR Architecture
Software Component design and implementation, Runtime Environment with port-based communication, interface definitions and type safety, and component lifecycle management and scheduling.

### Automotive C++ Programming
Modern C++20 with automotive coding standards, concepts for compile-time type safety, designated initializers for clear data structure initialization, thread-safe programming and resource management, template metaprogramming for type-safe interfaces, and RAII and smart pointer usage for embedded safety.

### Embedded Systems
Real-time scheduling and timing constraints, memory management for resource-constrained systems, deterministic behavior and error handling, and professional build systems and documentation.

### Automotive Domain Knowledge
CAN bus protocols and message encoding, ECU communication patterns and network topology, realistic automotive feature implementation, and industry-standard development practices.

## Career Relevance

This project directly demonstrates competencies required for automotive software engineering roles. ECU Software Development skills include AUTOSAR components, real-time systems, and C++ proficiency. Automotive Protocols experience covers CAN communication, message formats, and network programming. Embedded Systems knowledge encompasses resource management, timing constraints, and safety-critical patterns. Software Architecture capabilities include component-based design, interface specifications, and system integration.

The project provides excellent preparation for positions requiring AUTOSAR knowledge, embedded C++ development, and automotive domain expertise.

## Documentation

The autosar-cpp directory contains complete technical documentation in its README.md file. An educational blog post explaining the architecture is available in the docs subdirectory. Comprehensive code documentation is embedded throughout the include directory headers. Professional development workflow instructions are provided in the build.sh script.

---

*This project showcases the software architecture powering modern vehicles, from engine management to advanced driver assistance systems. The AUTOSAR C++ implementation demonstrates production-ready skills for automotive embedded development.*