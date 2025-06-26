import { describe, it, expect, beforeEach, vi } from 'vitest'
import { renderHook, act, waitFor } from '@testing-library/react'
import { useWebSocket } from './useWebSocket'
import type { VehicleStateMessage, UserCommandMessage } from '../types/autosar'

// Mock fetch globally
const mockFetch = vi.fn()
global.fetch = mockFetch

describe('useWebSocket (HTTP Polling)', () => {
  beforeEach(() => {
    vi.clearAllMocks()
    vi.clearAllTimers()
    vi.useFakeTimers()
    
    // Setup default successful fetch response
    mockFetch.mockResolvedValue({
      ok: true,
      json: vi.fn().mockResolvedValue({
        type: 'VEHICLE_STATE',
        timestamp: 1234567890,
        data: {
          engine: {
            rpm: 1000,
            temperature: 80,
            throttlePosition: 20,
            running: true,
            oilPressure: 60,
            fuelLevel: 75,
            speed: 20
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
              headlights: 'OFF',
              turnSignals: 'OFF',
              brakelights: false,
              reverselights: false,
              interiorLights: false
            }
          },
          ignition: 'OFF',
          gear: 'P',
          timestamp: 1234567890
        }
      })
    })
  })

  it('should initialize with disconnected state', () => {
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    expect(result.current.connected).toBe(false)
    expect(result.current.vehicleState).toBe(null)
    expect(result.current.errors).toEqual([])
  })

  it('should start polling on mount', async () => {
    renderHook(() => useWebSocket('http://localhost:8080'))
    
    // Let initial effect run
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })
    
    // Should make initial fetch call
    expect(mockFetch).toHaveBeenCalledWith('http://localhost:8080/api/vehicle', {
      method: 'GET',
      headers: {
        'Accept': 'application/json',
      },
    })
  })

  it('should fetch vehicle state and set connected', async () => {
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    // Wait for initial fetch to complete
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })

    expect(result.current.connected).toBe(true)
    expect(result.current.vehicleState).toBeTruthy()
    expect(result.current.vehicleState?.engine.rpm).toBe(1000)
  })

  it('should poll at regular intervals', async () => {
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    // Initial call
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })
    
    const initialCalls = mockFetch.mock.calls.length

    // Advance time by 1 second (polling interval) and run timers
    await act(async () => {
      vi.advanceTimersByTime(1000)
      await vi.runOnlyPendingTimersAsync()
    })

    expect(mockFetch.mock.calls.length).toBeGreaterThan(initialCalls)
  })

  it('should send commands via fetch', async () => {
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    const command = {
      type: 'USER_COMMAND' as const,
      component: 'engine' as const,
      action: 'setRPM',
      parameters: { value: 3000 }
    }

    await act(async () => {
      await result.current.sendCommand(command)
    })

    expect(mockFetch).toHaveBeenCalledWith('http://localhost:8080/api/command', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'Accept': 'application/json',
      },
      body: expect.stringContaining('"type":"USER_COMMAND"')
    })
  })

  it('should handle fetch errors', async () => {
    // Mock fetch to always fail for this test
    mockFetch.mockRejectedValue(new Error('Network error'))
    
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })

    expect(result.current.connected).toBe(false)
    expect(result.current.errors.length).toBeGreaterThan(0)
    expect(result.current.errors[0]).toContain('Network error')
  })

  it('should handle HTTP error responses', async () => {
    // Mock fetch to always return error response for this test
    mockFetch.mockResolvedValue({
      ok: false,
      status: 500,
      statusText: 'Internal Server Error'
    })
    
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })

    expect(result.current.connected).toBe(false)
    expect(result.current.errors.length).toBeGreaterThan(0)
    expect(result.current.errors[0]).toContain('HTTP 500')
  })

  it('should stop polling on unmount', () => {
    const { unmount } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    unmount()
    
    // Advance time to see if polling continues (it shouldn't)
    const initialCallCount = mockFetch.mock.calls.length
    act(() => {
      vi.advanceTimersByTime(5000)
    })
    
    expect(mockFetch.mock.calls.length).toBe(initialCallCount)
  })

  it('should clear errors', async () => {
    mockFetch.mockRejectedValue(new Error('Network error'))
    
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    // Wait for error to be added
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })

    expect(result.current.errors.length).toBeGreaterThan(0)

    // Clear errors
    act(() => {
      result.current.clearErrors()
    })

    expect(result.current.errors).toEqual([])
  })

  it('should reconnect when reconnect is called', async () => {
    const { result } = renderHook(() => useWebSocket('http://localhost:8080'))
    
    // Wait for initial connection
    await act(async () => {
      await vi.runOnlyPendingTimersAsync()
    })

    expect(result.current.connected).toBe(true)
    const initialCallCount = mockFetch.mock.calls.length

    // Call reconnect
    await act(async () => {
      result.current.reconnect()
      await vi.runOnlyPendingTimersAsync()
    })

    // Should make a new fetch call
    expect(mockFetch.mock.calls.length).toBeGreaterThan(initialCallCount)
  })
})