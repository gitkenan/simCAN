import { describe, it, expect, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { WarningLights } from './WarningLights'
import type { EngineData, DoorStatus, LightControl } from '../types/autosar'

// Clean up any global mocks
beforeEach(() => {
  vi.clearAllMocks()
})

describe('WarningLights', () => {
  const mockEngineData: EngineData = {
    rpm: 2000,
    temperature: 85,
    throttlePosition: 45,
    running: true,
    oilPressure: 60,
    fuelLevel: 75,
    speed: 50
  }

  const mockDoors: DoorStatus = {
    frontLeft: false,
    frontRight: false,
    rearLeft: false,
    rearRight: false,
    trunk: false,
    hood: false
  }

  const mockLights: LightControl = {
    headlights: 'OFF',
    turnSignals: 'OFF',
    brakelights: false,
    reverselights: false,
    interiorLights: false
  }

  it('should show all systems normal when no warnings are active', () => {
    // Use explicit warnings that are all false to ensure no warnings are shown
    const noWarnings = {
      engineCheck: false,
      oilPressure: false,
      batteryCharge: false,
      brakeSystem: false,
      absSystem: false,
      seatbelt: false,
      doorAjar: false,
      fuelLow: false,
      turnSignalLeft: false,
      turnSignalRight: false,
      highBeam: false,
      parkingBrake: false
    }
    
    render(
      <WarningLights 
        warnings={noWarnings}
        engineData={mockEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('Warning Lights')).toBeInTheDocument()
    expect(screen.getByText('✓ All Systems Normal')).toBeInTheDocument()
  })

  it('should render warning lights panel title', () => {
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('Warning Lights')).toBeInTheDocument()
  })

  it('should show oil pressure warning when oil pressure is low', () => {
    const lowOilEngineData = { ...mockEngineData, oilPressure: 15 }
    
    render(
      <WarningLights 
        engineData={lowOilEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('OIL')).toHaveLength(2) // Shows in both active warnings and grid
  })

  it('should show fuel low warning when fuel is low', () => {
    const lowFuelEngineData = { ...mockEngineData, fuelLevel: 15 }
    
    render(
      <WarningLights 
        engineData={lowFuelEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('FUEL')).toHaveLength(2)
  })

  it('should show door ajar warning when doors are open', () => {
    const openDoors = { ...mockDoors, frontLeft: true }
    
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={openDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('DOOR')).toHaveLength(2)
  })

  it('should show turn signal warnings when turn signals are active', () => {
    const leftTurnLights = { ...mockLights, turnSignals: 'LEFT' as const }
    
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={mockDoors}
        lights={leftTurnLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('L TURN')).toHaveLength(2)
  })

  it('should show high beam warning when high beams are on', () => {
    const highBeamLights = { ...mockLights, headlights: 'HIGH_BEAM' as const }
    
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={mockDoors}
        lights={highBeamLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('HIGH')).toHaveLength(2)
  })

  it('should show parking brake warning in park gear', () => {
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="P"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('PARK')).toHaveLength(2)
  })

  it('should show hazard warning when hazards are active', () => {
    const hazardLights = { ...mockLights, turnSignals: 'HAZARD' as const }
    
    render(
      <WarningLights 
        engineData={mockEngineData}
        doors={mockDoors}
        lights={hazardLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('L TURN')).toHaveLength(2)
    expect(screen.getAllByText('R TURN')).toHaveLength(2)
  })

  it('should display multiple warnings count', () => {
    const multipleWarnings = {
      engineData: { ...mockEngineData, fuelLevel: 10, oilPressure: 15 },
      doors: { ...mockDoors, frontLeft: true },
      lights: { ...mockLights, turnSignals: 'LEFT' as const },
      gear: 'P' as const
    }
    
    render(
      <WarningLights 
        engineData={multipleWarnings.engineData}
        doors={multipleWarnings.doors}
        lights={multipleWarnings.lights}
        gear={multipleWarnings.gear}
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getByText(/\d+ Warnings? Active/)).toBeInTheDocument()
  })

  it('should use explicit warnings when provided', () => {
    const explicitWarnings = {
      engineCheck: true,
      oilPressure: false,
      batteryCharge: true,
      brakeSystem: false,
      absSystem: false,
      seatbelt: false,
      doorAjar: false,
      fuelLow: false,
      turnSignalLeft: false,
      turnSignalRight: false,
      highBeam: false,
      parkingBrake: false
    }
    
    render(
      <WarningLights 
        warnings={explicitWarnings}
        engineData={mockEngineData}
        doors={mockDoors}
        lights={mockLights}
        gear="D"
      />
    )
    
    expect(screen.getByText('ACTIVE WARNINGS')).toBeInTheDocument()
    expect(screen.getAllByText('ENGINE')).toHaveLength(2)
    expect(screen.getAllByText('BATT')).toHaveLength(2)
    // Oil should not be shown in active warnings since explicitly set to false
    const oilElements = screen.queryAllByText('OIL')
    expect(oilElements).toHaveLength(1) // Only in the grid, not in active warnings
  })
})