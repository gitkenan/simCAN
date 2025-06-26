import React from 'react'
import type { LightControl } from '../types/autosar'

export interface LightingControlsProps {
  lights: LightControl
  onLightChange: (lightType: keyof LightControl, value: any) => void
}

export function LightingControls({ lights, onLightChange }: LightingControlsProps) {
  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-4">
        Lighting Controls
      </h3>

      {/* Headlights */}
      <div className="mb-4">
        <label className="block text-sm text-automotive-silver mb-2">
          Headlights
        </label>
        <div className="grid grid-cols-2 gap-2">
          {(['OFF', 'PARKING', 'LOW_BEAM', 'HIGH_BEAM'] as const).map((mode) => (
            <button
              key={mode}
              onClick={() => onLightChange('headlights', mode)}
              className={`automotive-button text-xs ${
                lights.headlights === mode ? 'bg-automotive-accent text-white' : ''
              }`}
            >
              {mode.replace('_', ' ')}
            </button>
          ))}
        </div>
      </div>

      {/* Turn Signals */}
      <div className="mb-4">
        <label className="block text-sm text-automotive-silver mb-2">
          Turn Signals
        </label>
        <div className="grid grid-cols-4 gap-2">
          {(['OFF', 'LEFT', 'RIGHT', 'HAZARD'] as const).map((signal) => (
            <button
              key={signal}
              onClick={() => onLightChange('turnSignals', signal)}
              className={`automotive-button text-xs ${
                lights.turnSignals === signal ? 'bg-automotive-accent text-white' : ''
              }`}
            >
              {signal}
            </button>
          ))}
        </div>
      </div>

      {/* Other Lights */}
      <div className="space-y-2">
        {['brakelights', 'reverselights', 'interiorLights'].map((lightType) => (
          <label key={lightType} className="flex items-center space-x-3 cursor-pointer">
            <input
              type="checkbox"
              checked={lights[lightType as keyof LightControl] as boolean}
              onChange={(e) => onLightChange(lightType as keyof LightControl, e.target.checked)}
              className="sr-only"
            />
            <div className={`w-4 h-4 rounded border border-automotive-steel ${
              lights[lightType as keyof LightControl] 
                ? 'bg-automotive-accent' 
                : 'bg-automotive-charcoal'
            }`}></div>
            <span className="text-sm text-automotive-chrome">
              {lightType.replace(/([A-Z])/g, ' $1').trim()}
            </span>
          </label>
        ))}
      </div>
    </div>
  )
}