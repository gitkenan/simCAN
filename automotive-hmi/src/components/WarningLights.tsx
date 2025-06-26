import React from 'react'
import type { WarningLights as WarningLightsType, EngineData, DoorStatus, LightControl } from '../types/autosar'

export interface WarningLightsProps {
  warnings?: WarningLightsType
  engineData: EngineData
  doors: DoorStatus
  lights: LightControl
  gear: 'P' | 'R' | 'N' | 'D' | 'S'
}

export function WarningLights({ warnings, engineData, doors, lights, gear }: WarningLightsProps) {
  // Auto-calculate warning states based on vehicle data
  const getWarningState = (warningType: keyof WarningLightsType): boolean => {
    if (warnings && warnings[warningType] !== undefined) {
      return warnings[warningType]
    }

    // Auto-calculate based on vehicle state
    switch (warningType) {
      case 'engineCheck':
        return !engineData.running && engineData.rpm > 0
      case 'oilPressure':
        return (engineData.oilPressure || 0) < 20
      case 'fuelLow':
        return (engineData.fuelLevel || 0) < 20
      case 'doorAjar':
        return Object.values(doors).some(door => door)
      case 'turnSignalLeft':
        return lights.turnSignals === 'LEFT' || lights.turnSignals === 'HAZARD'
      case 'turnSignalRight':
        return lights.turnSignals === 'RIGHT' || lights.turnSignals === 'HAZARD'
      case 'highBeam':
        return lights.headlights === 'HIGH_BEAM'
      case 'parkingBrake':
        return gear === 'P'
      case 'seatbelt':
        return false // Simulated - off by default for predictable testing
      case 'batteryCharge':
        return false // Simulated
      case 'brakeSystem':
        return false // Simulated
      case 'absSystem':
        return false // Simulated
      default:
        return false
    }
  }

  const warningLights = [
    {
      id: 'engineCheck',
      symbol: '🔧',
      label: 'ENGINE',
      color: 'text-automotive-warning',
      description: 'Check Engine'
    },
    {
      id: 'oilPressure',
      symbol: '🛢️',
      label: 'OIL',
      color: 'text-automotive-danger',
      description: 'Oil Pressure Low'
    },
    {
      id: 'batteryCharge',
      symbol: '🔋',
      label: 'BATT',
      color: 'text-automotive-danger',
      description: 'Battery Charge'
    },
    {
      id: 'brakeSystem',
      symbol: '🛑',
      label: 'BRAKE',
      color: 'text-automotive-danger',
      description: 'Brake System'
    },
    {
      id: 'absSystem',
      symbol: 'ABS',
      label: 'ABS',
      color: 'text-automotive-warning',
      description: 'Anti-lock Braking System'
    },
    {
      id: 'seatbelt',
      symbol: '🔗',
      label: 'BELT',
      color: 'text-automotive-warning',
      description: 'Fasten Seatbelt'
    },
    {
      id: 'doorAjar',
      symbol: '🚪',
      label: 'DOOR',
      color: 'text-automotive-warning',
      description: 'Door Ajar'
    },
    {
      id: 'fuelLow',
      symbol: '⛽',
      label: 'FUEL',
      color: 'text-automotive-warning',
      description: 'Fuel Level Low'
    },
    {
      id: 'turnSignalLeft',
      symbol: '←',
      label: 'L TURN',
      color: 'text-automotive-success',
      description: 'Left Turn Signal'
    },
    {
      id: 'turnSignalRight',
      symbol: '→',
      label: 'R TURN',
      color: 'text-automotive-success',
      description: 'Right Turn Signal'
    },
    {
      id: 'highBeam',
      symbol: '💡',
      label: 'HIGH',
      color: 'text-blue-400',
      description: 'High Beam'
    },
    {
      id: 'parkingBrake',
      symbol: 'P',
      label: 'PARK',
      color: 'text-automotive-danger',
      description: 'Parking Brake'
    }
  ]

  const activeWarnings = warningLights.filter(warning => 
    getWarningState(warning.id as keyof WarningLightsType)
  )

  return (
    <div className="bg-automotive-charcoal rounded-lg p-4 border border-automotive-steel">
      <h4 className="text-sm font-display text-automotive-chrome mb-4 text-center">
        Warning Lights
      </h4>

      {/* Active Warnings - Top Priority */}
      {activeWarnings.length > 0 && (
        <div className="mb-4">
          <div className="text-xs text-automotive-warning mb-2 text-center">ACTIVE WARNINGS</div>
          <div className="grid grid-cols-3 gap-2">
            {activeWarnings.map((warning) => (
              <div
                key={warning.id}
                className={`flex flex-col items-center p-2 rounded border border-current animate-pulse ${warning.color}`}
                title={warning.description}
              >
                <div className="text-lg">{warning.symbol}</div>
                <div className="text-xs font-display">{warning.label}</div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* All Warning Lights Grid */}
      <div className="grid grid-cols-4 gap-1">
        {warningLights.map((warning) => {
          const isActive = getWarningState(warning.id as keyof WarningLightsType)
          
          return (
            <div
              key={warning.id}
              className={`flex flex-col items-center p-1 rounded border transition-all duration-200 ${
                isActive 
                  ? `${warning.color} border-current bg-current/10 animate-pulse` 
                  : 'text-automotive-steel border-automotive-steel bg-automotive-black'
              }`}
              title={warning.description}
            >
              <div className={`text-sm ${isActive ? '' : 'opacity-30'}`}>
                {warning.symbol}
              </div>
              <div className={`text-xs font-display ${isActive ? '' : 'opacity-30'}`}>
                {warning.label}
              </div>
            </div>
          )
        })}
      </div>

      {/* Status Summary */}
      <div className="mt-4 text-center">
        {activeWarnings.length === 0 ? (
          <div className="text-xs text-automotive-success">
            ✓ All Systems Normal
          </div>
        ) : (
          <div className="text-xs text-automotive-warning">
            ⚠ {activeWarnings.length} Warning{activeWarnings.length !== 1 ? 's' : ''} Active
          </div>
        )}
      </div>
    </div>
  )
}