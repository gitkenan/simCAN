import React from 'react'
import type { EngineData } from '../types/autosar'
import { GAUGE_RANGES, STATE_COLORS } from '../types/autosar'

export interface InstrumentClusterProps {
  engineData: EngineData
  gear: 'P' | 'R' | 'N' | 'D' | 'S'
  ignition: 'OFF' | 'ACC' | 'ON' | 'START'
}

export function InstrumentCluster({ engineData, gear, ignition }: InstrumentClusterProps) {
  const getRPMColor = (rpm: number) => {
    if (rpm >= GAUGE_RANGES.RPM.redline) return STATE_COLORS.DANGER
    if (rpm >= GAUGE_RANGES.RPM.redline * 0.8) return STATE_COLORS.WARNING
    return STATE_COLORS.NORMAL
  }

  const getTemperatureColor = (temp: number) => {
    if (temp >= GAUGE_RANGES.TEMPERATURE.danger) return STATE_COLORS.DANGER
    if (temp >= GAUGE_RANGES.TEMPERATURE.warning) return STATE_COLORS.WARNING
    return STATE_COLORS.NORMAL
  }

  const getFuelColor = (fuel?: number) => {
    if (!fuel) return STATE_COLORS.OFF
    if (fuel <= GAUGE_RANGES.FUEL.reserve) return STATE_COLORS.DANGER
    if (fuel <= GAUGE_RANGES.FUEL.warning) return STATE_COLORS.WARNING
    return STATE_COLORS.NORMAL
  }

  const getSpeedColor = (speed?: number) => {
    if (!speed) return STATE_COLORS.OFF
    if (speed >= GAUGE_RANGES.SPEED.max * 0.9) return STATE_COLORS.WARNING
    return STATE_COLORS.NORMAL
  }

  const calculateGaugeRotation = (value: number, min: number, max: number) => {
    const percentage = Math.min(Math.max((value - min) / (max - min), 0), 1)
    return -135 + (percentage * 270) // -135deg to +135deg sweep
  }

  // Gauge component for reusable circular gauges
  const CircularGauge = ({ 
    value, 
    max, 
    min = 0, 
    label, 
    unit, 
    color,
    size = 'w-32 h-32'
  }: {
    value: number
    max: number
    min?: number
    label: string
    unit: string
    color: string
    size?: string
  }) => (
    <div className="gauge-container mx-auto relative mb-4">
      <svg
        viewBox="0 0 200 200" 
        className={`${size} transform -rotate-45`}
      >
        {/* Background arc */}
        <circle
          cx="100"
          cy="100"
          r="85"
          fill="none"
          stroke="#2a2a2a"
          strokeWidth="8"
          strokeDasharray="267"
          strokeDashoffset="67"
        />
        {/* Value arc */}
        <circle
          cx="100"
          cy="100"
          r="85"
          fill="none"
          stroke={color}
          strokeWidth="8"
          strokeDasharray="267"
          strokeDashoffset={267 - ((value - min) / (max - min)) * 200}
          style={{ transition: 'stroke-dashoffset 0.5s ease' }}
        />
      </svg>
      <div className="absolute inset-0 flex flex-col items-center justify-center">
        <div 
          className="text-2xl font-display font-bold"
          style={{ color }}
        >
          {value}
        </div>
        <div className="text-xs text-automotive-silver">{unit}</div>
      </div>
    </div>
  )

  return (
    <div className="bg-automotive-charcoal rounded-lg p-6 border border-automotive-steel">
      <h2 className="text-xl font-display text-automotive-chrome mb-6 text-center">
        Instrument Cluster
      </h2>

      {/* Engine Status */}
      {!engineData.running && (
        <div className="text-center mb-6">
          <div className="warning-light danger mx-auto mb-2"></div>
          <div className="text-automotive-danger font-display text-lg">
            ENGINE OFF
          </div>
        </div>
      )}

      {/* Main Gauges - Professional Automotive Layout */}
      <div className="flex justify-center space-x-8 mb-8">
        {/* Speedometer */}
        <div className="text-center" data-testid="speedometer">
          <CircularGauge
            value={engineData.speed || 0}
            max={GAUGE_RANGES.SPEED.max}
            min={GAUGE_RANGES.SPEED.min}
            label="Speed"
            unit="KM/H"
            color={getSpeedColor(engineData.speed)}
            size="w-40 h-40"
          />
          <div className="text-sm font-display text-automotive-chrome mt-2">
            SPEED
          </div>
        </div>

        {/* RPM Gauge */}
        <div className="text-center" data-testid="rpm-gauge">
          <CircularGauge
            value={engineData.rpm}
            max={GAUGE_RANGES.RPM.max}
            min={GAUGE_RANGES.RPM.min}
            label="RPM"
            unit="RPM"
            color={getRPMColor(engineData.rpm)}
            size="w-40 h-40"
          />
          <div className="text-sm font-display text-automotive-chrome mt-2">
            TACHOMETER
          </div>
        </div>
      </div>

      {/* Secondary Gauges */}
      <div className="grid grid-cols-4 gap-4 mb-6">
        {/* Temperature */}
        <div 
          className={`text-center p-3 rounded border border-automotive-steel ${
            engineData.temperature >= GAUGE_RANGES.TEMPERATURE.warning 
              ? 'text-automotive-warning' 
              : 'text-automotive-chrome'
          }`}
          data-testid="temperature-gauge"
        >
          <div className="text-lg font-display">
            {engineData.temperature}°C
          </div>
          <div className="text-xs text-automotive-silver">COOLANT</div>
        </div>

        {/* Fuel Level */}
        <div 
          className={`text-center p-3 rounded border border-automotive-steel ${
            (engineData.fuelLevel !== undefined && engineData.fuelLevel <= GAUGE_RANGES.FUEL.warning)
              ? 'text-automotive-warning'
              : 'text-automotive-chrome'
          }`}
          data-testid="fuel-gauge"
        >
          <div className="text-lg font-display">
            {engineData.fuelLevel || 0}%
          </div>
          <div className="text-xs text-automotive-silver">FUEL</div>
        </div>

        {/* Oil Pressure */}
        <div 
          className={`text-center p-3 rounded border border-automotive-steel ${
            (engineData.oilPressure !== undefined && engineData.oilPressure <= GAUGE_RANGES.OIL_PRESSURE.warning)
              ? 'text-automotive-warning'
              : 'text-automotive-chrome'
          }`}
          data-testid="oil-pressure-gauge"
        >
          <div className="text-lg font-display">
            {engineData.oilPressure || 0} PSI
          </div>
          <div className="text-xs text-automotive-silver">OIL</div>
        </div>

        {/* Throttle Position */}
        <div 
          className="text-center p-3 rounded border border-automotive-steel text-automotive-chrome"
          data-testid="throttle-indicator"
        >
          <div className="text-lg font-display">
            {engineData.throttlePosition}%
          </div>
          <div className="text-xs text-automotive-silver">THROTTLE</div>
        </div>
      </div>

      {/* Status Indicators */}
      <div className="grid grid-cols-2 gap-4">
        {/* Gear Indicator */}
        <div 
          className="text-center p-3 rounded bg-automotive-steel"
          data-testid="gear-indicator"
        >
          <div className="text-2xl font-display text-automotive-accent">
            {gear}
          </div>
          <div className="text-xs text-automotive-silver">GEAR</div>
        </div>

        {/* Ignition Status */}
        <div 
          className={`text-center p-3 rounded ${
            ignition === 'ON' ? 'bg-automotive-success/20' : 'bg-automotive-steel'
          }`}
          data-testid="ignition-status"
        >
          <div className={`text-lg font-display ${
            ignition === 'ON' ? 'text-automotive-success' : 'text-automotive-silver'
          }`}>
            {ignition}
          </div>
          <div className="text-xs text-automotive-silver">IGNITION</div>
        </div>
      </div>
    </div>
  )
}