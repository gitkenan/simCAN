"""
Basic AUTOSAR concepts for learning - simplified implementation
"""

from abc import ABC, abstractmethod
from typing import Any, Dict, List, Optional
from dataclasses import dataclass
from enum import Enum


class PortDirection(Enum):
    """Direction of data flow for ports"""
    SENDER = "sender"
    RECEIVER = "receiver"


class InterfaceType(Enum):
    """Types of AUTOSAR interfaces"""
    SENDER_RECEIVER = "sender_receiver"
    CLIENT_SERVER = "client_server"


@dataclass
class PortInterface:
    """Defines the interface for communication between SWCs"""
    name: str
    interface_type: InterfaceType
    data_elements: List[str]  # What data is exchanged
    
    def __str__(self):
        return f"Interface({self.name}: {', '.join(self.data_elements)})"


@dataclass
class Port:
    """AUTOSAR port - connection point for SWC communication"""
    name: str
    direction: PortDirection
    interface: PortInterface
    
    def __str__(self):
        return f"Port({self.name} - {self.direction.value} - {self.interface.name})"


class SoftwareComponent(ABC):
    """
    Base class for AUTOSAR Software Components (SWCs)
    
    In AUTOSAR, SWCs are the main building blocks of application software.
    They encapsulate functionality and communicate through well-defined ports.
    """
    
    def __init__(self, name: str):
        self.name = name
        self.ports: Dict[str, Port] = {}
        self.data: Dict[str, Any] = {}  # Internal data storage
        self.running = True
        
    def add_port(self, port: Port):
        """Add a port to this SWC"""
        self.ports[port.name] = port
        print(f"[{self.name}] Added {port}")
        
    def get_port(self, port_name: str) -> Optional[Port]:
        """Get a port by name"""
        return self.ports.get(port_name)
        
    def list_ports(self):
        """List all ports for debugging"""
        print(f"[{self.name}] Ports:")
        for port_name, port in self.ports.items():
            print(f"  - {port}")
            
    @abstractmethod
    def runnable_main(self):
        """
        Main runnable function - equivalent to AUTOSAR runnable entities.
        This is where the SWC's main logic executes.
        """
        pass
        
    def stop(self):
        """Stop the SWC"""
        self.running = False
        print(f"[{self.name}] Stopped")


# Common automotive interfaces for learning
ENGINE_DATA_INTERFACE = PortInterface(
    name="EngineDataInterface",
    interface_type=InterfaceType.SENDER_RECEIVER,
    data_elements=["RPM", "Temperature", "ThrottlePosition"]
)

DOOR_STATUS_INTERFACE = PortInterface(
    name="DoorStatusInterface", 
    interface_type=InterfaceType.SENDER_RECEIVER,
    data_elements=["FrontLeftOpen", "FrontRightOpen", "RearLeftOpen", "RearRightOpen"]
)

LIGHT_CONTROL_INTERFACE = PortInterface(
    name="LightControlInterface",
    interface_type=InterfaceType.SENDER_RECEIVER, 
    data_elements=["HeadlightsOn", "TurnSignalLeft", "TurnSignalRight"]
)