import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen } from '@testing-library/react'
import { VehicleDashboard } from './VehicleDashboard'
import type { VehicleState } from '../types/autosar'

// Mock the useWebSocket hook
vi.mock('../hooks/useWebSocket', () => ({
  useWebSocket: vi.fn()
}))

// Get the mocked function
import { useWebSocket } from '../hooks/useWebSocket'
const mockUseWebSocket = vi.mocked(useWebSocket)

beforeEach(() => {
  // Reset the mock before each test
  mockUseWebSocket.mockReset()
})

describe('VehicleDashboard', () => {
  it('should render connection status when disconnected', () => {
    mockUseWebSocket.mockReturnValue({
      connected: false,
      vehicleState: null,
      lastUpdate: 0,
      errors: [],
      sendCommand: vi.fn(),
      clearErrors: vi.fn(),
      reconnect: vi.fn(),
    })

    render(<VehicleDashboard />)
    
    expect(screen.getByText(/disconnected/i)).toBeInTheDocument()
  })

  it('should render main dashboard components when connected', () => {
    const mockVehicleState: VehicleState = {
      engine: {
        rpm: 2500,
        temperature: 85,
        throttlePosition: 45,
        running: true
      },
      body: {
        doors: {
          frontLeft: false,
          frontRight: false,
          rearLeft: false,
          rearRight: false,
          trunk: false,
          hood: false
        },
        windows: {
          frontLeft: 100,
          frontRight: 100,
          rearLeft: 100,
          rearRight: 100
        },
        lights: {
          headlights: 'LOW_BEAM',
          turnSignals: 'OFF',
          brakelights: false,
          reverselights: false,
          interiorLights: false
        }
      },
      ignition: 'ON',
      gear: 'D',
      timestamp: Date.now()
    }

    mockUseWebSocket.mockReturnValue({
      connected: true,
      vehicleState: mockVehicleState,
      lastUpdate: Date.now(),
      errors: [],
      sendCommand: vi.fn(),
      clearErrors: vi.fn(),
      reconnect: vi.fn(),
    })

    render(<VehicleDashboard />)
    
    // Check for main dashboard sections
    expect(screen.getByTestId('instrument-cluster')).toBeInTheDocument()
    expect(screen.getByTestId('center-console')).toBeInTheDocument()
    expect(screen.getByTestId('door-panel')).toBeInTheDocument()
    expect(screen.getByTestId('lighting-controls')).toBeInTheDocument()
  })

  it('should display connection errors when present', () => {
    const mockVehicleState: VehicleState = {
      engine: {
        rpm: 2500,
        temperature: 85,
        throttlePosition: 45,
        running: true
      },
      body: {
        doors: {
          frontLeft: false,
          frontRight: false,
          rearLeft: false,
          rearRight: false,
          trunk: false,
          hood: false
        },
        windows: {
          frontLeft: 100,
          frontRight: 100,
          rearLeft: 100,
          rearRight: 100
        },
        lights: {
          headlights: 'LOW_BEAM',
          turnSignals: 'OFF',
          brakelights: false,
          reverselights: false,
          interiorLights: false
        }
      },
      ignition: 'ON',
      gear: 'D',
      timestamp: Date.now()
    }

    mockUseWebSocket.mockReturnValue({
      connected: true,
      vehicleState: mockVehicleState,
      lastUpdate: Date.now(),
      errors: ['Connection timeout', 'Failed to send command'],
      sendCommand: vi.fn(),
      clearErrors: vi.fn(),
      reconnect: vi.fn(),
    })

    render(<VehicleDashboard />)
    
    expect(screen.getByText('Connection timeout')).toBeInTheDocument()
    expect(screen.getByText('Failed to send command')).toBeInTheDocument()
  })

  it('should show loading state when vehicleState is null but connected', () => {
    mockUseWebSocket.mockReturnValue({
      connected: true,
      vehicleState: null,
      lastUpdate: 0,
      errors: [],
      sendCommand: vi.fn(),
      clearErrors: vi.fn(),
      reconnect: vi.fn(),
    })

    render(<VehicleDashboard />)
    
    expect(screen.getByText(/waiting for vehicle data/i)).toBeInTheDocument()
  })
})