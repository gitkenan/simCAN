import { useState, useEffect, useRef, useCallback } from 'react'
import type { 
  VehicleState, 
  UserCommandMessage,
} from '../types/autosar'

interface UseHTTPPollingReturn {
  connected: boolean
  vehicleState: VehicleState | null
  lastUpdate: number
  errors: string[]
  sendCommand: (command: Omit<UserCommandMessage, 'timestamp'>) => void
  clearErrors: () => void
  reconnect: () => void
}

export function useWebSocket(baseUrl: string): UseHTTPPollingReturn {
  const [connected, setConnected] = useState(false)
  const [vehicleState, setVehicleState] = useState<VehicleState | null>(null)
  const [lastUpdate, setLastUpdate] = useState(0)
  const [errors, setErrors] = useState<string[]>([])
  
  const pollingIntervalRef = useRef<NodeJS.Timeout>()
  const pollInterval = 1000 // 1 second polling

  const addError = useCallback((error: string) => {
    setErrors(prev => [...prev.slice(-4), error]) // Keep last 5 errors
  }, [])

  const clearErrors = useCallback(() => {
    setErrors([])
  }, [])

  const fetchVehicleState = useCallback(async () => {
    try {
      const response = await fetch(`${baseUrl}/api/vehicle`, {
        method: 'GET',
        headers: {
          'Accept': 'application/json',
        },
      })

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`)
      }

      const data = await response.json()
      
      if (data.type === 'VEHICLE_STATE' && data.data) {
        setVehicleState(data.data)
        setLastUpdate(data.timestamp)
        setConnected(true)
      } else {
        throw new Error('Invalid response format')
      }
    } catch (error) {
      setConnected(false)
      addError(`Failed to fetch vehicle state: ${error}`)
    }
  }, [baseUrl, addError])

  const sendCommand = useCallback(async (command: Omit<UserCommandMessage, 'timestamp'>) => {
    try {
      const fullCommand: UserCommandMessage = {
        ...command,
        timestamp: Date.now()
      }

      const response = await fetch(`${baseUrl}/api/command`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json',
        },
        body: JSON.stringify(fullCommand),
      })

      if (!response.ok) {
        throw new Error(`Command failed: HTTP ${response.status}`)
      }

      const result = await response.json()
      console.log('Command response:', result)
    } catch (error) {
      addError(`Failed to send command: ${error}`)
    }
  }, [baseUrl, addError])

  const startPolling = useCallback(() => {
    // Initial fetch
    fetchVehicleState()
    
    // Set up polling
    pollingIntervalRef.current = setInterval(fetchVehicleState, pollInterval)
  }, [fetchVehicleState])

  const stopPolling = useCallback(() => {
    if (pollingIntervalRef.current) {
      clearInterval(pollingIntervalRef.current)
      pollingIntervalRef.current = undefined
    }
    setConnected(false)
  }, [])

  const reconnect = useCallback(() => {
    stopPolling()
    setErrors([])
    startPolling()
  }, [startPolling, stopPolling])

  useEffect(() => {
    startPolling()

    return () => {
      stopPolling()
    }
  }, [startPolling, stopPolling])

  return {
    connected,
    vehicleState,
    lastUpdate,
    errors,
    sendCommand,
    clearErrors,
    reconnect
  }
}