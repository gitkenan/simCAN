"""
Engine Software Component (SWC) - AUTOSAR Learning Implementation

This demonstrates how an Engine Control Unit would be implemented as an 
AUTOSAR Software Component with proper ports and interfaces.
"""

import time
import random
import struct
import can
from .autosar_base import (
    SoftwareComponent, Port, PortDirection, 
    ENGINE_DATA_INTERFACE
)


class EngineSWC(SoftwareComponent):
    """
    Engine Software Component
    
    In a real vehicle, this would be part of the Engine Control Module (ECM).
    It monitors and controls engine parameters like RPM, temperature, and throttle.
    
    AUTOSAR Ports:
    - EngineData_P (Sender): Sends engine data to other SWCs
    """
    
    def __init__(self, can_bus):
        super().__init__("EngineSWC")
        
        # CAN bus for low-level communication
        self.can_bus = can_bus
        
        # Engine state variables
        self.engine_rpm = 800  # Idle RPM
        self.engine_temperature = 90  # Celsius
        self.throttle_position = 0  # 0-100%
        
        # Create AUTOSAR ports
        self.setup_ports()
        
    def setup_ports(self):
        """Setup AUTOSAR ports for this SWC"""
        
        # Sender port for engine data
        engine_data_port = Port(
            name="EngineData_P",
            direction=PortDirection.SENDER,
            interface=ENGINE_DATA_INTERFACE
        )
        self.add_port(engine_data_port)
        
    def update_engine_simulation(self):
        """Simulate engine behavior - this would be real sensor readings in production"""
        
        # Simulate RPM changes (idle to redline)
        rpm_change = random.randint(-50, 100)
        self.engine_rpm = max(800, min(6000, self.engine_rpm + rpm_change))
        
        # Simulate temperature changes
        temp_change = random.uniform(-0.5, 1.0)
        self.engine_temperature = max(80, min(120, self.engine_temperature + temp_change))
        
        # Simulate throttle (for demo - normally from pedal sensor)
        if random.random() < 0.1:  # 10% chance to change throttle
            self.throttle_position = random.randint(0, 100)
            
    def send_engine_data_via_rte(self, rte):
        """
        Send engine data through RTE to other SWCs
        This is the AUTOSAR way - SWCs don't communicate directly
        """
        engine_data = {
            "RPM": self.engine_rpm,
            "Temperature": self.engine_temperature, 
            "ThrottlePosition": self.throttle_position
        }
        
        # Send through RTE (Runtime Environment)
        rte.send_data("EngineSWC", "EngineData_P", engine_data)
        
    def send_can_message(self):
        """
        Send CAN message - this represents the Basic Software (BSW) layer
        In real AUTOSAR, this would be handled by the COM module
        """
        try:
            # Pack engine data into CAN message (automotive standard format)
            can_data = struct.pack(
                '>HBB',  # Big-endian: 2-byte RPM, 1-byte temp, 1-byte throttle
                int(self.engine_rpm),
                int(self.engine_temperature),
                int(self.throttle_position)
            )
            
            # Critical priority CAN ID for engine data
            message = can.Message(
                arbitration_id=0x110,  # Standard engine RPM message ID
                data=can_data,
                is_extended_id=False
            )
            
            self.can_bus.send(message)
            
        except Exception as e:
            print(f"[EngineSWC] CAN send error: {e}")
            
    def runnable_main(self):
        """
        Main runnable entity for Engine SWC
        
        In real AUTOSAR, this would be scheduled by the OS at specific intervals
        (e.g., every 10ms for critical engine data)
        """
        print(f"[{self.name}] Starting runnable (simulated 10Hz scheduling)")
        
        while self.running:
            try:
                # 1. Update engine simulation
                self.update_engine_simulation()
                
                # 2. Send data through RTE (will be set up when RTE is ready)
                # self.send_engine_data_via_rte(rte)
                
                # 3. Send CAN message (BSW layer communication)
                self.send_can_message()
                
                # 4. Log current state for learning
                print(f"[{self.name}] RPM: {self.engine_rpm:4d}, "
                      f"Temp: {self.engine_temperature:5.1f}°C, "
                      f"Throttle: {self.throttle_position:3d}%")
                
                # Sleep for 100ms (10Hz update rate)
                time.sleep(0.1)
                
            except Exception as e:
                print(f"[{self.name}] Error in runnable: {e}")
                break
                
    def get_current_data(self):
        """Get current engine data - used by RTE for port communication"""
        return {
            "RPM": self.engine_rpm,
            "Temperature": self.engine_temperature,
            "ThrottlePosition": self.throttle_position
        }