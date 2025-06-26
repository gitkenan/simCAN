import React, { useState } from 'react'

export interface CenterConsoleProps {
  ignition: 'OFF' | 'ACC' | 'ON' | 'START'
  gear: 'P' | 'R' | 'N' | 'D' | 'S'
  onIgnitionChange: (state: 'OFF' | 'ACC' | 'ON' | 'START') => void
  onGearChange: (gear: 'P' | 'R' | 'N' | 'D' | 'S') => void
}

export function CenterConsole({ ignition, gear, onIgnitionChange, onGearChange }: CenterConsoleProps) {
  const [startHeld, setStartHeld] = useState(false)

  const getIgnitionColor = (state: 'OFF' | 'ACC' | 'ON' | 'START') => {
    if (ignition === state) {
      switch (state) {
        case 'OFF': return 'text-automotive-silver bg-automotive-steel'
        case 'ACC': return 'text-automotive-warning bg-automotive-warning/20'
        case 'ON': return 'text-automotive-success bg-automotive-success/20'
        case 'START': return 'text-white bg-automotive-accent animate-pulse'
      }
    }
    return 'text-automotive-silver bg-automotive-charcoal hover:bg-automotive-steel'
  }

  const getGearColor = (gearOption: 'P' | 'R' | 'N' | 'D' | 'S') => {
    if (gear === gearOption) {
      switch (gearOption) {
        case 'P': return 'text-white bg-automotive-success'
        case 'R': return 'text-white bg-automotive-warning'
        case 'N': return 'text-white bg-automotive-steel'
        case 'D': case 'S': return 'text-white bg-automotive-accent'
      }
    }
    return 'text-automotive-silver bg-automotive-charcoal hover:bg-automotive-steel'
  }

  const handleIgnitionChange = (state: 'OFF' | 'ACC' | 'ON' | 'START') => {
    if (state === 'START') {
      // START is momentary - automatically return to ON after brief period
      setStartHeld(true)
      onIgnitionChange('START')
      setTimeout(() => {
        onIgnitionChange('ON')
        setStartHeld(false)
      }, 1000)
    } else {
      onIgnitionChange(state)
    }
  }

  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-6 text-center">
        Vehicle Control
      </h3>

      {/* Ignition Switch - Rotary Style */}
      <div className="mb-8">
        <div className="text-center mb-4">
          <div className="text-sm text-automotive-silver mb-2">IGNITION SWITCH</div>
          <div className="relative inline-block">
            {/* Ignition positions in circular arrangement */}
            <div className="relative w-32 h-32 bg-automotive-black rounded-full border-2 border-automotive-steel">
              {/* Center indicator */}
              <div className="absolute top-1/2 left-1/2 transform -translate-x-1/2 -translate-y-1/2 w-4 h-4 bg-automotive-accent rounded-full"></div>
              
              {/* Position indicators */}
              {[
                { pos: 'OFF', angle: 45, color: getIgnitionColor('OFF') },
                { pos: 'ACC', angle: 135, color: getIgnitionColor('ACC') },
                { pos: 'ON', angle: 225, color: getIgnitionColor('ON') },
                { pos: 'START', angle: 315, color: getIgnitionColor('START') }
              ].map(({ pos, angle, color }) => (
                <button
                  key={pos}
                  onClick={() => handleIgnitionChange(pos as any)}
                  className={`absolute transform -translate-x-1/2 -translate-y-1/2 w-12 h-6 text-xs font-display rounded border border-automotive-steel transition-all duration-200 ${color}`}
                  style={{
                    left: `${50 + 35 * Math.cos((angle - 90) * Math.PI / 180)}%`,
                    top: `${50 + 35 * Math.sin((angle - 90) * Math.PI / 180)}%`,
                  }}
                  disabled={pos === 'START' && startHeld}
                >
                  {pos}
                </button>
              ))}
            </div>
          </div>
        </div>
        
        {/* Current Status */}
        <div className="text-center">
          <div className={`text-lg font-display ${
            ignition === 'ON' ? 'text-automotive-success' : 
            ignition === 'START' ? 'text-automotive-accent' :
            ignition === 'ACC' ? 'text-automotive-warning' : 
            'text-automotive-silver'
          }`}>
            {ignition}
          </div>
          <div className="text-xs text-automotive-silver">CURRENT POSITION</div>
        </div>
      </div>

      {/* PRNDL Gear Selector */}
      <div>
        <div className="text-center mb-4">
          <div className="text-sm text-automotive-silver mb-2">GEAR SELECTOR</div>
          <div className="inline-flex bg-automotive-black rounded-lg border border-automotive-steel p-1">
            {(['P', 'R', 'N', 'D', 'S'] as const).map((gearOption) => (
              <button
                key={gearOption}
                onClick={() => onGearChange(gearOption)}
                className={`w-12 h-12 rounded-md font-display font-bold text-lg transition-all duration-200 ${getGearColor(gearOption)}`}
                disabled={ignition === 'OFF'}
              >
                {gearOption}
              </button>
            ))}
          </div>
        </div>
        
        {/* Gear descriptions */}
        <div className="text-center">
          <div className="text-xs text-automotive-silver">
            {gear === 'P' && 'PARK'}
            {gear === 'R' && 'REVERSE'}
            {gear === 'N' && 'NEUTRAL'}
            {gear === 'D' && 'DRIVE'}
            {gear === 'S' && 'SPORT'}
          </div>
        </div>
      </div>

      {/* Status Indicators */}
      <div className="mt-6 grid grid-cols-2 gap-4">
        <div className={`text-center p-2 rounded border ${
          ignition !== 'OFF' ? 'border-automotive-success text-automotive-success' : 'border-automotive-steel text-automotive-silver'
        }`}>
          <div className="text-xs">READY</div>
          <div className={`warning-light ${ignition !== 'OFF' ? 'success' : 'off'} mx-auto mt-1`}></div>
        </div>
        
        <div className={`text-center p-2 rounded border ${
          gear !== 'P' && ignition !== 'OFF' ? 'border-automotive-warning text-automotive-warning' : 'border-automotive-steel text-automotive-silver'
        }`}>
          <div className="text-xs">BRAKE</div>
          <div className={`warning-light ${gear !== 'P' && ignition !== 'OFF' ? 'warning' : 'off'} mx-auto mt-1`}></div>
        </div>
      </div>
    </div>
  )
}