"""
Body Control Software Component (SWC) - AUTOSAR Learning Implementation

This demonstrates how a Body Control Module would be implemented as an 
AUTOSAR Software Component, handling doors, windows, and lighting systems.
"""

import time
import random
import struct
import can
from .autosar_base import (
    SoftwareComponent, Port, PortDirection,
    DOOR_STATUS_INTERFACE, LIGHT_CONTROL_INTERFACE, ENGINE_DATA_INTERFACE
)


class BodyControlSWC(SoftwareComponent):
    """
    Body Control Software Component
    
    In a real vehicle, this would be part of the Body Control Module (BCM).
    It manages comfort and convenience features like doors, windows, and lights.
    
    AUTOSAR Ports:
    - DoorStatus_P (Sender): Sends door status to other SWCs
    - LightControl_P (Receiver): Receives light commands from HMI/switches
    - EngineData_R (Receiver): Receives engine data to control auto features
    """
    
    def __init__(self, can_bus):
        super().__init__("BodyControlSWC")
        
        # CAN bus for low-level communication
        self.can_bus = can_bus
        
        # Body control state variables
        self.doors = {
            "front_left": False,    # False = closed, True = open
            "front_right": False,
            "rear_left": False, 
            "rear_right": False
        }
        
        self.lights = {
            "headlights": False,
            "turn_left": False,
            "turn_right": False,
            "hazards": False
        }
        
        self.windows = {
            "front_left": 0,    # 0-100% open
            "front_right": 0,
            "rear_left": 0,
            "rear_right": 0
        }
        
        # Auto features state
        self.auto_lights_enabled = True
        self.received_engine_data = None
        
        # Create AUTOSAR ports
        self.setup_ports()
        
    def setup_ports(self):
        """Setup AUTOSAR ports for this SWC"""
        
        # Sender port for door status
        door_status_port = Port(
            name="DoorStatus_P",
            direction=PortDirection.SENDER,
            interface=DOOR_STATUS_INTERFACE
        )
        self.add_port(door_status_port)
        
        # Receiver port for light control commands
        light_control_port = Port(
            name="LightControl_P", 
            direction=PortDirection.RECEIVER,
            interface=LIGHT_CONTROL_INTERFACE
        )
        self.add_port(light_control_port)
        
        # Receiver port for engine data (for auto features)
        engine_data_port = Port(
            name="EngineData_R",
            direction=PortDirection.RECEIVER,
            interface=ENGINE_DATA_INTERFACE
        )
        self.add_port(engine_data_port)
        
    def simulate_door_activity(self):
        """Simulate random door activity for demonstration"""
        if random.random() < 0.05:  # 5% chance per cycle
            door_name = random.choice(list(self.doors.keys()))
            self.doors[door_name] = not self.doors[door_name]
            status = "OPENED" if self.doors[door_name] else "CLOSED"
            print(f"[{self.name}] {door_name.replace('_', ' ').title()} door {status}")
            
    def simulate_window_activity(self):
        """Simulate window movement"""
        if random.random() < 0.03:  # 3% chance per cycle
            window_name = random.choice(list(self.windows.keys()))
            # Random window position change
            self.windows[window_name] = random.randint(0, 100)
            print(f"[{self.name}] {window_name.replace('_', ' ').title()} window: {self.windows[window_name]}% open")
            
    def auto_light_control(self):
        """Automatic light control based on engine RPM (demo feature)"""
        if not self.auto_lights_enabled or not self.received_engine_data:
            return
            
        engine_rpm = self.received_engine_data.get("RPM", 0)
        
        # Auto headlights: turn on when RPM > 1000 (engine running)
        should_be_on = engine_rpm > 1000
        if self.lights["headlights"] != should_be_on:
            self.lights["headlights"] = should_be_on
            status = "ON" if should_be_on else "OFF"
            print(f"[{self.name}] Auto headlights: {status} (RPM: {engine_rpm})")
            
    def send_door_status_via_rte(self, rte):
        """Send door status through RTE to other SWCs"""
        door_data = {
            "FrontLeftOpen": self.doors["front_left"],
            "FrontRightOpen": self.doors["front_right"], 
            "RearLeftOpen": self.doors["rear_left"],
            "RearRightOpen": self.doors["rear_right"]
        }
        
        rte.send_data("BodyControlSWC", "DoorStatus_P", door_data)
        
    def receive_engine_data_via_rte(self, rte):
        """Receive engine data from RTE"""
        engine_data = rte.receive_data("BodyControlSWC", "EngineData_R")
        if engine_data:
            self.received_engine_data = engine_data
            
    def send_can_messages(self):
        """Send body control CAN messages"""
        try:
            # Door status message (ID: 0x220)
            door_bits = 0
            if self.doors["front_left"]: door_bits |= 0x01
            if self.doors["front_right"]: door_bits |= 0x02
            if self.doors["rear_left"]: door_bits |= 0x04
            if self.doors["rear_right"]: door_bits |= 0x08
            
            door_message = can.Message(
                arbitration_id=0x220,
                data=[door_bits, 0, 0, 0, 0, 0, 0, 0],
                is_extended_id=False
            )
            self.can_bus.send(door_message)
            
            # Light status message (ID: 0x230)
            light_bits = 0
            if self.lights["headlights"]: light_bits |= 0x01
            if self.lights["turn_left"]: light_bits |= 0x02
            if self.lights["turn_right"]: light_bits |= 0x04
            if self.lights["hazards"]: light_bits |= 0x08
            
            light_message = can.Message(
                arbitration_id=0x230,
                data=[light_bits, 0, 0, 0, 0, 0, 0, 0],
                is_extended_id=False
            )
            self.can_bus.send(light_message)
            
        except Exception as e:
            print(f"[{self.name}] CAN send error: {e}")
            
    def runnable_main(self):
        """
        Main runnable entity for Body Control SWC
        
        Scheduled at 2Hz (every 500ms) - body control doesn't need to be as fast as engine
        """
        print(f"[{self.name}] Starting runnable (simulated 2Hz scheduling)")
        
        while self.running:
            try:
                # 1. Simulate body control activities
                self.simulate_door_activity()
                self.simulate_window_activity()
                
                # 2. Receive data from other SWCs (will be set up when RTE is ready)
                # self.receive_engine_data_via_rte(rte)
                
                # 3. Execute auto features
                self.auto_light_control()
                
                # 4. Send data through RTE
                # self.send_door_status_via_rte(rte)
                
                # 5. Send CAN messages
                self.send_can_messages()
                
                # 6. Log current state periodically
                if random.random() < 0.2:  # 20% chance to log
                    open_doors = [name for name, is_open in self.doors.items() if is_open]
                    active_lights = [name for name, is_on in self.lights.items() if is_on]
                    
                    if open_doors or active_lights:
                        print(f"[{self.name}] Status - Open doors: {open_doors}, Active lights: {active_lights}")
                
                # Sleep for 500ms (2Hz update rate)
                time.sleep(0.5)
                
            except Exception as e:
                print(f"[{self.name}] Error in runnable: {e}")
                break
                
    def get_current_door_status(self):
        """Get current door status - used by RTE"""
        return {
            "FrontLeftOpen": self.doors["front_left"],
            "FrontRightOpen": self.doors["front_right"],
            "RearLeftOpen": self.doors["rear_left"], 
            "RearRightOpen": self.doors["rear_right"]
        }
        
    def set_light_command(self, light_data):
        """Set light commands - called by RTE when receiving data"""
        self.lights["headlights"] = light_data.get("HeadlightsOn", False)
        self.lights["turn_left"] = light_data.get("TurnSignalLeft", False)
        self.lights["turn_right"] = light_data.get("TurnSignalRight", False)