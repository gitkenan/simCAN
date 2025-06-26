import React, { useState } from 'react'
import type { DoorStatus, WindowPosition } from '../types/autosar'

export interface DoorPanelProps {
  doors: DoorStatus
  windows: WindowPosition
  onDoorToggle: (door: keyof DoorStatus) => void
  onWindowChange: (window: keyof WindowPosition, position: number) => void
}

export function DoorPanel({ doors, windows, onDoorToggle, onWindowChange }: DoorPanelProps) {
  const [selectedControl, setSelectedControl] = useState<'doors' | 'windows'>('doors')

  const getDoorColor = (isOpen: boolean) => {
    return isOpen 
      ? 'text-automotive-warning border-automotive-warning bg-automotive-warning/20' 
      : 'text-automotive-success border-automotive-success bg-automotive-success/10'
  }

  const getWindowHeight = (position: number) => {
    return `${Math.max(10, position)}%`
  }

  // Vehicle diagram component
  const VehicleDiagram = () => (
    <div className="relative w-40 h-56 mx-auto mb-6">
      {/* Vehicle Body */}
      <div className="absolute inset-0 border-2 border-automotive-steel rounded-lg bg-automotive-black">
        {/* Front */}
        <div className="absolute top-0 left-1/2 transform -translate-x-1/2 w-8 h-2 bg-automotive-steel rounded-b"></div>
        
        {/* Doors */}
        {/* Front Left Door */}
        <button
          onClick={() => onDoorToggle('frontLeft')}
          className={`absolute top-6 left-0 w-6 h-16 border-l-2 border-t-2 border-b-2 rounded-l transition-all duration-200 ${getDoorColor(doors.frontLeft)}`}
          style={{ transform: doors.frontLeft ? 'translateX(-8px)' : 'translateX(0)' }}
          title="Front Left Door"
        >
          <div className="text-xs mt-6 transform -rotate-90">FL</div>
        </button>

        {/* Front Right Door */}
        <button
          onClick={() => onDoorToggle('frontRight')}
          className={`absolute top-6 right-0 w-6 h-16 border-r-2 border-t-2 border-b-2 rounded-r transition-all duration-200 ${getDoorColor(doors.frontRight)}`}
          style={{ transform: doors.frontRight ? 'translateX(8px)' : 'translateX(0)' }}
          title="Front Right Door"
        >
          <div className="text-xs mt-6 transform rotate-90">FR</div>
        </button>

        {/* Rear Left Door */}
        <button
          onClick={() => onDoorToggle('rearLeft')}
          className={`absolute bottom-6 left-0 w-6 h-16 border-l-2 border-t-2 border-b-2 rounded-l transition-all duration-200 ${getDoorColor(doors.rearLeft)}`}
          style={{ transform: doors.rearLeft ? 'translateX(-8px)' : 'translateX(0)' }}
          title="Rear Left Door"
        >
          <div className="text-xs mt-6 transform -rotate-90">RL</div>
        </button>

        {/* Rear Right Door */}
        <button
          onClick={() => onDoorToggle('rearRight')}
          className={`absolute bottom-6 right-0 w-6 h-16 border-r-2 border-t-2 border-b-2 rounded-r transition-all duration-200 ${getDoorColor(doors.rearRight)}`}
          style={{ transform: doors.rearRight ? 'translateX(8px)' : 'translateX(0)' }}
          title="Rear Right Door"
        >
          <div className="text-xs mt-6 transform rotate-90">RR</div>
        </button>

        {/* Trunk */}
        <button
          onClick={() => onDoorToggle('trunk')}
          className={`absolute bottom-0 left-1/2 transform -translate-x-1/2 w-8 h-4 border-2 rounded-t transition-all duration-200 ${getDoorColor(doors.trunk)}`}
          style={{ transform: doors.trunk ? 'translateY(6px) translateX(-50%)' : 'translateY(0) translateX(-50%)' }}
          title="Trunk"
        >
          <div className="text-xs">T</div>
        </button>

        {/* Hood */}
        <button
          onClick={() => onDoorToggle('hood')}
          className={`absolute top-0 left-1/2 transform -translate-x-1/2 w-8 h-4 border-2 rounded-b transition-all duration-200 ${getDoorColor(doors.hood)}`}
          style={{ transform: doors.hood ? 'translateY(-6px) translateX(-50%)' : 'translateY(0) translateX(-50%)' }}
          title="Hood"
        >
          <div className="text-xs">H</div>
        </button>

        {/* Windows (if selected) */}
        {selectedControl === 'windows' && (
          <>
            {/* Front Left Window */}
            <div className="absolute top-8 left-2 w-3 h-12 bg-automotive-black border border-automotive-steel">
              <div 
                className="absolute bottom-0 w-full bg-blue-400/30 transition-all duration-300"
                style={{ height: getWindowHeight(windows.frontLeft) }}
              ></div>
            </div>

            {/* Front Right Window */}
            <div className="absolute top-8 right-2 w-3 h-12 bg-automotive-black border border-automotive-steel">
              <div 
                className="absolute bottom-0 w-full bg-blue-400/30 transition-all duration-300"
                style={{ height: getWindowHeight(windows.frontRight) }}
              ></div>
            </div>

            {/* Rear Left Window */}
            <div className="absolute bottom-8 left-2 w-3 h-12 bg-automotive-black border border-automotive-steel">
              <div 
                className="absolute bottom-0 w-full bg-blue-400/30 transition-all duration-300"
                style={{ height: getWindowHeight(windows.rearLeft) }}
              ></div>
            </div>

            {/* Rear Right Window */}
            <div className="absolute bottom-8 right-2 w-3 h-12 bg-automotive-black border border-automotive-steel">
              <div 
                className="absolute bottom-0 w-full bg-blue-400/30 transition-all duration-300"
                style={{ height: getWindowHeight(windows.rearRight) }}
              ></div>
            </div>
          </>
        )}
      </div>
    </div>
  )

  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-6 text-center">
        Vehicle Body
      </h3>

      {/* Control Type Selector */}
      <div className="flex mb-6 bg-automotive-black rounded-lg p-1">
        <button
          onClick={() => setSelectedControl('doors')}
          className={`flex-1 py-2 px-4 rounded-md font-display text-sm transition-all duration-200 ${
            selectedControl === 'doors'
              ? 'bg-automotive-accent text-white'
              : 'text-automotive-silver hover:bg-automotive-steel'
          }`}
        >
          🚪 Doors
        </button>
        <button
          onClick={() => setSelectedControl('windows')}
          className={`flex-1 py-2 px-4 rounded-md font-display text-sm transition-all duration-200 ${
            selectedControl === 'windows'
              ? 'bg-automotive-accent text-white'
              : 'text-automotive-silver hover:bg-automotive-steel'
          }`}
        >
          🪟 Windows
        </button>
      </div>

      {/* Vehicle Diagram */}
      <VehicleDiagram />

      {/* Door Controls */}
      {selectedControl === 'doors' && (
        <div className="space-y-4">
          <div className="text-center mb-4">
            <div className="text-sm text-automotive-silver">DOOR STATUS</div>
            <div className="text-xs text-automotive-silver mt-1">Tap doors on diagram to toggle</div>
          </div>

          {/* Door Status Grid */}
          <div className="grid grid-cols-2 gap-3">
            {Object.entries(doors).map(([door, isOpen]) => (
              <div
                key={door}
                className={`p-3 rounded border transition-all duration-200 ${getDoorColor(isOpen)}`}
              >
                <div className="text-center">
                  <div className="text-sm font-display">
                    {door === 'frontLeft' && 'Front Left'}
                    {door === 'frontRight' && 'Front Right'}
                    {door === 'rearLeft' && 'Rear Left'}
                    {door === 'rearRight' && 'Rear Right'}
                    {door === 'trunk' && 'Trunk'}
                    {door === 'hood' && 'Hood'}
                  </div>
                  <div className="text-xs mt-1">
                    {isOpen ? '🔓 OPEN' : '🔒 CLOSED'}
                  </div>
                </div>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Window Controls */}
      {selectedControl === 'windows' && (
        <div className="space-y-4">
          <div className="text-center mb-4">
            <div className="text-sm text-automotive-silver">WINDOW POSITION</div>
            <div className="text-xs text-automotive-silver mt-1">0% = Closed, 100% = Fully Open</div>
          </div>

          {Object.entries(windows).map(([window, position]) => (
            <div key={window} className="space-y-2">
              <div className="flex justify-between items-center">
                <span className="text-sm text-automotive-chrome font-display">
                  {window === 'frontLeft' && 'Front Left'}
                  {window === 'frontRight' && 'Front Right'}
                  {window === 'rearLeft' && 'Rear Left'}
                  {window === 'rearRight' && 'Rear Right'}
                </span>
                <span className="text-sm text-automotive-accent font-display">
                  {position}%
                </span>
              </div>
              
              <div className="flex items-center space-x-3">
                <button
                  onClick={() => onWindowChange(window as keyof WindowPosition, 0)}
                  className="w-8 h-8 rounded bg-automotive-steel text-automotive-chrome hover:bg-automotive-accent transition-colors duration-200 text-xs"
                >
                  ▼
                </button>
                
                <div className="flex-1 relative">
                  <input
                    type="range"
                    min="0"
                    max="100"
                    value={position}
                    onChange={(e) => onWindowChange(window as keyof WindowPosition, parseInt(e.target.value))}
                    className="w-full h-3 bg-automotive-steel rounded-lg appearance-none cursor-pointer slider"
                    style={{
                      background: `linear-gradient(to right, #3b82f6 0%, #3b82f6 ${position}%, #4a5568 ${position}%, #4a5568 100%)`
                    }}
                  />
                  <div className="absolute top-4 left-0 right-0 flex justify-between text-xs text-automotive-silver">
                    <span>Closed</span>
                    <span>Open</span>
                  </div>
                </div>
                
                <button
                  onClick={() => onWindowChange(window as keyof WindowPosition, 100)}
                  className="w-8 h-8 rounded bg-automotive-steel text-automotive-chrome hover:bg-automotive-accent transition-colors duration-200 text-xs"
                >
                  ▲
                </button>
              </div>
            </div>
          ))}
        </div>
      )}
    </div>
  )
}