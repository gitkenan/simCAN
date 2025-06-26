import React from 'react'
import type { DoorStatus, WindowPosition } from '../types/autosar'

export interface DoorPanelProps {
  doors: DoorStatus
  windows: WindowPosition
  onDoorToggle: (door: keyof DoorStatus) => void
  onWindowChange: (window: keyof WindowPosition, position: number) => void
}

export function DoorPanel({ doors, windows, onDoorToggle, onWindowChange }: DoorPanelProps) {
  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-4">
        Body Controls
      </h3>

      {/* Door Controls */}
      <div className="mb-6">
        <label className="block text-sm text-automotive-silver mb-2">
          Door Locks
        </label>
        <div className="grid grid-cols-2 gap-2">
          {Object.entries(doors).map(([door, isOpen]) => (
            <button
              key={door}
              onClick={() => onDoorToggle(door as keyof DoorStatus)}
              className={`automotive-button text-sm ${
                isOpen ? 'bg-automotive-warning text-black' : ''
              }`}
            >
              {door.replace(/([A-Z])/g, ' $1').trim()}: {isOpen ? 'Open' : 'Closed'}
            </button>
          ))}
        </div>
      </div>

      {/* Window Controls */}
      <div>
        <label className="block text-sm text-automotive-silver mb-2">
          Windows
        </label>
        <div className="space-y-3">
          {Object.entries(windows).map(([window, position]) => (
            <div key={window} className="flex items-center space-x-3">
              <span className="text-sm text-automotive-silver w-20">
                {window.replace(/([A-Z])/g, ' $1').trim()}:
              </span>
              <input
                type="range"
                min="0"
                max="100"
                value={position}
                onChange={(e) => onWindowChange(window as keyof WindowPosition, parseInt(e.target.value))}
                className="flex-1 h-2 bg-automotive-steel rounded-lg appearance-none cursor-pointer"
              />
              <span className="text-sm text-automotive-chrome w-12 text-right">
                {position}%
              </span>
            </div>
          ))}
        </div>
      </div>
    </div>
  )
}