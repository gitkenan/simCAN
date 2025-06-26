import { describe, it, expect, beforeEach, vi } from 'vitest'
import { renderHook, act } from '@testing-library/react'
import { useWebSocket } from './useWebSocket'
import type { VehicleStateMessage, UserCommandMessage } from '../types/autosar'

describe('useWebSocket', () => {
  let mockWebSocketInstance: any
  let mockWebSocketConstructor: any

  beforeEach(() => {
    mockWebSocketInstance = {
      send: vi.fn(),
      close: vi.fn(),
      readyState: WebSocket.OPEN,
      addEventListener: vi.fn(),
      removeEventListener: vi.fn(),
      onopen: null,
      onclose: null,
      onmessage: null,
      onerror: null,
    }

    mockWebSocketConstructor = vi.fn(() => mockWebSocketInstance)
    global.WebSocket = mockWebSocketConstructor as any
  })

  it('should initialize with disconnected state', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    expect(result.current.connected).toBe(false)
    expect(result.current.vehicleState).toBe(null)
    expect(result.current.errors).toEqual([])
  })

  it('should connect to WebSocket on mount', () => {
    renderHook(() => useWebSocket('ws://localhost:8080'))
    
    expect(mockWebSocketConstructor).toHaveBeenCalledWith('ws://localhost:8080')
  })

  it('should handle connection open', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    act(() => {
      mockWebSocketInstance.onopen?.(new Event('open'))
    })

    expect(result.current.connected).toBe(true)
  })

  it('should handle vehicle state messages', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    // First connect
    act(() => {
      mockWebSocketInstance.onopen?.(new Event('open'))
    })

    const mockVehicleState: VehicleStateMessage = {
      type: 'VEHICLE_STATE',
      timestamp: Date.now(),
      data: {
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
    }

    act(() => {
      mockWebSocketInstance.onmessage?.({ 
        data: JSON.stringify(mockVehicleState) 
      } as MessageEvent)
    })

    expect(result.current.vehicleState).toEqual(mockVehicleState.data)
    expect(result.current.lastUpdate).toBe(mockVehicleState.timestamp)
  })

  it('should send user commands when connected', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    // First connect
    act(() => {
      mockWebSocketInstance.onopen?.(new Event('open'))
    })

    const command = {
      type: 'USER_COMMAND' as const,
      component: 'engine' as const,
      action: 'setRPM',
      parameters: { value: 3000 }
    }

    act(() => {
      result.current.sendCommand(command)
    })

    expect(mockWebSocketInstance.send).toHaveBeenCalledWith(
      expect.stringContaining('"type":"USER_COMMAND"')
    )
    expect(mockWebSocketInstance.send).toHaveBeenCalledWith(
      expect.stringContaining('"component":"engine"')
    )
  })

  it('should handle connection errors', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    act(() => {
      mockWebSocketInstance.onerror?.(new Event('error'))
    })

    expect(result.current.connected).toBe(false)
    expect(result.current.errors.length).toBeGreaterThan(0)
  })

  it('should not send commands when disconnected', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    const command = {
      type: 'USER_COMMAND' as const,
      component: 'engine' as const,
      action: 'setRPM',
      parameters: { value: 3000 }
    }

    act(() => {
      result.current.sendCommand(command)
    })

    expect(mockWebSocketInstance.send).not.toHaveBeenCalled()
    expect(result.current.errors).toContain(
      expect.stringContaining('Cannot send command: not connected')
    )
  })

  it('should cleanup on unmount', () => {
    const { unmount } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    unmount()
    
    expect(mockWebSocketInstance.close).toHaveBeenCalled()
  })

  it('should clear errors', () => {
    const { result } = renderHook(() => useWebSocket('ws://localhost:8080'))
    
    // Add an error
    act(() => {
      mockWebSocketInstance.onerror?.(new Event('error'))
    })

    expect(result.current.errors.length).toBeGreaterThan(0)

    // Clear errors
    act(() => {
      result.current.clearErrors()
    })

    expect(result.current.errors).toEqual([])
  })
})