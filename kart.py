#!/usr/bin/env python3
"""
ESC Motor Control System for Kart
==================================

This module provides comprehensive control for Electronic Speed Controllers (ESC)
used in kart motor control systems. It includes:

- PWM signal generation for ESC control
- Multi-motor parallel control
- Safety features and error handling
- Calibration and initialization procedures
- Real-time monitoring and logging

Hardware Requirements:
- Raspberry Pi with GPIO pins
- ESC compatible with servo PWM signals (typically 1-2ms pulse width, 50Hz)
- Proper power supply for motors and ESC

Pin Configuration:
- GPIO 18: Motor 1 ESC control (PWM)
- GPIO 19: Motor 2 ESC control (PWM) - optional for dual motor setup
- GPIO 21: Emergency stop input (pull-up resistor)
- GPIO 20: Status LED output

Author: Created for kart motor control system
"""

import RPi.GPIO as GPIO
import time
import threading
import signal
import sys
import logging
from typing import List, Optional, Tuple
from dataclasses import dataclass
from concurrent.futures import ThreadPoolExecutor
import queue

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/tmp/kart_motor.log'),
        logging.StreamHandler()
    ]
)
logger = logging.getLogger(__name__)

@dataclass
class MotorConfig:
    """Configuration for a single motor/ESC"""
    pin: int
    name: str
    min_pulse_width: float = 1.0  # ms
    max_pulse_width: float = 2.0  # ms
    neutral_pulse_width: float = 1.5  # ms
    frequency: int = 50  # Hz
    
@dataclass
class SafetyLimits:
    """Safety limits for motor operation"""
    max_acceleration_rate: float = 0.1  # Maximum change per update cycle
    max_speed: float = 100.0  # Maximum speed percentage
    emergency_stop_timeout: float = 0.1  # seconds
    watchdog_timeout: float = 2.0  # seconds

