#!/usr/bin/env python3
"""
Automotive CAN Bus Simulation using python-can library
Simulates multiple ECUs (Engine, BCM, PDU) with real automotive message types
"""

import can
import time
import random
import threading
import struct
from datetime import datetime
from enum import Enum


class CANPriority(Enum):
    """CAN message priority levels based on automotive standards"""
    CRITICAL = 0x100  # Safety-critical messages (0x100-0x1FF)
    BODY = 0x200      # Body control messages (0x200-0x2FF) 
    COMFORT = 0x300   # Comfort/convenience messages (0x300-0x3FF)


class EngineECU:
    """Engine Control Unit - handles engine-related data"""
    
    def __init__(self, bus):
        self.bus = bus
        self.engine_rpm = 800  # Idle RPM
        self.engine_temp = 90  # Celsius
        self.throttle_pos = 0  # 0-100%
        self.running = True
        
    def simulate(self):
        """Simulate engine ECU sending periodic messages"""
        print("[ENGINE ECU] Starting engine control unit simulation")
        
        while self.running:
            try:
                # Engine RPM (ID: 0x110 - Critical priority)
                self.engine_rpm = max(800, self.engine_rpm + random.randint(-50, 100))
                self.engine_rpm = min(6000, self.engine_rpm)  # Redline limit
                
                rpm_message = can.Message(
                    arbitration_id=0x110,
                    data=struct.pack('>HBB', self.engine_rpm, self.throttle_pos, 0),
                    is_extended_id=False
                )
                self.bus.send(rpm_message)
                
                timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                print(f"[ENGINE ECU] {timestamp} - Engine RPM: {self.engine_rpm}, Throttle: {self.throttle_pos}%")
                
                # Engine Temperature (ID: 0x111)
                self.engine_temp += random.uniform(-1, 2)
                self.engine_temp = max(70, min(120, self.engine_temp))  # 70-120°C range
                
                temp_message = can.Message(
                    arbitration_id=0x111,
                    data=struct.pack('>BxxxBBB', int(self.engine_temp), 0, 0, 0),
                    is_extended_id=False
                )
                self.bus.send(temp_message)
                
                # Vehicle Speed (ID: 0x120 - Critical for safety systems)
                speed_kmh = max(0, (self.engine_rpm - 800) // 50)  # Rough speed calculation
                speed_message = can.Message(
                    arbitration_id=0x120,
                    data=struct.pack('>HxxxxBB', speed_kmh, 0, 0),
                    is_extended_id=False
                )
                self.bus.send(speed_message)
                
            except can.CanError as e:
                print(f"[ENGINE ECU] CAN Error: {e}")
            except Exception as e:
                print(f"[ENGINE ECU] Error: {e}")
                
            time.sleep(0.1)  # 10Hz update rate - typical for engine data


class BodyControlModule:
    """Body Control Module - handles doors, windows, lights"""
    
    def __init__(self, bus):
        self.bus = bus
        self.door_status = {'front_left': False, 'front_right': False, 
                           'rear_left': False, 'rear_right': False}
        self.window_status = {'front_left': 100, 'front_right': 100,
                             'rear_left': 100, 'rear_right': 100}  # 0-100% open
        self.lights = {'headlights': False, 'indicators': 0}  # indicators: 0=off, 1=left, 2=right
        self.running = True
        
    def simulate(self):
        """Simulate BCM sending periodic body control messages"""
        print("[BCM] Starting Body Control Module simulation")
        
        while self.running:
            try:
                # Door Status (ID: 0x200 - Body control priority)
                # Randomly change door status
                if random.random() < 0.05:  # 5% chance to change door status
                    door = random.choice(list(self.door_status.keys()))
                    self.door_status[door] = not self.door_status[door]
                
                door_byte = 0
                for i, (door, status) in enumerate(self.door_status.items()):
                    if status:
                        door_byte |= (1 << i)
                
                door_message = can.Message(
                    arbitration_id=0x200,
                    data=[door_byte, 0, 0, 0, 0, 0, 0, 0],
                    is_extended_id=False
                )
                self.bus.send(door_message)
                
                # Window Control (ID: 0x210)
                if random.random() < 0.03:  # 3% chance to change window position
                    window = random.choice(list(self.window_status.keys()))
                    self.window_status[window] = random.randint(0, 100)
                
                window_data = []
                for window, position in self.window_status.items():
                    window_data.append(position)
                
                window_message = can.Message(
                    arbitration_id=0x210,
                    data=window_data[:8],  # First 4 windows
                    is_extended_id=False
                )
                self.bus.send(window_message)
                
                # Lighting Control (ID: 0x220)
                if random.random() < 0.02:  # 2% chance to change lights
                    self.lights['headlights'] = random.choice([True, False])
                    self.lights['indicators'] = random.randint(0, 2)
                
                light_byte = 0
                if self.lights['headlights']:
                    light_byte |= 0x01
                light_byte |= (self.lights['indicators'] << 1)
                
                light_message = can.Message(
                    arbitration_id=0x220,
                    data=[light_byte, 0, 0, 0, 0, 0, 0, 0],
                    is_extended_id=False
                )
                self.bus.send(light_message)
                
                timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                open_doors = [door for door, status in self.door_status.items() if status]
                if open_doors or any(pos < 100 for pos in self.window_status.values()):
                    print(f"[BCM] {timestamp} - Doors open: {open_doors}, Windows moving")
                
            except can.CanError as e:
                print(f"[BCM] CAN Error: {e}")
            except Exception as e:
                print(f"[BCM] Error: {e}")
                
            time.sleep(0.5)  # 2Hz update rate - typical for body control


class PowerDistributionUnit:
    """Power Distribution Unit - handles electrical system status"""
    
    def __init__(self, bus):
        self.bus = bus
        self.battery_voltage = 12.6  # Volts
        self.alternator_status = True
        self.fuse_status = [True] * 16  # 16 fuses
        self.running = True
        
    def simulate(self):
        """Simulate PDU sending electrical system messages"""
        print("[PDU] Starting Power Distribution Unit simulation")
        
        while self.running:
            try:
                # Battery Status (ID: 0x300 - Comfort priority but important)
                self.battery_voltage += random.uniform(-0.1, 0.1)
                self.battery_voltage = max(11.0, min(14.4, self.battery_voltage))
                
                # Simulate alternator failure occasionally
                if random.random() < 0.001:  # 0.1% chance
                    self.alternator_status = not self.alternator_status
                
                voltage_int = int(self.battery_voltage * 10)  # 0.1V precision
                battery_message = can.Message(
                    arbitration_id=0x300,
                    data=struct.pack('>HBxxxBB', voltage_int, 
                                   int(self.alternator_status), 0, 0),
                    is_extended_id=False
                )
                self.bus.send(battery_message)
                
                # Fuse Status (ID: 0x310)
                if random.random() < 0.0005:  # 0.05% chance of fuse issue
                    fuse_idx = random.randint(0, 15)
                    self.fuse_status[fuse_idx] = not self.fuse_status[fuse_idx]
                
                fuse_bytes = []
                for i in range(0, 16, 8):
                    byte_val = 0
                    for j in range(8):
                        if i + j < 16 and self.fuse_status[i + j]:
                            byte_val |= (1 << j)
                    fuse_bytes.append(byte_val)
                
                fuse_message = can.Message(
                    arbitration_id=0x310,
                    data=fuse_bytes + [0] * (8 - len(fuse_bytes)),
                    is_extended_id=False
                )
                self.bus.send(fuse_message)
                
                timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                if not self.alternator_status or not all(self.fuse_status):
                    blown_fuses = [i for i, status in enumerate(self.fuse_status) if not status]
                    print(f"[PDU] {timestamp} - Battery: {self.battery_voltage:.1f}V, "
                          f"Alternator: {'OK' if self.alternator_status else 'FAULT'}, "
                          f"Blown fuses: {blown_fuses}")
                
            except can.CanError as e:
                print(f"[PDU] CAN Error: {e}")
            except Exception as e:
                print(f"[PDU] Error: {e}")
                
            time.sleep(1.0)  # 1Hz update rate - typical for power systems


class AutomotiveCANLogger:
    """Advanced CAN logger that understands automotive message formats"""
    
    def __init__(self, bus):
        self.bus = bus
        self.message_count = {}
        self.error_count = 0
        self.running = True
        
    def decode_message(self, message):
        """Decode known automotive CAN messages"""
        msg_id = message.arbitration_id
        data = message.data
        
        try:
            if msg_id == 0x110:  # Engine RPM
                rpm, throttle = struct.unpack('>HB', data[:3])
                return f"Engine RPM: {rpm}, Throttle: {throttle}%"
            elif msg_id == 0x111:  # Engine Temperature
                temp = struct.unpack('>B', data[:1])[0]
                return f"Engine Temp: {temp}°C"
            elif msg_id == 0x120:  # Vehicle Speed
                speed = struct.unpack('>H', data[:2])[0]
                return f"Vehicle Speed: {speed} km/h"
            elif msg_id == 0x200:  # Door Status
                door_byte = data[0]
                doors = ['FL', 'FR', 'RL', 'RR']
                open_doors = [doors[i] for i in range(4) if door_byte & (1 << i)]
                return f"Doors Open: {open_doors}" if open_doors else "All Doors Closed"
            elif msg_id == 0x210:  # Window Status
                windows = [f"{pos}%" for pos in data[:4]]
                return f"Windows [FL,FR,RL,RR]: {windows}"
            elif msg_id == 0x220:  # Lighting
                light_byte = data[0]
                headlights = "ON" if light_byte & 0x01 else "OFF"
                indicators = ["OFF", "LEFT", "RIGHT"][(light_byte >> 1) & 0x03]
                return f"Headlights: {headlights}, Indicators: {indicators}"
            elif msg_id == 0x300:  # Battery Status
                voltage = struct.unpack('>H', data[:2])[0] / 10.0
                alternator = "OK" if data[2] else "FAULT"
                return f"Battery: {voltage:.1f}V, Alternator: {alternator}"
            elif msg_id == 0x310:  # Fuse Status
                blown_fuses = []
                for byte_idx, byte_val in enumerate(data[:2]):
                    for bit in range(8):
                        if not (byte_val & (1 << bit)):
                            blown_fuses.append(byte_idx * 8 + bit)
                return f"Blown Fuses: {blown_fuses}" if blown_fuses else "All Fuses OK"
            else:
                return f"Unknown message type, Raw Data: {list(data)}"
        except (struct.error, IndexError):
            return f"Decode error, Raw Data: {list(data)}"
    
    def log_messages(self):
        """Log and decode all received CAN messages"""
        print("[LOGGER] Starting automotive CAN message logger...")
        
        while self.running:
            try:
                message = self.bus.recv(timeout=1.0)
                
                if message is not None:
                    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                    
                    # Count messages per ID
                    msg_id = message.arbitration_id
                    self.message_count[msg_id] = self.message_count.get(msg_id, 0) + 1
                    
                    # Decode message
                    decoded = self.decode_message(message)
                    
                    # Determine priority level
                    if 0x100 <= msg_id <= 0x1FF:
                        priority = "CRITICAL"
                    elif 0x200 <= msg_id <= 0x2FF:
                        priority = "BODY"
                    elif 0x300 <= msg_id <= 0x3FF:
                        priority = "COMFORT"
                    else:
                        priority = "UNKNOWN"
                    
                    print(f"[LOGGER] {timestamp} - ID:0x{msg_id:03X} [{priority}] - {decoded}")
                    
            except can.CanError as e:
                self.error_count += 1
                print(f"[LOGGER] CAN Error #{self.error_count}: {e}")
            except Exception as e:
                print(f"[LOGGER] Unexpected error: {e}")


def main():
    """Main function to set up automotive CAN bus simulation"""
    print("=" * 80)
    print("Automotive CAN Bus Simulation Starting...")
    print("Simulating Engine ECU, Body Control Module, and Power Distribution Unit")
    print("=" * 80)
    
    try:
        # Create virtual CAN bus
        bus = can.interface.Bus(interface='virtual', channel='vcan0')
        print(f"[MAIN] Virtual CAN bus created: {bus}")
        
        # Create ECU instances
        engine_ecu = EngineECU(bus)
        bcm = BodyControlModule(bus)
        pdu = PowerDistributionUnit(bus)
        logger = AutomotiveCANLogger(bus)
        
        # Create and start threads
        threads = [
            threading.Thread(target=engine_ecu.simulate, daemon=True),
            threading.Thread(target=bcm.simulate, daemon=True),
            threading.Thread(target=pdu.simulate, daemon=True),
            threading.Thread(target=logger.log_messages, daemon=True)
        ]
        
        for thread in threads:
            thread.start()
        
        print("[MAIN] All ECUs and logger started")
        print("[MAIN] Press Ctrl+C to stop the simulation")
        print("-" * 80)
        
        # Keep main thread alive
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n[MAIN] Simulation stopped by user")
            
            # Stop all ECUs
            engine_ecu.running = False
            bcm.running = False
            pdu.running = False
            logger.running = False
            
    except Exception as e:
        print(f"[MAIN] Error setting up CAN bus: {e}")
        print("[MAIN] Make sure python-can is properly installed")
    
    finally:
        try:
            bus.shutdown()
            print("[MAIN] CAN bus shutdown complete")
        except:
            pass


if __name__ == "__main__":
    main()