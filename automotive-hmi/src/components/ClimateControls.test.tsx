import { describe, it, expect, vi } from 'vitest'
import { render, screen, fireEvent } from '@testing-library/react'
import { ClimateControls } from './ClimateControls'

describe('ClimateControls', () => {
  const mockOnClimateChange = vi.fn()

  beforeEach(() => {
    vi.clearAllMocks()
  })

  it('should render with default climate state', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    expect(screen.getByText('Climate Control')).toBeInTheDocument()
    expect(screen.getByText('22°')).toBeInTheDocument()
    expect(screen.getByText('AUTO')).toBeInTheDocument()
  })

  it('should render with custom climate state', () => {
    const customClimate = {
      temperature: 25,
      fanSpeed: 2,
      mode: 'HEAT' as const,
      airDirection: 'FEET' as const,
      acEnabled: false,
      recirculation: true,
      rearDefrost: false
    }

    render(
      <ClimateControls 
        climate={customClimate}
        onClimateChange={mockOnClimateChange} 
      />
    )
    
    expect(screen.getByText('25°')).toBeInTheDocument()
    expect(screen.getByText('HEAT')).toBeInTheDocument()
  })

  it('should allow temperature adjustment', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click temperature up button
    const tempUpButton = screen.getByText('+')
    fireEvent.click(tempUpButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ temperature: 23 })
    )
  })

  it('should allow temperature decrease', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click temperature down button
    const tempDownButton = screen.getByText('-')
    fireEvent.click(tempDownButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ temperature: 21 })
    )
  })

  it('should respect temperature limits', () => {
    const extremeClimate = {
      temperature: 16, // At minimum
      fanSpeed: 3,
      mode: 'AUTO' as const,
      airDirection: 'FACE' as const,
      acEnabled: true,
      recirculation: false,
      rearDefrost: false
    }

    render(
      <ClimateControls 
        climate={extremeClimate}
        onClimateChange={mockOnClimateChange} 
      />
    )
    
    // Try to go below minimum
    const tempDownButton = screen.getByText('-')
    fireEvent.click(tempDownButton)
    
    // Should stay at minimum (16)
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ temperature: 16 })
    )
  })

  it('should allow mode changes', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click HEAT mode
    const heatButton = screen.getByText('HEAT')
    fireEvent.click(heatButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ mode: 'HEAT' })
    )
  })

  it('should allow fan speed changes', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click fan speed 5
    const fanSpeed5 = screen.getByText('5')
    fireEvent.click(fanSpeed5)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ fanSpeed: 5 })
    )
  })

  it('should allow air direction changes', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click FEET direction
    const feetButton = screen.getByText('FEET')
    fireEvent.click(feetButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ airDirection: 'FEET' })
    )
  })

  it('should toggle A/C', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click A/C toggle
    const acButton = screen.getByText('A/C')
    fireEvent.click(acButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ acEnabled: false }) // Was true by default
    )
  })

  it('should toggle recirculation', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click recirculation toggle
    const recircButton = screen.getByText('Recirculation')
    fireEvent.click(recircButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ recirculation: true }) // Was false by default
    )
  })

  it('should toggle rear defrost', () => {
    render(<ClimateControls onClimateChange={mockOnClimateChange} />)
    
    // Click rear defrost toggle
    const rearDefrostButton = screen.getByText('Rear Defrost')
    fireEvent.click(rearDefrostButton)
    
    expect(mockOnClimateChange).toHaveBeenCalledWith(
      expect.objectContaining({ rearDefrost: true }) // Was false by default
    )
  })
})