class ESCController:
    """
    Enhanced ESC Controller with safety features and parallel processing
    """
    
    def __init__(self, motors: List[MotorConfig], safety_limits: SafetyLimits):
        """
        Initialize ESC controller
        
        Args:
            motors: List of motor configurations
            safety_limits: Safety limits configuration
        """
        self.motors = motors
        self.safety_limits = safety_limits
        self.pwm_objects = {}
        self.current_speeds = {motor.name: 0.0 for motor in motors}
        self.target_speeds = {motor.name: 0.0 for motor in motors}
        self.is_running = False
        self.emergency_stop = False
        self.last_heartbeat = time.time()
        
        # Thread management
        self.control_thread = None
        self.monitor_thread = None
        self.command_queue = queue.Queue()
        self.executor = ThreadPoolExecutor(max_workers=4)
        
        # GPIO pins
        self.emergency_pin = 21
        self.status_led_pin = 20
        
        # Setup signal handlers for graceful shutdown
        signal.signal(signal.SIGINT, self._signal_handler)
        signal.signal(signal.SIGTERM, self._signal_handler)
        
        logger.info("ESC Controller initialized")
    
    def initialize(self) -> bool:
        """
        Initialize GPIO and PWM objects
        
        Returns:
            bool: True if initialization successful
        """
        try:
            # Setup GPIO
            GPIO.setmode(GPIO.BCM)
            GPIO.setwarnings(False)
            
            # Initialize motor pins
            for motor in self.motors:
                GPIO.setup(motor.pin, GPIO.OUT)
                pwm = GPIO.PWM(motor.pin, motor.frequency)
                pwm.start(self._speed_to_duty_cycle(motor, 0))
                self.pwm_objects[motor.name] = pwm
                logger.info(f"Initialized motor {motor.name} on pin {motor.pin}")
            
            # Setup emergency stop pin (with pull-up)
            GPIO.setup(self.emergency_pin, GPIO.IN, pull_up_down=GPIO.PUD_UP)
            GPIO.add_event_detect(self.emergency_pin, GPIO.FALLING, 
                                callback=self._emergency_stop_callback, bouncetime=50)
            
            # Setup status LED
            GPIO.setup(self.status_led_pin, GPIO.OUT)
            GPIO.output(self.status_led_pin, GPIO.LOW)
            
            logger.info("GPIO initialization complete")
            return True
            
        except Exception as e:
            logger.error(f"GPIO initialization failed: {e}")
            return False
    
    def calibrate_escs(self) -> bool:
        """
        Calibrate ESCs by sending max, then min, then neutral signals
        
        Returns:
            bool: True if calibration successful
        """
        try:
            logger.info("Starting ESC calibration...")
            
            # Send maximum signal
            for motor in self.motors:
                duty_cycle = self._speed_to_duty_cycle(motor, 100)
                self.pwm_objects[motor.name].ChangeDutyCycle(duty_cycle)
            
            logger.info("Sending maximum signal for 3 seconds...")
            time.sleep(3)
            
            # Send minimum signal
            for motor in self.motors:
                duty_cycle = self._speed_to_duty_cycle(motor, -100)
                self.pwm_objects[motor.name].ChangeDutyCycle(duty_cycle)
            
            logger.info("Sending minimum signal for 3 seconds...")
            time.sleep(3)
            
            # Send neutral signal
            for motor in self.motors:
                duty_cycle = self._speed_to_duty_cycle(motor, 0)
                self.pwm_objects[motor.name].ChangeDutyCycle(duty_cycle)
            
            logger.info("ESC calibration complete")
            return True
            
        except Exception as e:
            logger.error(f"ESC calibration failed: {e}")
            return False
    
    def start(self) -> bool:
        """
        Start the motor control system
        
        Returns:
            bool: True if started successfully
        """
        if not self.initialize():
            return False
        
        try:
            self.is_running = True
            self.emergency_stop = False
            
            # Start control thread
            self.control_thread = threading.Thread(target=self._control_loop, daemon=True)
            self.control_thread.start()
            
            # Start monitoring thread
            self.monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
            self.monitor_thread.start()
            
            # Indicate system is running
            GPIO.output(self.status_led_pin, GPIO.HIGH)
            
            logger.info("Motor control system started")
            return True
            
        except Exception as e:
            logger.error(f"Failed to start motor control system: {e}")
            return False
    
    def stop(self):
        """Stop the motor control system safely"""
        logger.info("Stopping motor control system...")
        
        self.is_running = False
        
        # Stop all motors immediately
        self.emergency_stop_all()
        
        # Wait for threads to finish
        if self.control_thread and self.control_thread.is_alive():
            self.control_thread.join(timeout=1.0)
        
        if self.monitor_thread and self.monitor_thread.is_alive():
            self.monitor_thread.join(timeout=1.0)
        
        # Shutdown executor
        self.executor.shutdown(wait=True)
        
        # Cleanup GPIO
        self._cleanup_gpio()
        
        logger.info("Motor control system stopped")
    
    def set_motor_speed(self, motor_name: str, speed: float, immediate: bool = False) -> bool:
        """
        Set target speed for a motor
        
        Args:
            motor_name: Name of the motor
            speed: Speed percentage (-100 to 100)
            immediate: If True, bypass acceleration limits
            
        Returns:
            bool: True if command accepted
        """
        if self.emergency_stop:
            logger.warning("Emergency stop active - command rejected")
            return False
        
        if motor_name not in self.target_speeds:
            logger.error(f"Unknown motor: {motor_name}")
            return False
        
        # Clamp speed to safety limits
        speed = max(-self.safety_limits.max_speed, 
                   min(self.safety_limits.max_speed, speed))
        
        try:
            command = {
                'type': 'set_speed',
                'motor': motor_name,
                'speed': speed,
                'immediate': immediate,
                'timestamp': time.time()
            }
            self.command_queue.put(command, timeout=0.1)
            
            # Update heartbeat
            self.last_heartbeat = time.time()
            
            logger.debug(f"Speed command queued: {motor_name} -> {speed}%")
            return True
            
        except queue.Full:
            logger.warning("Command queue full - command dropped")
            return False
    
    def set_all_motors_speed(self, speed: float, immediate: bool = False) -> bool:
        """
        Set speed for all motors simultaneously
        
        Args:
            speed: Speed percentage (-100 to 100)
            immediate: If True, bypass acceleration limits
            
        Returns:
            bool: True if all commands accepted
        """
        success = True
        for motor in self.motors:
            if not self.set_motor_speed(motor.name, speed, immediate):
                success = False
        return success
    
    def emergency_stop_all(self):
        """Emergency stop all motors immediately"""
        logger.warning("EMERGENCY STOP ACTIVATED")
        self.emergency_stop = True
        
        # Immediately set all motors to neutral
        for motor in self.motors:
            try:
                duty_cycle = self._speed_to_duty_cycle(motor, 0)
                self.pwm_objects[motor.name].ChangeDutyCycle(duty_cycle)
                self.current_speeds[motor.name] = 0.0
                self.target_speeds[motor.name] = 0.0
            except Exception as e:
                logger.error(f"Error during emergency stop for {motor.name}: {e}")
        
        # Flash status LED
        self._flash_status_led()
    
    def reset_emergency_stop(self) -> bool:
        """
        Reset emergency stop condition
        
        Returns:
            bool: True if reset successful
        """
        if not GPIO.input(self.emergency_pin):
            logger.warning("Cannot reset emergency stop - hardware switch still active")
            return False
        
        self.emergency_stop = False
        self.last_heartbeat = time.time()
        GPIO.output(self.status_led_pin, GPIO.HIGH)
        
        logger.info("Emergency stop reset")
        return True
    
    def get_motor_status(self) -> dict:
        """
        Get current status of all motors
        
        Returns:
            dict: Motor status information
        """
        return {
            'motors': {
                motor.name: {
                    'current_speed': self.current_speeds[motor.name],
                    'target_speed': self.target_speeds[motor.name],
                    'pin': motor.pin
                } for motor in self.motors
            },
            'system': {
                'running': self.is_running,
                'emergency_stop': self.emergency_stop,
                'last_heartbeat': self.last_heartbeat
            }
        }
    
    def _control_loop(self):
        """Main control loop running in separate thread"""
        logger.info("Control loop started")
        
        while self.is_running:
            try:
                # Process commands from queue
                self._process_commands()
                
                # Update motor speeds with acceleration limiting
                self._update_motor_speeds()
                
                # Check for watchdog timeout
                self._check_watchdog()
                
                time.sleep(0.02)  # 50Hz control loop
                
            except Exception as e:
                logger.error(f"Error in control loop: {e}")
    
    def _monitor_loop(self):
        """Monitoring loop for system health"""
        logger.info("Monitor loop started")
        
        while self.is_running:
            try:
                # Log system status periodically
                if int(time.time()) % 10 == 0:
                    status = self.get_motor_status()
                    logger.info(f"System status: {status}")
                
                time.sleep(1.0)
                
            except Exception as e:
                logger.error(f"Error in monitor loop: {e}")
    
    def _process_commands(self):
        """Process commands from the command queue"""
        try:
            while not self.command_queue.empty():
                command = self.command_queue.get_nowait()
                
                if command['type'] == 'set_speed':
                    motor_name = command['motor']
                    speed = command['speed']
                    immediate = command['immediate']
                    
                    if immediate:
                        self.current_speeds[motor_name] = speed
                    
                    self.target_speeds[motor_name] = speed
                    
        except queue.Empty:
            pass
        except Exception as e:
            logger.error(f"Error processing commands: {e}")
    
    def _update_motor_speeds(self):
        """Update motor speeds with acceleration limiting"""
        for motor in self.motors:
            try:
                current = self.current_speeds[motor.name]
                target = self.target_speeds[motor.name]
                
                if self.emergency_stop:
                    target = 0.0
                    self.target_speeds[motor.name] = 0.0
                
                # Apply acceleration limiting
                diff = target - current
                max_change = self.safety_limits.max_acceleration_rate * 100  # per cycle
                
                if abs(diff) > max_change:
                    change = max_change if diff > 0 else -max_change
                    new_speed = current + change
                else:
                    new_speed = target
                
                # Update PWM
                duty_cycle = self._speed_to_duty_cycle(motor, new_speed)
                self.pwm_objects[motor.name].ChangeDutyCycle(duty_cycle)
                
                self.current_speeds[motor.name] = new_speed
                
            except Exception as e:
                logger.error(f"Error updating motor {motor.name}: {e}")
    
    def _check_watchdog(self):
        """Check for watchdog timeout"""
        if time.time() - self.last_heartbeat > self.safety_limits.watchdog_timeout:
            logger.warning("Watchdog timeout - stopping motors")
            self.emergency_stop_all()
    
    def _speed_to_duty_cycle(self, motor: MotorConfig, speed: float) -> float:
        """
        Convert speed percentage to PWM duty cycle
        
        Args:
            motor: Motor configuration
            speed: Speed percentage (-100 to 100)
            
        Returns:
            float: PWM duty cycle percentage
        """
        # Convert speed to pulse width
        if speed == 0:
            pulse_width = motor.neutral_pulse_width
        elif speed > 0:
            # Forward: neutral to max
            pulse_width = motor.neutral_pulse_width + \
                         (speed / 100.0) * (motor.max_pulse_width - motor.neutral_pulse_width)
        else:
            # Reverse: neutral to min
            pulse_width = motor.neutral_pulse_width + \
                         (speed / 100.0) * (motor.neutral_pulse_width - motor.min_pulse_width)
        
        # Convert pulse width to duty cycle
        period = 1000.0 / motor.frequency  # Period in ms
        duty_cycle = (pulse_width / period) * 100.0
        
        return duty_cycle
    
    def _emergency_stop_callback(self, channel):
        """GPIO callback for emergency stop button"""
        logger.warning("Hardware emergency stop triggered")
        self.emergency_stop_all()
    
    def _flash_status_led(self):
        """Flash status LED to indicate emergency stop"""
        def flash():
            for _ in range(10):
                GPIO.output(self.status_led_pin, GPIO.HIGH)
                time.sleep(0.1)
                GPIO.output(self.status_led_pin, GPIO.LOW)
                time.sleep(0.1)
        
        self.executor.submit(flash)
    
    def _signal_handler(self, signum, frame):
        """Handle system signals for graceful shutdown"""
        logger.info(f"Received signal {signum}, shutting down...")
        self.stop()
        sys.exit(0)
    
    def _cleanup_gpio(self):
        """Clean up GPIO resources"""
        try:
            # Stop PWM
            for pwm in self.pwm_objects.values():
                pwm.stop()
            
            # Turn off status LED
            GPIO.output(self.status_led_pin, GPIO.LOW)
            
            # Cleanup GPIO
            GPIO.cleanup()
            
            logger.info("GPIO cleanup complete")
            
        except Exception as e:
            logger.error(f"Error during GPIO cleanup: {e}")

