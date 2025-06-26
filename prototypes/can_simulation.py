#!/usr/bin/env python3
"""
CAN Bus Simulation using python-can library
Simulates a sensor sending data and a logger receiving messages
"""

import can
import time
import random
import threading
from datetime import datetime


def sensor_simulator(bus, sensor_id=0x123):
    """
    Simulate a sensor sending random data every second
    """
    print(f"[SENSOR] Starting sensor simulator with ID: 0x{sensor_id:03X}")
    
    while True:
        # Generate random sensor data (temperature, pressure, etc.)
        sensor_value = random.randint(0, 1000)
        
        # Create CAN message
        message = can.Message(
            arbitration_id=sensor_id,
            data=[
                (sensor_value >> 8) & 0xFF,  # High byte
                sensor_value & 0xFF,         # Low byte
                random.randint(0, 255),      # Additional random data
                random.randint(0, 255),
            ],
            is_extended_id=False
        )
        
        try:
            bus.send(message)
            timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
            print(f"[SENSOR] {timestamp} - Sent: ID=0x{sensor_id:03X}, Data={sensor_value}")
        except can.CanError as e:
            print(f"[SENSOR] Error sending message: {e}")
        
        time.sleep(1)


def message_logger(bus):
    """
    Log all received CAN messages
    """
    print("[LOGGER] Starting message logger...")
    
    while True:
        try:
            # Wait for a message with timeout
            message = bus.recv(timeout=1.0)
            
            if message is not None:
                timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
                
                # Extract sensor value from first two bytes
                if len(message.data) >= 2:
                    sensor_value = (message.data[0] << 8) | message.data[1]
                    print(f"[LOGGER] {timestamp} - Received: ID=0x{message.arbitration_id:03X}, "
                          f"Value={sensor_value}, Raw Data={list(message.data)}")
                else:
                    print(f"[LOGGER] {timestamp} - Received: ID=0x{message.arbitration_id:03X}, "
                          f"Raw Data={list(message.data)}")
                    
        except can.CanError as e:
            print(f"[LOGGER] Error receiving message: {e}")
        except Exception as e:
            print(f"[LOGGER] Unexpected error: {e}")


def main():
    """
    Main function to set up CAN bus and start simulation
    """
    print("=" * 60)
    print("CAN Bus Simulation Starting...")
    print("=" * 60)
    
    try:
        # Create virtual CAN bus
        bus = can.interface.Bus(interface='virtual', channel='vcan0')
        print(f"[MAIN] Virtual CAN bus created: {bus}")
        
        # Create and start threads
        sensor_thread = threading.Thread(target=sensor_simulator, args=(bus,), daemon=True)
        logger_thread = threading.Thread(target=message_logger, args=(bus,), daemon=True)
        
        # Start both threads
        sensor_thread.start()
        logger_thread.start()
        
        print("[MAIN] Both sensor and logger threads started")
        print("[MAIN] Press Ctrl+C to stop the simulation")
        print("-" * 60)
        
        # Keep main thread alive
        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\n[MAIN] Simulation stopped by user")
            
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