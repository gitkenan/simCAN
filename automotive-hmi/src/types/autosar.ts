// AUTOSAR data type definitions for frontend
export interface EngineData {
  rpm: number
  temperature: number
  throttlePosition: number
  running: boolean
  oilPressure?: number
  fuelLevel?: number
  speed?: number  // Vehicle speed in km/h
}

export interface DoorStatus {
  frontLeft: boolean
  frontRight: boolean
  rearLeft: boolean
  rearRight: boolean
  trunk: boolean
  hood: boolean
}

export interface WindowPosition {
  frontLeft: number  // 0-100 percentage
  frontRight: number
  rearLeft: number
  rearRight: number
  sunroof?: number
}

export interface LightControl {
  headlights: 'OFF' | 'PARKING' | 'LOW_BEAM' | 'HIGH_BEAM'
  turnSignals: 'OFF' | 'LEFT' | 'RIGHT' | 'HAZARD'
  brakelights: boolean
  reverselights: boolean
  interiorLights: boolean
}

export interface WarningLights {
  engineCheck: boolean
  oilPressure: boolean
  batteryCharge: boolean
  brakeSystem: boolean
  absSystem: boolean
  seatbelt: boolean
  doorAjar: boolean
  fuelLow: boolean
  turnSignalLeft: boolean
  turnSignalRight: boolean
  highBeam: boolean
  parkingBrake: boolean
}

export interface VehicleState {
  engine: EngineData
  body: {
    doors: DoorStatus
    windows: WindowPosition
    lights: LightControl
  }
  ignition: 'OFF' | 'ACC' | 'ON' | 'START'
  gear: 'P' | 'R' | 'N' | 'D' | 'S'
  warnings?: WarningLights
  timestamp: number
}

// WebSocket message types
export interface VehicleStateMessage {
  type: 'VEHICLE_STATE'
  timestamp: number
  data: VehicleState
}

export interface UserCommandMessage {
  type: 'USER_COMMAND'
  component: 'engine' | 'body' | 'ignition' | 'transmission'
  action: string
  parameters: Record<string, any>
  timestamp: number
}

export interface SystemStatusMessage {
  type: 'SYSTEM_STATUS'
  status: 'CONNECTED' | 'DISCONNECTED' | 'ERROR'
  message?: string
  timestamp: number
}

export type WebSocketMessage = VehicleStateMessage | UserCommandMessage | SystemStatusMessage

// Component state types
export interface DashboardState {
  connected: boolean
  vehicleState: VehicleState | null
  lastUpdate: number
  errors: string[]
}

// Gauge value ranges
export const GAUGE_RANGES = {
  RPM: { min: 0, max: 8000, redline: 6500 },
  TEMPERATURE: { min: 0, max: 120, warning: 90, danger: 105 },
  THROTTLE: { min: 0, max: 100 },
  SPEED: { min: 0, max: 200 },
  FUEL: { min: 0, max: 100, warning: 20, reserve: 10 },
  OIL_PRESSURE: { min: 0, max: 100, warning: 20 }
} as const

// Color mappings for different states
export const STATE_COLORS = {
  NORMAL: '#10b981',
  WARNING: '#f59e0b',
  DANGER: '#ef4444',
  OFF: '#6b7280',
  ACTIVE: '#3b82f6'
} as const