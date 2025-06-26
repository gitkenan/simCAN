import React from 'react'

export interface CenterConsoleProps {
  ignition: 'OFF' | 'ACC' | 'ON' | 'START'
  gear: 'P' | 'R' | 'N' | 'D' | 'S'
  onIgnitionChange: (state: 'OFF' | 'ACC' | 'ON' | 'START') => void
  onGearChange: (gear: 'P' | 'R' | 'N' | 'D' | 'S') => void
}

export function CenterConsole({ ignition, gear, onIgnitionChange, onGearChange }: CenterConsoleProps) {
  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-4">
        Center Console
      </h3>

      {/* Ignition Control */}
      <div className="mb-6">
        <label className="block text-sm text-automotive-silver mb-2">
          Ignition
        </label>
        <div className="grid grid-cols-4 gap-2">
          {(['OFF', 'ACC', 'ON', 'START'] as const).map((state) => (
            <button
              key={state}
              onClick={() => onIgnitionChange(state)}
              className={`automotive-button text-sm ${
                ignition === state ? 'bg-automotive-accent text-white' : ''
              }`}
            >
              {state}
            </button>
          ))}
        </div>
      </div>

      {/* Gear Selector */}
      <div>
        <label className="block text-sm text-automotive-silver mb-2">
          Gear Selector
        </label>
        <div className="grid grid-cols-5 gap-2">
          {(['P', 'R', 'N', 'D', 'S'] as const).map((gearOption) => (
            <button
              key={gearOption}
              onClick={() => onGearChange(gearOption)}
              className={`automotive-button ${
                gear === gearOption ? 'bg-automotive-accent text-white' : ''
              }`}
            >
              {gearOption}
            </button>
          ))}
        </div>
      </div>
    </div>
  )
}