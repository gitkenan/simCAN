import React, { useState, useEffect } from 'react'
import type { LightControl } from '../types/autosar'

export interface LightingControlsProps {
  lights: LightControl
  onLightChange: (lightType: keyof LightControl, value: any) => void
}

export function LightingControls({ lights, onLightChange }: LightingControlsProps) {
  const [blinking, setBlinking] = useState(false)

  // Handle turn signal blinking animation
  useEffect(() => {
    let interval: NodeJS.Timeout | null = null
    
    if (lights.turnSignals === 'LEFT' || lights.turnSignals === 'RIGHT' || lights.turnSignals === 'HAZARD') {
      interval = setInterval(() => {
        setBlinking(prev => !prev)
      }, 500) // Blink every 500ms (realistic automotive timing)
    } else {
      setBlinking(false)
    }

    return () => {
      if (interval) clearInterval(interval)
    }
  }, [lights.turnSignals])

  const getHeadlightColor = (mode: string) => {
    if (lights.headlights === mode) {
      switch (mode) {
        case 'OFF': return 'bg-automotive-steel text-automotive-silver'
        case 'PARKING': return 'bg-automotive-warning/30 text-automotive-warning border-automotive-warning'
        case 'LOW_BEAM': return 'bg-automotive-success/30 text-automotive-success border-automotive-success'
        case 'HIGH_BEAM': return 'bg-automotive-accent text-white border-automotive-accent'
        default: return 'bg-automotive-charcoal text-automotive-silver'
      }
    }
    return 'bg-automotive-charcoal text-automotive-silver hover:bg-automotive-steel'
  }

  const getTurnSignalStyle = (direction: 'LEFT' | 'RIGHT') => {
    const isActive = lights.turnSignals === direction || lights.turnSignals === 'HAZARD'
    const shouldBlink = isActive && blinking
    
    return `relative overflow-hidden transition-all duration-150 ${
      isActive 
        ? shouldBlink 
          ? 'bg-automotive-warning text-black shadow-lg shadow-automotive-warning/50' 
          : 'bg-automotive-warning/30 text-automotive-warning border-automotive-warning'
        : 'bg-automotive-charcoal text-automotive-silver hover:bg-automotive-steel'
    }`
  }

  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-6 text-center">
        Lighting System
      </h3>

      {/* Headlight Switch */}
      <div className="mb-8">
        <div className="text-center mb-4">
          <div className="text-sm text-automotive-silver mb-2">HEADLIGHT SWITCH</div>
          <div className="grid grid-cols-2 gap-2">
            {(['OFF', 'PARKING', 'LOW_BEAM', 'HIGH_BEAM'] as const).map((mode) => (
              <button
                key={mode}
                onClick={() => onLightChange('headlights', mode)}
                className={`p-3 rounded border font-display text-sm transition-all duration-200 ${getHeadlightColor(mode)}`}
              >
                <div className="flex flex-col items-center">
                  <div className="text-xs">{mode.replace('_', ' ')}</div>
                  {mode !== 'OFF' && (
                    <div className={`w-3 h-3 rounded-full mt-1 ${
                      lights.headlights === mode ? 'bg-current' : 'bg-automotive-steel'
                    }`}></div>
                  )}
                </div>
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Turn Signal Stalk */}
      <div className="mb-8">
        <div className="text-center mb-4">
          <div className="text-sm text-automotive-silver mb-2">TURN SIGNAL STALK</div>
          
          {/* Visual Turn Signal Indicator */}
          <div className="flex justify-center items-center space-x-4 mb-4">
            <div className={`w-8 h-8 rounded-full flex items-center justify-center text-xl transition-all duration-150 ${
              (lights.turnSignals === 'LEFT' || lights.turnSignals === 'HAZARD') && blinking
                ? 'bg-automotive-warning text-black shadow-lg shadow-automotive-warning/50'
                : 'bg-automotive-black border border-automotive-steel text-automotive-steel'
            }`}>
              ←
            </div>
            
            <div className="text-automotive-chrome font-display">
              {lights.turnSignals}
            </div>
            
            <div className={`w-8 h-8 rounded-full flex items-center justify-center text-xl transition-all duration-150 ${
              (lights.turnSignals === 'RIGHT' || lights.turnSignals === 'HAZARD') && blinking
                ? 'bg-automotive-warning text-black shadow-lg shadow-automotive-warning/50'
                : 'bg-automotive-black border border-automotive-steel text-automotive-steel'
            }`}>
              →
            </div>
          </div>

          {/* Turn Signal Controls */}
          <div className="grid grid-cols-2 gap-4 mb-4">
            <button
              onClick={() => onLightChange('turnSignals', lights.turnSignals === 'LEFT' ? 'OFF' : 'LEFT')}
              className={`p-4 rounded border font-display transition-all duration-200 ${getTurnSignalStyle('LEFT')}`}
            >
              <div className="text-2xl">←</div>
              <div className="text-xs">LEFT</div>
            </button>
            
            <button
              onClick={() => onLightChange('turnSignals', lights.turnSignals === 'RIGHT' ? 'OFF' : 'RIGHT')}
              className={`p-4 rounded border font-display transition-all duration-200 ${getTurnSignalStyle('RIGHT')}`}
            >
              <div className="text-2xl">→</div>
              <div className="text-xs">RIGHT</div>
            </button>
          </div>

          {/* Hazard Button */}
          <button
            onClick={() => onLightChange('turnSignals', lights.turnSignals === 'HAZARD' ? 'OFF' : 'HAZARD')}
            className={`w-full p-3 rounded border font-display transition-all duration-200 ${
              lights.turnSignals === 'HAZARD'
                ? blinking
                  ? 'bg-automotive-danger text-white shadow-lg shadow-automotive-danger/50'
                  : 'bg-automotive-danger/30 text-automotive-danger border-automotive-danger'
                : 'bg-automotive-charcoal text-automotive-silver hover:bg-automotive-steel border-automotive-steel'
            }`}
          >
            <div className="flex items-center justify-center space-x-2">
              <span className="text-lg">⚠</span>
              <span>HAZARD</span>
              <span className="text-lg">⚠</span>
            </div>
          </button>
        </div>
      </div>

      {/* Other Lights */}
      <div className="space-y-3">
        <div className="text-sm text-automotive-silver mb-2">OTHER LIGHTS</div>
        {[
          { key: 'brakelights', label: 'Brake Lights' },
          { key: 'reverselights', label: 'Reverse Lights' },
          { key: 'interiorLights', label: 'Interior Lights' }
        ].map(({ key, label }) => (
          <label key={key} className="flex items-center justify-between p-3 rounded border border-automotive-steel hover:bg-automotive-steel/30 cursor-pointer transition-all duration-200">
            <span className="text-sm text-automotive-chrome font-display">{label}</span>
            <div className="flex items-center space-x-3">
              <input
                type="checkbox"
                checked={lights[key as keyof LightControl] as boolean}
                onChange={(e) => onLightChange(key as keyof LightControl, e.target.checked)}
                className="sr-only"
              />
              <div className={`w-6 h-6 rounded border-2 transition-all duration-200 ${
                lights[key as keyof LightControl]
                  ? 'bg-automotive-success border-automotive-success'
                  : 'border-automotive-steel'
              }`}>
                {lights[key as keyof LightControl] && (
                  <div className="w-full h-full flex items-center justify-center text-white text-sm">✓</div>
                )}
              </div>
            </div>
          </label>
        ))}
      </div>
    </div>
  )
}