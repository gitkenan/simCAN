import React from 'react'
import { useWebSocket } from '../hooks/useWebSocket'
import { InstrumentCluster } from './InstrumentCluster'
import { CenterConsole } from './CenterConsole'
import { DoorPanel } from './DoorPanel'
import { LightingControls } from './LightingControls'
import { ClimateControls } from './ClimateControls'
import { WarningLights } from './WarningLights'

export interface VehicleDashboardProps {
  apiBaseUrl?: string
}

export function VehicleDashboard({ apiBaseUrl = 'http://localhost:8080' }: VehicleDashboardProps) {
  const {
    connected,
    vehicleState,
    errors,
    sendCommand,
    clearErrors,
    reconnect
  } = useWebSocket(apiBaseUrl)

  if (!connected) {
    return (
      <div className="min-h-screen bg-automotive-black flex items-center justify-center">
        <div className="text-center p-8">
          <div className="warning-light danger mb-4 mx-auto"></div>
          <h1 className="text-2xl font-display text-automotive-chrome mb-4">
            Disconnected
          </h1>
          <p className="text-automotive-silver mb-6">
            Vehicle communication system offline
          </p>
          <button 
            onClick={reconnect}
            className="automotive-button"
          >
            Reconnect
          </button>
        </div>
      </div>
    )
  }

  if (!vehicleState) {
    return (
      <div className="min-h-screen bg-automotive-black flex items-center justify-center">
        <div className="text-center p-8">
          <div className="warning-light on mb-4 mx-auto animate-blink"></div>
          <h1 className="text-2xl font-display text-automotive-chrome mb-4">
            Waiting for vehicle data...
          </h1>
          <p className="text-automotive-silver">
            Establishing communication with vehicle systems
          </p>
        </div>
      </div>
    )
  }

  return (
    <div className="min-h-screen bg-automotive-black text-automotive-chrome font-automotive">
      {/* Error Display */}
      {errors.length > 0 && (
        <div className="fixed top-4 right-4 z-50 space-y-2">
          {errors.map((error, index) => (
            <div 
              key={index}
              className="bg-automotive-danger/20 border border-automotive-danger p-3 rounded-md text-sm"
            >
              {error}
            </div>
          ))}
          <button 
            onClick={clearErrors}
            className="automotive-button text-xs w-full"
          >
            Clear Errors
          </button>
        </div>
      )}

      {/* Main Dashboard Layout */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6 p-6 min-h-screen">
        {/* Left Panel - Instrument Cluster */}
        <div className="lg:col-span-1" data-testid="instrument-cluster">
          <InstrumentCluster 
            engineData={vehicleState.engine}
            gear={vehicleState.gear}
            ignition={vehicleState.ignition}
          />
        </div>

        {/* Center Panel - Main Controls */}
        <div className="lg:col-span-1 space-y-6">
          <div data-testid="center-console">
            <CenterConsole 
              ignition={vehicleState.ignition}
              gear={vehicleState.gear}
              onIgnitionChange={(state) => 
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'ignition',
                  action: 'setState',
                  parameters: { state }
                })
              }
              onGearChange={(gear) =>
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'transmission',
                  action: 'setGear',
                  parameters: { gear }
                })
              }
            />
          </div>

          <div data-testid="lighting-controls">
            <LightingControls 
              lights={vehicleState.body.lights}
              onLightChange={(lightType, value) =>
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'body',
                  action: 'setLight',
                  parameters: { lightType, value }
                })
              }
            />
          </div>

          <div data-testid="climate-controls">
            <ClimateControls 
              onClimateChange={(climate) =>
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'climate',
                  action: 'setClimate',
                  parameters: climate
                })
              }
            />
          </div>
        </div>

        {/* Right Panel - Body Controls */}
        <div className="lg:col-span-1 space-y-6">
          <div data-testid="door-panel">
            <DoorPanel 
              doors={vehicleState.body.doors}
              windows={vehicleState.body.windows}
              onDoorToggle={(door) =>
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'body',
                  action: 'toggleDoor',
                  parameters: { door }
                })
              }
              onWindowChange={(window, position) =>
                sendCommand({
                  type: 'USER_COMMAND',
                  component: 'body',
                  action: 'setWindow',
                  parameters: { window, position }
                })
              }
            />
          </div>

          <div data-testid="warning-lights">
            <WarningLights 
              warnings={vehicleState.warnings}
              engineData={vehicleState.engine}
              doors={vehicleState.body.doors}
              lights={vehicleState.body.lights}
              gear={vehicleState.gear}
            />
          </div>
        </div>
      </div>

      {/* Status Bar */}
      <div className="fixed bottom-0 left-0 right-0 bg-automotive-charcoal border-t border-automotive-steel p-2">
        <div className="flex justify-between items-center text-xs text-automotive-silver">
          <div className="flex items-center space-x-4">
            <div className="flex items-center space-x-2">
              <div className="warning-light success"></div>
              <span>Connected</span>
            </div>
            <div>
              Ignition: {vehicleState.ignition}
            </div>
            <div>
              Gear: {vehicleState.gear}
            </div>
          </div>
          <div>
            {new Date(vehicleState.timestamp).toLocaleTimeString()}
          </div>
        </div>
      </div>
    </div>
  )
}