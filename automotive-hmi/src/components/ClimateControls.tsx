import React, { useState } from 'react'

export interface ClimateState {
  temperature: number
  fanSpeed: number
  mode: 'OFF' | 'HEAT' | 'COOL' | 'AUTO'
  airDirection: 'FACE' | 'FEET' | 'DEFROST' | 'MIX'
  acEnabled: boolean
  recirculation: boolean
  rearDefrost: boolean
}

export interface ClimateControlsProps {
  climate?: ClimateState
  onClimateChange?: (climate: ClimateState) => void
}

export function ClimateControls({ 
  climate = {
    temperature: 22,
    fanSpeed: 3,
    mode: 'AUTO',
    airDirection: 'FACE',
    acEnabled: true,
    recirculation: false,
    rearDefrost: false
  },
  onClimateChange 
}: ClimateControlsProps) {
  const [localClimate, setLocalClimate] = useState<ClimateState>(climate)

  const updateClimate = (updates: Partial<ClimateState>) => {
    const newClimate = { ...localClimate, ...updates }
    setLocalClimate(newClimate)
    onClimateChange?.(newClimate)
  }

  const getModeColor = (mode: string) => {
    if (localClimate.mode === mode) {
      switch (mode) {
        case 'OFF': return 'bg-automotive-steel text-automotive-silver'
        case 'HEAT': return 'bg-automotive-danger/30 text-automotive-danger border-automotive-danger'
        case 'COOL': return 'bg-blue-500/30 text-blue-400 border-blue-400'
        case 'AUTO': return 'bg-automotive-success/30 text-automotive-success border-automotive-success'
        default: return 'bg-automotive-charcoal text-automotive-silver'
      }
    }
    return 'bg-automotive-charcoal text-automotive-silver hover:bg-automotive-steel'
  }

  const getDirectionIcon = (direction: string) => {
    switch (direction) {
      case 'FACE': return '👤'
      case 'FEET': return '🦶'
      case 'DEFROST': return '❄️'
      case 'MIX': return '🔄'
      default: return '?'
    }
  }

  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h3 className="text-lg font-display text-automotive-chrome mb-6 text-center">
        Climate Control
      </h3>

      {/* Temperature Control */}
      <div className="mb-6">
        <div className="text-center mb-4">
          <div className="text-sm text-automotive-silver mb-2">TEMPERATURE</div>
          <div className="flex items-center justify-center space-x-4">
            <button
              onClick={() => updateClimate({ temperature: Math.max(16, localClimate.temperature - 1) })}
              className="w-10 h-10 rounded-full bg-blue-500/30 text-blue-400 border border-blue-400 hover:bg-blue-500/50 transition-all duration-200"
            >
              -
            </button>
            
            <div className="text-center">
              <div className="text-3xl font-display text-automotive-chrome">
                {localClimate.temperature}°
              </div>
              <div className="text-xs text-automotive-silver">CELSIUS</div>
            </div>
            
            <button
              onClick={() => updateClimate({ temperature: Math.min(30, localClimate.temperature + 1) })}
              className="w-10 h-10 rounded-full bg-automotive-danger/30 text-automotive-danger border border-automotive-danger hover:bg-automotive-danger/50 transition-all duration-200"
            >
              +
            </button>
          </div>
        </div>
      </div>

      {/* Mode Selection */}
      <div className="mb-6">
        <div className="text-center mb-2">
          <div className="text-sm text-automotive-silver mb-2">MODE</div>
          <div className="grid grid-cols-4 gap-2">
            {(['OFF', 'HEAT', 'COOL', 'AUTO'] as const).map((mode) => (
              <button
                key={mode}
                onClick={() => updateClimate({ mode })}
                className={`p-2 rounded border font-display text-xs transition-all duration-200 ${getModeColor(mode)}`}
              >
                {mode}
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Fan Speed */}
      <div className="mb-6">
        <div className="text-center mb-2">
          <div className="text-sm text-automotive-silver mb-2">FAN SPEED</div>
          <div className="flex justify-center items-center space-x-2">
            {[1, 2, 3, 4, 5].map((speed) => (
              <button
                key={speed}
                onClick={() => updateClimate({ fanSpeed: speed })}
                className={`w-8 h-8 rounded border transition-all duration-200 ${
                  localClimate.fanSpeed >= speed
                    ? 'bg-automotive-accent text-white border-automotive-accent'
                    : 'bg-automotive-charcoal text-automotive-silver border-automotive-steel hover:bg-automotive-steel'
                }`}
              >
                {speed}
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Air Direction */}
      <div className="mb-6">
        <div className="text-center mb-2">
          <div className="text-sm text-automotive-silver mb-2">AIR DIRECTION</div>
          <div className="grid grid-cols-4 gap-2">
            {(['FACE', 'FEET', 'DEFROST', 'MIX'] as const).map((direction) => (
              <button
                key={direction}
                onClick={() => updateClimate({ airDirection: direction })}
                className={`p-3 rounded border font-display text-xs transition-all duration-200 ${
                  localClimate.airDirection === direction
                    ? 'bg-automotive-accent text-white border-automotive-accent'
                    : 'bg-automotive-charcoal text-automotive-silver border-automotive-steel hover:bg-automotive-steel'
                }`}
              >
                <div className="text-lg mb-1">{getDirectionIcon(direction)}</div>
                <div>{direction}</div>
              </button>
            ))}
          </div>
        </div>
      </div>

      {/* Toggle Controls */}
      <div className="space-y-3">
        {[
          { key: 'acEnabled', label: 'A/C', icon: '❄️' },
          { key: 'recirculation', label: 'Recirculation', icon: '🔄' },
          { key: 'rearDefrost', label: 'Rear Defrost', icon: '🚗' }
        ].map(({ key, label, icon }) => (
          <button
            key={key}
            onClick={() => updateClimate({ [key]: !localClimate[key as keyof ClimateState] })}
            className={`w-full flex items-center justify-between p-3 rounded border font-display transition-all duration-200 ${
              localClimate[key as keyof ClimateState]
                ? 'bg-automotive-success/30 text-automotive-success border-automotive-success'
                : 'bg-automotive-charcoal text-automotive-silver border-automotive-steel hover:bg-automotive-steel'
            }`}
          >
            <div className="flex items-center space-x-3">
              <span className="text-lg">{icon}</span>
              <span className="text-sm">{label}</span>
            </div>
            <div className={`w-6 h-6 rounded border-2 transition-all duration-200 ${
              localClimate[key as keyof ClimateState]
                ? 'bg-automotive-success border-automotive-success'
                : 'border-automotive-steel'
            }`}>
              {localClimate[key as keyof ClimateState] && (
                <div className="w-full h-full flex items-center justify-center text-white text-sm">✓</div>
              )}
            </div>
          </button>
        ))}
      </div>
    </div>
  )
}