# Makefile for Automotive CAN Simulation

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -pthread
TARGET = automotive_can_sim
SOURCE = can_simulation_cpp.cpp

# Default target
all: $(TARGET)

# Build the C++ simulation
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)
	@echo "Build complete! Run with: ./$(TARGET)"

# Clean build artifacts
clean:
	rm -f $(TARGET)
	@echo "Clean complete!"

# Install dependencies (for Python version)
install-deps:
	pip3 install python-can
	@echo "Python dependencies installed!"

# Run Python version
run-python:
	python3 automotive_can_simulation.py

# Run C++ version
run-cpp: $(TARGET)
	./$(TARGET)

# Run original simple version
run-simple:
	python3 can_simulation.py

.PHONY: all clean install-deps run-python run-cpp run-simple