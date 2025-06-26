"""
Software Components (SWCs) package for AUTOSAR learning
"""

from .autosar_base import (
    SoftwareComponent, Port, PortDirection, InterfaceType, PortInterface,
    ENGINE_DATA_INTERFACE, DOOR_STATUS_INTERFACE, LIGHT_CONTROL_INTERFACE
)
from .engine_swc import EngineSWC
from .body_swc import BodyControlSWC

__all__ = [
    'SoftwareComponent', 'Port', 'PortDirection', 'InterfaceType', 'PortInterface',
    'ENGINE_DATA_INTERFACE', 'DOOR_STATUS_INTERFACE', 'LIGHT_CONTROL_INTERFACE',
    'EngineSWC', 'BodyControlSWC'
]