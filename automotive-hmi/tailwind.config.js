/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        // Automotive color palette
        automotive: {
          black: '#0a0a0a',
          charcoal: '#1a1a1a',
          steel: '#2a2a2a',
          silver: '#888888',
          chrome: '#c0c0c0',
          warning: '#ff6b00',
          danger: '#dc2626',
          success: '#16a34a',
          info: '#0ea5e9',
          accent: '#3b82f6',
        },
        gauge: {
          green: '#10b981',
          yellow: '#f59e0b',
          orange: '#f97316',
          red: '#ef4444',
        }
      },
      fontFamily: {
        'automotive': ['Roboto', 'Arial', 'sans-serif'],
        'display': ['Orbitron', 'monospace'],
      },
      animation: {
        'gauge-sweep': 'gauge-sweep 2s ease-out',
        'blink': 'blink 1s infinite',
      },
      keyframes: {
        'gauge-sweep': {
          '0%': { transform: 'rotate(-135deg)' },
          '100%': { transform: 'rotate(var(--target-rotation))' },
        },
        'blink': {
          '0%, 50%': { opacity: '1' },
          '51%, 100%': { opacity: '0.3' },
        }
      }
    },
  },
  plugins: [],
}