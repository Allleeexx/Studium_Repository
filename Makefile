# Makefile for Kart ESC Motor Control System
# ==========================================

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
LIBS = -lwiringPi -lrt
TARGET_CPP = kart_control
SOURCE_CPP = kart_control.cpp

# Python requirements
PYTHON = python3
PIP = pip3

.PHONY: all clean install-deps install-python-deps test help

# Default target
all: $(TARGET_CPP)

# Compile C++ version
$(TARGET_CPP): $(SOURCE_CPP)
	@echo "Compiling C++ ESC controller..."
	$(CXX) $(CXXFLAGS) -o $(TARGET_CPP) $(SOURCE_CPP) $(LIBS)
	@echo "C++ compilation complete: $(TARGET_CPP)"

# Install system dependencies
install-deps:
	@echo "Installing system dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential wiringpi python3-dev python3-pip
	@echo "System dependencies installed"

# Install Python dependencies
install-python-deps:
	@echo "Installing Python dependencies..."
	$(PIP) install --user RPi.GPIO
	@echo "Python dependencies installed"

# Setup for Raspberry Pi
setup-rpi: install-deps install-python-deps
	@echo "Setting up for Raspberry Pi..."
	# Enable GPIO group permissions
	sudo usermod -a -G gpio $(USER)
	@echo "Setup complete. Please log out and log back in for group changes to take effect."

# Test compilation (without hardware dependencies)
test-compile:
	@echo "Testing compilation without hardware dependencies..."
	$(CXX) $(CXXFLAGS) -DTEST_MODE -o $(TARGET_CPP)_test $(SOURCE_CPP)
	@echo "Test compilation successful"

# Run Python version (for testing)
run-python:
	@echo "Running Python version..."
	$(PYTHON) kart.py

# Run C++ version
run-cpp: $(TARGET_CPP)
	@echo "Running C++ version..."
	sudo ./$(TARGET_CPP)

# Check Python syntax
check-python:
	@echo "Checking Python syntax..."
	$(PYTHON) -m py_compile kart.py
	@echo "Python syntax check passed"

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGET_CPP) $(TARGET_CPP)_test
	find . -name "*.pyc" -delete
	find . -name "__pycache__" -delete
	@echo "Clean complete"

# Help
help:
	@echo "Kart ESC Motor Control System - Makefile"
	@echo "========================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all              - Build C++ version (default)"
	@echo "  install-deps     - Install system dependencies"
	@echo "  install-python-deps - Install Python dependencies"
	@echo "  setup-rpi        - Complete setup for Raspberry Pi"
	@echo "  test-compile     - Test compilation without hardware deps"
	@echo "  run-python       - Run Python version"
	@echo "  run-cpp          - Run C++ version (requires sudo)"
	@echo "  check-python     - Check Python syntax"
	@echo "  clean            - Clean build artifacts"
	@echo "  help             - Show this help"
	@echo ""
	@echo "Hardware Requirements:"
	@echo "  - Raspberry Pi with GPIO access"
	@echo "  - ESC compatible with servo PWM signals"
	@echo "  - WiringPi library (for C++ version)"
	@echo "  - RPi.GPIO library (for Python version)"
	@echo ""
	@echo "Pin Configuration:"
	@echo "  - GPIO 18: Motor 1 ESC control (PWM)"
	@echo "  - GPIO 19: Motor 2 ESC control (PWM) - optional"
	@echo "  - GPIO 21: Emergency stop input (pull-up resistor)"
	@echo "  - GPIO 20: Status LED output"