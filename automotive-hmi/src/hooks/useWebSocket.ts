import { useState, useEffect, useRef, useCallback } from 'react'
import type { 
  VehicleState, 
  WebSocketMessage, 
  VehicleStateMessage, 
  UserCommandMessage,
  SystemStatusMessage 
} from '../types/autosar'

interface UseWebSocketReturn {
  connected: boolean
  vehicleState: VehicleState | null
  lastUpdate: number
  errors: string[]
  sendCommand: (command: Omit<UserCommandMessage, 'timestamp'>) => void
  clearErrors: () => void
  reconnect: () => void
}

export function useWebSocket(url: string): UseWebSocketReturn {
  const [connected, setConnected] = useState(false)
  const [vehicleState, setVehicleState] = useState<VehicleState | null>(null)
  const [lastUpdate, setLastUpdate] = useState(0)
  const [errors, setErrors] = useState<string[]>([])
  
  const wsRef = useRef<WebSocket | null>(null)
  const reconnectTimeoutRef = useRef<NodeJS.Timeout>()
  const reconnectAttempts = useRef(0)
  const maxReconnectAttempts = 5
  const reconnectDelay = 3000

  const addError = useCallback((error: string) => {
    setErrors(prev => [...prev.slice(-4), error]) // Keep last 5 errors
  }, [])

  const clearErrors = useCallback(() => {
    setErrors([])
  }, [])

  const connect = useCallback(() => {
    try {
      if (wsRef.current?.readyState === WebSocket.OPEN) {
        return
      }

      wsRef.current = new WebSocket(url)

      wsRef.current.onopen = () => {
        setConnected(true)
        setErrors([])
        reconnectAttempts.current = 0
        console.log('WebSocket connected to', url)
      }

      wsRef.current.onmessage = (event) => {
        try {
          const message: WebSocketMessage = JSON.parse(event.data)
          
          switch (message.type) {
            case 'VEHICLE_STATE':
              const vehicleMessage = message as VehicleStateMessage
              setVehicleState(vehicleMessage.data)
              setLastUpdate(vehicleMessage.timestamp)
              break
              
            case 'SYSTEM_STATUS':
              const statusMessage = message as SystemStatusMessage
              if (statusMessage.status === 'ERROR' && statusMessage.message) {
                addError(`System error: ${statusMessage.message}`)
              }
              break
              
            default:
              console.log('Unknown message type:', message)
          }
        } catch (error) {
          addError(`Failed to parse message: ${error}`)
        }
      }

      wsRef.current.onerror = (error) => {
        setConnected(false)
        addError(`Connection error: ${error}`)
        console.error('WebSocket error:', error)
      }

      wsRef.current.onclose = (event) => {
        setConnected(false)
        
        if (!event.wasClean && reconnectAttempts.current < maxReconnectAttempts) {
          reconnectAttempts.current++
          addError(`Connection lost. Reconnecting... (${reconnectAttempts.current}/${maxReconnectAttempts})`)
          
          reconnectTimeoutRef.current = setTimeout(() => {
            connect()
          }, reconnectDelay)
        } else if (reconnectAttempts.current >= maxReconnectAttempts) {
          addError(`Failed to reconnect after ${maxReconnectAttempts} attempts`)
        }
      }

    } catch (error) {
      addError(`Failed to connect: ${error}`)
    }
  }, [url, addError])

  const sendCommand = useCallback((command: Omit<UserCommandMessage, 'timestamp'>) => {
    if (!wsRef.current || wsRef.current.readyState !== WebSocket.OPEN) {
      addError('Cannot send command: not connected')
      return
    }

    const fullCommand: UserCommandMessage = {
      ...command,
      timestamp: Date.now()
    }

    try {
      wsRef.current.send(JSON.stringify(fullCommand))
    } catch (error) {
      addError(`Failed to send command: ${error}`)
    }
  }, [addError])

  const reconnect = useCallback(() => {
    if (reconnectTimeoutRef.current) {
      clearTimeout(reconnectTimeoutRef.current)
    }
    
    if (wsRef.current) {
      wsRef.current.close()
    }
    
    reconnectAttempts.current = 0
    connect()
  }, [connect])

  useEffect(() => {
    connect()

    return () => {
      if (reconnectTimeoutRef.current) {
        clearTimeout(reconnectTimeoutRef.current)
      }
      
      if (wsRef.current) {
        wsRef.current.close()
      }
    }
  }, [connect])

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