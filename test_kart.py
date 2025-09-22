#!/usr/bin/env python3
"""
Test script for Kart ESC Motor Control System
===========================================

This script tests the core functionality of the ESC controller
without requiring actual hardware. It uses mock GPIO operations
to verify the code logic and safety features.
"""

import sys
import time
import threading
from unittest.mock import Mock, patch
import logging

# Mock RPi.GPIO for testing without hardware
sys.modules['RPi.GPIO'] = Mock()
sys.modules['RPi'] = Mock()

# Import our kart module after mocking GPIO
from kart import ESCController, MotorConfig, SafetyLimits, create_default_config

# Configure test logging
logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)

class KartControllerTest:
    """Test class for ESC Controller functionality"""
    
    def __init__(self):
        self.test_results = []
        
    def run_test(self, test_name, test_func):
        """Run a single test and record results"""
        try:
            logger.info(f"Running test: {test_name}")
            test_func()
            self.test_results.append((test_name, "PASSED", None))
            logger.info(f"✓ {test_name} PASSED")
        except Exception as e:
            self.test_results.append((test_name, "FAILED", str(e)))
            logger.error(f"✗ {test_name} FAILED: {e}")
    
    def test_configuration_creation(self):
        """Test configuration creation"""
        motors, safety_limits = create_default_config()
        
        assert len(motors) == 1, "Should have 1 motor by default"
        assert motors[0].name == "main_motor", "Motor name should be 'main_motor'"
        assert motors[0].pin == 18, "Motor pin should be 18"
        assert safety_limits.max_speed == 80.0, "Max speed should be 80%"
        
    def test_controller_initialization(self):
        """Test controller initialization"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        assert controller.motors == motors, "Motors should be set correctly"
        assert controller.safety_limits == safety_limits, "Safety limits should be set"
        assert not controller.is_running, "Controller should not be running initially"
        assert not controller.emergency_stop, "Emergency stop should be false initially"
    
    def test_speed_conversion(self):
        """Test speed to duty cycle conversion"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        motor = motors[0]
        
        # Test neutral speed (0%)
        duty_cycle = controller._speed_to_duty_cycle(motor, 0)
        expected = (1.5 / 20) * 100  # 1.5ms / 20ms period * 100%
        assert abs(duty_cycle - expected) < 0.1, f"Neutral duty cycle should be ~{expected}%"
        
        # Test maximum forward speed (100%)
        duty_cycle = controller._speed_to_duty_cycle(motor, 100)
        expected = (2.0 / 20) * 100  # 2.0ms / 20ms period * 100%
        assert abs(duty_cycle - expected) < 0.1, f"Max forward duty cycle should be ~{expected}%"
        
        # Test maximum reverse speed (-100%)
        duty_cycle = controller._speed_to_duty_cycle(motor, -100)
        expected = (1.0 / 20) * 100  # 1.0ms / 20ms period * 100%
        assert abs(duty_cycle - expected) < 0.1, f"Max reverse duty cycle should be ~{expected}%"
    
    def test_safety_limits(self):
        """Test safety limit enforcement"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        # Mock the command queue and threading
        controller.command_queue = Mock()
        controller.command_queue.put = Mock()
        
        # Test speed limiting
        result = controller.set_motor_speed("main_motor", 150)  # Over 100%
        assert result, "Command should be accepted but speed will be clamped"
        
        # Test emergency stop override
        controller.emergency_stop = True
        result = controller.set_motor_speed("main_motor", 50)
        assert not result, "Command should be rejected during emergency stop"
    
    def test_emergency_stop_functionality(self):
        """Test emergency stop functionality"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        # Mock PWM objects
        controller.pwm_objects = {"main_motor": Mock()}
        controller.pwm_objects["main_motor"].ChangeDutyCycle = Mock()
        
        # Test emergency stop
        controller.emergency_stop_all()
        
        assert controller.emergency_stop, "Emergency stop flag should be set"
        assert controller.current_speeds["main_motor"] == 0.0, "Current speed should be 0"
        assert controller.target_speeds["main_motor"] == 0.0, "Target speed should be 0"
        
        # Verify PWM was called to stop motor
        controller.pwm_objects["main_motor"].ChangeDutyCycle.assert_called()
    
    def test_multi_motor_control(self):
        """Test multiple motor control"""
        # Create configuration with two motors
        motors = [
            MotorConfig(18, "motor1"),
            MotorConfig(19, "motor2")
        ]
        safety_limits = SafetyLimits()
        
        controller = ESCController(motors, safety_limits)
        
        assert len(controller.current_speeds) == 2, "Should have speed tracking for both motors"
        assert "motor1" in controller.current_speeds, "Motor1 should be tracked"
        assert "motor2" in controller.current_speeds, "Motor2 should be tracked"
        
        # Mock command queue
        controller.command_queue = Mock()
        controller.command_queue.put = Mock()
        
        # Test setting all motors
        result = controller.set_all_motors_speed(50)
        assert result, "Setting all motors should succeed"
        
        # Should have called put twice (once for each motor)
        assert controller.command_queue.put.call_count == 2, "Should queue command for each motor"
    
    def test_watchdog_timeout_logic(self):
        """Test watchdog timeout calculation"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        # Set last heartbeat to past
        past_time = time.time() - 5.0  # 5 seconds ago
        controller.last_heartbeat = past_time
        
        # Mock emergency stop
        original_emergency_stop = controller.emergency_stop_all
        controller.emergency_stop_all = Mock()
        
        # Test watchdog check
        controller._check_watchdog()
        
        # Should trigger emergency stop due to timeout
        controller.emergency_stop_all.assert_called()
        
        # Restore original method
        controller.emergency_stop_all = original_emergency_stop
    
    def test_acceleration_limiting(self):
        """Test acceleration limiting logic"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        # Set initial conditions
        controller.current_speeds["main_motor"] = 0.0
        controller.target_speeds["main_motor"] = 100.0
        
        # Mock PWM objects
        controller.pwm_objects = {"main_motor": Mock()}
        controller.pwm_objects["main_motor"].ChangeDutyCycle = Mock()
        
        # Update motor speeds (should apply acceleration limiting)
        controller._update_motor_speeds()
        
        # Speed should increase but not jump directly to 100%
        new_speed = controller.current_speeds["main_motor"]
        max_change = safety_limits.max_acceleration_rate * 100
        
        assert 0 < new_speed <= max_change, f"Speed should increase gradually, got {new_speed}"
        assert new_speed < 100.0, "Speed should not jump to target immediately"
    
    def test_status_reporting(self):
        """Test system status reporting"""
        motors, safety_limits = create_default_config()
        controller = ESCController(motors, safety_limits)
        
        status = controller.get_motor_status()
        
        assert "motors" in status, "Status should include motors section"
        assert "system" in status, "Status should include system section"
        assert "main_motor" in status["motors"], "Status should include main motor"
        assert "running" in status["system"], "Status should include running state"
        assert "emergency_stop" in status["system"], "Status should include emergency stop state"
    
    def run_all_tests(self):
        """Run all tests and report results"""
        logger.info("Starting Kart ESC Controller Tests")
        logger.info("=" * 50)
        
        # List of all test methods
        tests = [
            ("Configuration Creation", self.test_configuration_creation),
            ("Controller Initialization", self.test_controller_initialization),
            ("Speed Conversion", self.test_speed_conversion),
            ("Safety Limits", self.test_safety_limits),
            ("Emergency Stop", self.test_emergency_stop_functionality),
            ("Multi-Motor Control", self.test_multi_motor_control),
            ("Watchdog Timeout", self.test_watchdog_timeout_logic),
            ("Acceleration Limiting", self.test_acceleration_limiting),
            ("Status Reporting", self.test_status_reporting),
        ]
        
        # Run all tests
        for test_name, test_func in tests:
            self.run_test(test_name, test_func)
        
        # Report results
        logger.info("=" * 50)
        logger.info("Test Results Summary:")
        
        passed = sum(1 for _, result, _ in self.test_results if result == "PASSED")
        failed = sum(1 for _, result, _ in self.test_results if result == "FAILED")
        
        logger.info(f"Tests Passed: {passed}")
        logger.info(f"Tests Failed: {failed}")
        logger.info(f"Total Tests: {len(self.test_results)}")
        
        if failed > 0:
            logger.info("\nFailed Tests:")
            for name, result, error in self.test_results:
                if result == "FAILED":
                    logger.error(f"  {name}: {error}")
        
        return failed == 0

def main():
    """Main test function"""
    print("Kart ESC Motor Control System - Test Suite")
    print("=" * 50)
    
    # Create and run tests
    test_suite = KartControllerTest()
    success = test_suite.run_all_tests()
    
    if success:
        print("\n🎉 All tests passed! The ESC controller implementation is working correctly.")
        print("\nNext steps:")
        print("1. Connect your hardware (Raspberry Pi, ESC, motor)")
        print("2. Verify pin connections match the configuration")
        print("3. Run 'python3 kart.py' to start the actual controller")
        print("4. Test with low speeds first for safety")
        return 0
    else:
        print("\n❌ Some tests failed. Please review the errors above.")
        print("Fix any issues before running with actual hardware.")
        return 1

if __name__ == "__main__":
    sys.exit(main())