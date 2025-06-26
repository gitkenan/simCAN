import { describe, it, expect } from 'vitest'
import { render, screen } from '@testing-library/react'
import { InstrumentCluster } from './InstrumentCluster'
import type { EngineData } from '../types/autosar'

describe('InstrumentCluster', () => {
  const mockEngineData: EngineData = {
    rpm: 2500,
    temperature: 85,
    throttlePosition: 45,
    running: true,
    oilPressure: 60,
    fuelLevel: 75
  }

  it('should render RPM gauge with correct value', () => {
    render(
      <InstrumentCluster 
        engineData={mockEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('rpm-gauge')).toBeInTheDocument()
    expect(screen.getByText('2500')).toBeInTheDocument()
    expect(screen.getByText('RPM')).toBeInTheDocument()
  })

  it('should render temperature gauge with warning for high temp', () => {
    const hotEngineData = { ...mockEngineData, temperature: 95 }
    
    render(
      <InstrumentCluster 
        engineData={hotEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('temperature-gauge')).toBeInTheDocument()
    expect(screen.getByText('95°C')).toBeInTheDocument()
    // Should show warning color for high temperature
    const tempGauge = screen.getByTestId('temperature-gauge')
    expect(tempGauge).toHaveClass('text-automotive-warning')
  })

  it('should render throttle position indicator', () => {
    render(
      <InstrumentCluster 
        engineData={mockEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('throttle-indicator')).toBeInTheDocument()
    expect(screen.getByText('45%')).toBeInTheDocument()
  })

  it('should render gear indicator', () => {
    render(
      <InstrumentCluster 
        engineData={mockEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('gear-indicator')).toBeInTheDocument()
    expect(screen.getByText('D')).toBeInTheDocument()
  })

  it('should render ignition status', () => {
    render(
      <InstrumentCluster 
        engineData={mockEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('ignition-status')).toBeInTheDocument()
    expect(screen.getByText('ON')).toBeInTheDocument()
  })

  it('should show engine stopped state when not running', () => {
    const stoppedEngineData = { ...mockEngineData, running: false, rpm: 0 }
    
    render(
      <InstrumentCluster 
        engineData={stoppedEngineData}
        gear="P"
        ignition="OFF"
      />
    )

    expect(screen.getByText('ENGINE OFF')).toBeInTheDocument()
  })

  it('should render fuel gauge when fuel level is provided', () => {
    render(
      <InstrumentCluster 
        engineData={mockEngineData}
        gear="D"
        ignition="ON"
      />
    )

    expect(screen.getByTestId('fuel-gauge')).toBeInTheDocument()
    expect(screen.getByText('75%')).toBeInTheDocument()
  })

  it('should show low fuel warning when fuel is below 20%', () => {
    const lowFuelData = { ...mockEngineData, fuelLevel: 15 }
    
    render(
      <InstrumentCluster 
        engineData={lowFuelData}
        gear="D"
        ignition="ON"
      />
    )

    const fuelGauge = screen.getByTestId('fuel-gauge')
    expect(fuelGauge).toHaveClass('text-automotive-warning')
  })
})