def create_default_config() -> Tuple[List[MotorConfig], SafetyLimits]:
    """
    Create default configuration for kart motor control
    
    Returns:
        Tuple containing motor configurations and safety limits
    """
    motors = [
        MotorConfig(
            pin=18,
            name="main_motor",
            min_pulse_width=1.0,
            max_pulse_width=2.0,
            neutral_pulse_width=1.5,
            frequency=50
        )
    ]
    
    safety_limits = SafetyLimits(
        max_acceleration_rate=0.05,  # Conservative acceleration
        max_speed=80.0,  # Limit to 80% max speed for safety
        emergency_stop_timeout=0.1,
        watchdog_timeout=2.0
    )
    
    return motors, safety_limits

def main():
    """Main function demonstrating the ESC controller usage"""
    print("Kart ESC Motor Control System")
    print("============================")
    
    # Create configuration
    motors, safety_limits = create_default_config()
    
    # Create controller
    controller = ESCController(motors, safety_limits)
    
    try:
        # Start the system
        if not controller.start():
            print("Failed to start motor control system")
            return
        
        print("Motor control system started successfully!")
        print("Commands:")
        print("  'f <speed>' - Set forward speed (0-100)")
        print("  'r <speed>' - Set reverse speed (0-100)")
        print("  's' - Stop motors")
        print("  'c' - Calibrate ESCs")
        print("  'e' - Emergency stop")
        print("  'reset' - Reset emergency stop")
        print("  'status' - Show system status")
        print("  'quit' - Exit")
        
        # Interactive control loop
        while True:
            try:
                cmd = input("\n> ").strip().lower()
                
                if cmd == 'quit':
                    break
                elif cmd == 's':
                    controller.set_all_motors_speed(0)
                    print("Motors stopped")
                elif cmd == 'c':
                    print("Calibrating ESCs...")
                    controller.calibrate_escs()
                    print("Calibration complete")
                elif cmd == 'e':
                    controller.emergency_stop_all()
                    print("Emergency stop activated")
                elif cmd == 'reset':
                    if controller.reset_emergency_stop():
                        print("Emergency stop reset")
                    else:
                        print("Cannot reset emergency stop")
                elif cmd == 'status':
                    status = controller.get_motor_status()
                    print(f"System Status: {status}")
                elif cmd.startswith('f '):
                    try:
                        speed = float(cmd.split()[1])
                        controller.set_all_motors_speed(speed)
                        print(f"Forward speed set to {speed}%")
                    except (IndexError, ValueError):
                        print("Invalid speed value")
                elif cmd.startswith('r '):
                    try:
                        speed = float(cmd.split()[1])
                        controller.set_all_motors_speed(-speed)
                        print(f"Reverse speed set to {speed}%")
                    except (IndexError, ValueError):
                        print("Invalid speed value")
                else:
                    print("Unknown command")
                    
            except KeyboardInterrupt:
                break
            except Exception as e:
                logger.error(f"Error in main loop: {e}")
    
    finally:
        controller.stop()
        print("System shutdown complete")

if __name__ == "__main__":
    main()