# AUTOSAR Learning Implementation

This folder contains a simplified AUTOSAR implementation built on top of our existing CAN simulation to help understand the key concepts.

## Project Structure

```
autosar-learning/
├── swc/                 # Software Components
│   ├── engine_swc.py    # Engine Software Component
│   └── body_swc.py      # Body Control Software Component
├── rte/                 # Runtime Environment
│   └── rte.py           # Simple RTE implementation
├── config/              # Configuration files
│   └── swc_config.py    # SWC configuration
├── docs/                # Learning documentation
└── main.py              # Main AUTOSAR application
```

## Key AUTOSAR Concepts Implemented

### Software Components (SWCs)
- **Engine SWC**: Handles engine data (RPM, temperature)
- **Body Control SWC**: Manages doors, windows, lights

### Runtime Environment (RTE)
- Routes data between SWCs through defined ports
- Provides interface abstraction
- Handles timing and scheduling

### Ports and Interfaces
- **Sender Ports**: Send data out of SWC
- **Receiver Ports**: Receive data into SWC
- **Client/Server Ports**: For service-oriented communication

## Learning Objectives

1. Understand what AUTOSAR Software Components actually are
2. See how the Runtime Environment routes data between components
3. Experience the relationship between AUTOSAR and CAN communication
4. Get hands-on with automotive software architecture concepts