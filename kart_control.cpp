/*
 * ESC Motor Control System for Kart (C++ Implementation)
 * =====================================================
 * 
 * High-performance C++ implementation for ESC motor control with:
 * - Real-time performance with minimal latency
 * - Multi-threaded parallel processing
 * - Hardware-level GPIO control
 * - Advanced safety systems and error handling
 * - Memory-efficient design
 * 
 * Compile with: g++ -std=c++17 -pthread -lwiringPi -lrt -o kart_control kart_control.cpp
 * 
 * Hardware Requirements:
 * - Raspberry Pi with wiringPi library
 * - ESC compatible with servo PWM signals
 * - Proper power supply and safety switches
 * 
 * Author: Created for high-performance kart motor control
 */

#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <future>
#include <fstream>
#include <signal.h>
#include <cmath>
#include <algorithm>
#include <wiringPi.h>
#include <softPwm.h>

// Logging system
class Logger {
private:
    std::mutex log_mutex;
    std::ofstream log_file;
    bool console_output;

public:
    enum Level { DEBUG, INFO, WARNING, ERROR };

    Logger(const std::string& filename = "/tmp/kart_motor_cpp.log", bool console = true) 
        : log_file(filename), console_output(console) {}

    void log(Level level, const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::string level_str;
        switch(level) {
            case DEBUG: level_str = "DEBUG"; break;
            case INFO: level_str = "INFO"; break;
            case WARNING: level_str = "WARNING"; break;
            case ERROR: level_str = "ERROR"; break;
        }
        
        char timestamp[26];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t));
        
        std::string log_entry = std::string(timestamp) + " - " + level_str + " - " + message;
        
        if (log_file.is_open()) {
            log_file << log_entry << std::endl;
            log_file.flush();
        }
        
        if (console_output) {
            std::cout << log_entry << std::endl;
        }
    }
};

// Global logger instance
static Logger g_logger;

// Motor configuration structure
struct MotorConfig {
    int pin;
    std::string name;
    double min_pulse_width;    // milliseconds
    double max_pulse_width;    // milliseconds
    double neutral_pulse_width; // milliseconds
    int frequency;             // Hz
    
    MotorConfig(int p, const std::string& n, double min_pw = 1.0, 
                double max_pw = 2.0, double neutral_pw = 1.5, int freq = 50)
        : pin(p), name(n), min_pulse_width(min_pw), max_pulse_width(max_pw),
          neutral_pulse_width(neutral_pw), frequency(freq) {}
};

// Safety limits configuration
struct SafetyLimits {
    double max_acceleration_rate;  // Maximum change per update cycle
    double max_speed;              // Maximum speed percentage
    double emergency_stop_timeout; // seconds
    double watchdog_timeout;       // seconds
    
    SafetyLimits(double max_accel = 0.05, double max_spd = 80.0,
                 double emerg_timeout = 0.1, double wd_timeout = 2.0)
        : max_acceleration_rate(max_accel), max_speed(max_spd),
          emergency_stop_timeout(emerg_timeout), watchdog_timeout(wd_timeout) {}
};

// Command structure for thread communication
struct Command {
    enum Type { SET_SPEED, EMERGENCY_STOP, CALIBRATE, SHUTDOWN };
    
    Type type;
    std::string motor_name;
    double speed;
    bool immediate;
    std::chrono::steady_clock::time_point timestamp;
    
    Command(Type t, const std::string& motor = "", double spd = 0.0, bool imm = false)
        : type(t), motor_name(motor), speed(spd), immediate(imm),
          timestamp(std::chrono::steady_clock::now()) {}
};

// High-performance ESC Controller class
class ESCController {
private:
    std::vector<MotorConfig> motors;
    SafetyLimits safety_limits;
    
    // State variables (thread-safe)
    std::atomic<bool> is_running{false};
    std::atomic<bool> emergency_stop{false};
    std::atomic<bool> shutdown_requested{false};
    
    // Motor speeds (protected by mutex)
    mutable std::mutex speed_mutex;
    std::unordered_map<std::string, double> current_speeds;
    std::unordered_map<std::string, double> target_speeds;
    
    // Threading
    std::unique_ptr<std::thread> control_thread;
    std::unique_ptr<std::thread> monitor_thread;
    std::unique_ptr<std::thread> command_thread;
    
    // Command queue
    std::queue<Command> command_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_cv;
    
    // Timing
    std::atomic<std::chrono::steady_clock::time_point> last_heartbeat;
    
    // GPIO pins
    static constexpr int EMERGENCY_PIN = 21;
    static constexpr int STATUS_LED_PIN = 20;
    
    // Real-time performance
    static constexpr std::chrono::microseconds CONTROL_PERIOD{20000}; // 50Hz
    static constexpr int PWM_RANGE = 1000;

public:
    ESCController(const std::vector<MotorConfig>& motor_configs, 
                  const SafetyLimits& limits)
        : motors(motor_configs), safety_limits(limits) {
        
        // Initialize speed maps
        for (const auto& motor : motors) {
            current_speeds[motor.name] = 0.0;
            target_speeds[motor.name] = 0.0;
        }
        
        last_heartbeat.store(std::chrono::steady_clock::now());
        
        // Setup signal handlers
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        
        g_logger.log(Logger::INFO, "ESC Controller initialized with " + 
                     std::to_string(motors.size()) + " motors");
    }
    
    ~ESCController() {
        stop();
    }
    
    bool initialize() {
        try {
            // Initialize wiringPi
            if (wiringPiSetupGpio() == -1) {
                g_logger.log(Logger::ERROR, "Failed to initialize wiringPi");
                return false;
            }
            
            // Setup motor pins with software PWM
            for (const auto& motor : motors) {
                pinMode(motor.pin, OUTPUT);
                if (softPwmCreate(motor.pin, 0, PWM_RANGE) != 0) {
                    g_logger.log(Logger::ERROR, "Failed to create PWM for motor " + motor.name);
                    return false;
                }
                g_logger.log(Logger::INFO, "Initialized motor " + motor.name + 
                           " on pin " + std::to_string(motor.pin));
            }
            
            // Setup emergency stop pin
            pinMode(EMERGENCY_PIN, INPUT);
            pullUpDnControl(EMERGENCY_PIN, PUD_UP);
            if (wiringPiISR(EMERGENCY_PIN, INT_EDGE_FALLING, &emergency_interrupt) < 0) {
                g_logger.log(Logger::ERROR, "Failed to setup emergency stop interrupt");
                return false;
            }
            
            // Setup status LED
            pinMode(STATUS_LED_PIN, OUTPUT);
            digitalWrite(STATUS_LED_PIN, LOW);
            
            g_logger.log(Logger::INFO, "GPIO initialization complete");
            return true;
            
        } catch (const std::exception& e) {
            g_logger.log(Logger::ERROR, "Initialization failed: " + std::string(e.what()));
            return false;
        }
    }
    
    bool start() {
        if (!initialize()) {
            return false;
        }
        
        try {
            is_running.store(true);
            emergency_stop.store(false);
            shutdown_requested.store(false);
            
            // Start worker threads
            control_thread = std::make_unique<std::thread>(&ESCController::control_loop, this);
            monitor_thread = std::make_unique<std::thread>(&ESCController::monitor_loop, this);
            command_thread = std::make_unique<std::thread>(&ESCController::command_loop, this);
            
            // Set real-time priority for control thread
            set_thread_priority(control_thread.get(), 80);
            
            digitalWrite(STATUS_LED_PIN, HIGH);
            
            g_logger.log(Logger::INFO, "Motor control system started");
            return true;
            
        } catch (const std::exception& e) {
            g_logger.log(Logger::ERROR, "Failed to start: " + std::string(e.what()));
            return false;
        }
    }
    
    void stop() {
        g_logger.log(Logger::INFO, "Stopping motor control system...");
        
        shutdown_requested.store(true);
        is_running.store(false);
        
        // Stop all motors immediately
        emergency_stop_all();
        
        // Notify command thread
        queue_cv.notify_all();
        
        // Wait for threads to finish
        if (control_thread && control_thread->joinable()) {
            control_thread->join();
        }
        if (monitor_thread && monitor_thread->joinable()) {
            monitor_thread->join();
        }
        if (command_thread && command_thread->joinable()) {
            command_thread->join();
        }
        
        cleanup_gpio();
        g_logger.log(Logger::INFO, "Motor control system stopped");
    }
    
    bool set_motor_speed(const std::string& motor_name, double speed, bool immediate = false) {
        if (emergency_stop.load()) {
            g_logger.log(Logger::WARNING, "Emergency stop active - command rejected");
            return false;
        }
        
        // Clamp speed to safety limits
        speed = std::clamp(speed, -safety_limits.max_speed, safety_limits.max_speed);
        
        // Queue command
        Command cmd(Command::SET_SPEED, motor_name, speed, immediate);
        
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            command_queue.push(cmd);
        }
        queue_cv.notify_one();
        
        // Update heartbeat
        last_heartbeat.store(std::chrono::steady_clock::now());
        
        return true;
    }
    
    bool set_all_motors_speed(double speed, bool immediate = false) {
        bool success = true;
        for (const auto& motor : motors) {
            if (!set_motor_speed(motor.name, speed, immediate)) {
                success = false;
            }
        }
        return success;
    }
    
    void emergency_stop_all() {
        g_logger.log(Logger::WARNING, "EMERGENCY STOP ACTIVATED");
        emergency_stop.store(true);
        
        // Immediately set all motors to neutral
        for (const auto& motor : motors) {
            int duty_cycle = speed_to_duty_cycle(motor, 0.0);
            softPwmWrite(motor.pin, duty_cycle);
            
            std::lock_guard<std::mutex> lock(speed_mutex);
            current_speeds[motor.name] = 0.0;
            target_speeds[motor.name] = 0.0;
        }
        
        // Flash status LED
        std::thread(&ESCController::flash_status_led, this).detach();
    }
    
    bool reset_emergency_stop() {
        if (digitalRead(EMERGENCY_PIN) == LOW) {
            g_logger.log(Logger::WARNING, "Cannot reset - hardware switch still active");
            return false;
        }
        
        emergency_stop.store(false);
        last_heartbeat.store(std::chrono::steady_clock::now());
        digitalWrite(STATUS_LED_PIN, HIGH);
        
        g_logger.log(Logger::INFO, "Emergency stop reset");
        return true;
    }
    
    bool calibrate_escs() {
        g_logger.log(Logger::INFO, "Starting ESC calibration...");
        
        try {
            // Send maximum signal
            for (const auto& motor : motors) {
                int duty_cycle = speed_to_duty_cycle(motor, 100.0);
                softPwmWrite(motor.pin, duty_cycle);
            }
            
            g_logger.log(Logger::INFO, "Sending maximum signal for 3 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            // Send minimum signal
            for (const auto& motor : motors) {
                int duty_cycle = speed_to_duty_cycle(motor, -100.0);
                softPwmWrite(motor.pin, duty_cycle);
            }
            
            g_logger.log(Logger::INFO, "Sending minimum signal for 3 seconds...");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            
            // Send neutral signal
            for (const auto& motor : motors) {
                int duty_cycle = speed_to_duty_cycle(motor, 0.0);
                softPwmWrite(motor.pin, duty_cycle);
            }
            
            g_logger.log(Logger::INFO, "ESC calibration complete");
            return true;
            
        } catch (const std::exception& e) {
            g_logger.log(Logger::ERROR, "ESC calibration failed: " + std::string(e.what()));
            return false;
        }
    }
    
    std::string get_status() const {
        std::lock_guard<std::mutex> lock(speed_mutex);
        
        std::string status = "Motors: ";
        for (const auto& motor : motors) {
            status += motor.name + "(current:" + std::to_string(current_speeds.at(motor.name)) +
                     " target:" + std::to_string(target_speeds.at(motor.name)) + ") ";
        }
        
        status += "Running:" + std::to_string(is_running.load()) +
                 " Emergency:" + std::to_string(emergency_stop.load());
        
        return status;
    }

private:
    void control_loop() {
        g_logger.log(Logger::INFO, "Control loop started");
        
        auto next_cycle = std::chrono::steady_clock::now();
        
        while (is_running.load() && !shutdown_requested.load()) {
            try {
                // Update motor speeds with acceleration limiting
                update_motor_speeds();
                
                // Check watchdog
                check_watchdog();
                
                // Sleep until next cycle (real-time scheduling)
                next_cycle += CONTROL_PERIOD;
                std::this_thread::sleep_until(next_cycle);
                
            } catch (const std::exception& e) {
                g_logger.log(Logger::ERROR, "Error in control loop: " + std::string(e.what()));
            }
        }
        
        g_logger.log(Logger::INFO, "Control loop stopped");
    }
    
    void monitor_loop() {
        g_logger.log(Logger::INFO, "Monitor loop started");
        
        while (is_running.load() && !shutdown_requested.load()) {
            try {
                // Log status periodically
                static int counter = 0;
                if (++counter % 10 == 0) {
                    g_logger.log(Logger::INFO, "Status: " + get_status());
                }
                
                std::this_thread::sleep_for(std::chrono::seconds(1));
                
            } catch (const std::exception& e) {
                g_logger.log(Logger::ERROR, "Error in monitor loop: " + std::string(e.what()));
            }
        }
        
        g_logger.log(Logger::INFO, "Monitor loop stopped");
    }
    
    void command_loop() {
        g_logger.log(Logger::INFO, "Command loop started");
        
        while (!shutdown_requested.load()) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            
            queue_cv.wait(lock, [this] { 
                return !command_queue.empty() || shutdown_requested.load(); 
            });
            
            while (!command_queue.empty()) {
                Command cmd = command_queue.front();
                command_queue.pop();
                lock.unlock();
                
                process_command(cmd);
                
                lock.lock();
            }
        }
        
        g_logger.log(Logger::INFO, "Command loop stopped");
    }
    
    void process_command(const Command& cmd) {
        switch (cmd.type) {
            case Command::SET_SPEED: {
                std::lock_guard<std::mutex> lock(speed_mutex);
                
                if (current_speeds.find(cmd.motor_name) != current_speeds.end()) {
                    if (cmd.immediate) {
                        current_speeds[cmd.motor_name] = cmd.speed;
                    }
                    target_speeds[cmd.motor_name] = cmd.speed;
                }
                break;
            }
            case Command::EMERGENCY_STOP:
                emergency_stop_all();
                break;
            case Command::CALIBRATE:
                calibrate_escs();
                break;
            case Command::SHUTDOWN:
                shutdown_requested.store(true);
                break;
        }
    }
    
    void update_motor_speeds() {
        std::lock_guard<std::mutex> lock(speed_mutex);
        
        for (const auto& motor : motors) {
            double current = current_speeds[motor.name];
            double target = target_speeds[motor.name];
            
            if (emergency_stop.load()) {
                target = 0.0;
                target_speeds[motor.name] = 0.0;
            }
            
            // Apply acceleration limiting
            double diff = target - current;
            double max_change = safety_limits.max_acceleration_rate * 100.0;
            
            double new_speed;
            if (std::abs(diff) > max_change) {
                new_speed = current + (diff > 0 ? max_change : -max_change);
            } else {
                new_speed = target;
            }
            
            // Update PWM
            int duty_cycle = speed_to_duty_cycle(motor, new_speed);
            softPwmWrite(motor.pin, duty_cycle);
            
            current_speeds[motor.name] = new_speed;
        }
    }
    
    void check_watchdog() {
        auto now = std::chrono::steady_clock::now();
        auto last_beat = last_heartbeat.load();
        
        auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - last_beat);
        
        if (elapsed.count() > safety_limits.watchdog_timeout) {
            g_logger.log(Logger::WARNING, "Watchdog timeout - stopping motors");
            emergency_stop_all();
        }
    }
    
    int speed_to_duty_cycle(const MotorConfig& motor, double speed) const {
        double pulse_width;
        
        if (speed == 0.0) {
            pulse_width = motor.neutral_pulse_width;
        } else if (speed > 0) {
            pulse_width = motor.neutral_pulse_width + 
                         (speed / 100.0) * (motor.max_pulse_width - motor.neutral_pulse_width);
        } else {
            pulse_width = motor.neutral_pulse_width + 
                         (speed / 100.0) * (motor.neutral_pulse_width - motor.min_pulse_width);
        }
        
        // Convert to duty cycle (0-PWM_RANGE)
        double period = 1000.0 / motor.frequency;  // Period in ms
        double duty_percent = (pulse_width / period);
        
        return static_cast<int>(duty_percent * PWM_RANGE);
    }
    
    void flash_status_led() {
        for (int i = 0; i < 10; ++i) {
            digitalWrite(STATUS_LED_PIN, HIGH);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            digitalWrite(STATUS_LED_PIN, LOW);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void cleanup_gpio() {
        try {
            // Turn off status LED
            digitalWrite(STATUS_LED_PIN, LOW);
            
            // Set all motors to neutral
            for (const auto& motor : motors) {
                softPwmWrite(motor.pin, speed_to_duty_cycle(motor, 0.0));
            }
            
            g_logger.log(Logger::INFO, "GPIO cleanup complete");
            
        } catch (const std::exception& e) {
            g_logger.log(Logger::ERROR, "Error during cleanup: " + std::string(e.what()));
        }
    }
    
    static void set_thread_priority(std::thread* thread, int priority) {
        sched_param sch;
        int policy;
        pthread_getschedparam(thread->native_handle(), &policy, &sch);
        sch.sched_priority = priority;
        if (pthread_setschedparam(thread->native_handle(), SCHED_FIFO, &sch)) {
            g_logger.log(Logger::WARNING, "Failed to set thread priority");
        }
    }
    
    // Static members for signal handling
    static ESCController* instance;
    
    static void signal_handler(int signum) {
        g_logger.log(Logger::INFO, "Received signal " + std::to_string(signum) + ", shutting down...");
        if (instance) {
            instance->stop();
        }
        exit(0);
    }
    
    static void emergency_interrupt() {
        g_logger.log(Logger::WARNING, "Hardware emergency stop triggered");
        if (instance) {
            instance->emergency_stop_all();
        }
    }
};

// Static member definition
ESCController* ESCController::instance = nullptr;

// Factory function for default configuration
std::pair<std::vector<MotorConfig>, SafetyLimits> create_default_config() {
    std::vector<MotorConfig> motors = {
        MotorConfig(18, "main_motor", 1.0, 2.0, 1.5, 50)
    };
    
    SafetyLimits limits(0.05, 80.0, 0.1, 2.0);
    
    return std::make_pair(motors, limits);
}

// Main function
int main() {
    std::cout << "Kart ESC Motor Control System (C++)" << std::endl;
    std::cout << "====================================" << std::endl;
    
    // Create configuration
    auto [motors, safety_limits] = create_default_config();
    
    // Create controller
    auto controller = std::make_unique<ESCController>(motors, safety_limits);
    ESCController::instance = controller.get();
    
    try {
        // Start the system
        if (!controller->start()) {
            std::cout << "Failed to start motor control system" << std::endl;
            return 1;
        }
        
        std::cout << "Motor control system started successfully!" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  'f <speed>' - Set forward speed (0-100)" << std::endl;
        std::cout << "  'r <speed>' - Set reverse speed (0-100)" << std::endl;
        std::cout << "  's' - Stop motors" << std::endl;
        std::cout << "  'c' - Calibrate ESCs" << std::endl;
        std::cout << "  'e' - Emergency stop" << std::endl;
        std::cout << "  'reset' - Reset emergency stop" << std::endl;
        std::cout << "  'status' - Show system status" << std::endl;
        std::cout << "  'quit' - Exit" << std::endl;
        
        // Interactive control loop
        std::string input;
        while (std::getline(std::cin, input)) {
            if (input == "quit") {
                break;
            } else if (input == "s") {
                controller->set_all_motors_speed(0);
                std::cout << "Motors stopped" << std::endl;
            } else if (input == "c") {
                std::cout << "Calibrating ESCs..." << std::endl;
                controller->calibrate_escs();
                std::cout << "Calibration complete" << std::endl;
            } else if (input == "e") {
                controller->emergency_stop_all();
                std::cout << "Emergency stop activated" << std::endl;
            } else if (input == "reset") {
                if (controller->reset_emergency_stop()) {
                    std::cout << "Emergency stop reset" << std::endl;
                } else {
                    std::cout << "Cannot reset emergency stop" << std::endl;
                }
            } else if (input == "status") {
                std::cout << "System Status: " << controller->get_status() << std::endl;
            } else if (input.substr(0, 2) == "f ") {
                try {
                    double speed = std::stod(input.substr(2));
                    controller->set_all_motors_speed(speed);
                    std::cout << "Forward speed set to " << speed << "%" << std::endl;
                } catch (const std::exception&) {
                    std::cout << "Invalid speed value" << std::endl;
                }
            } else if (input.substr(0, 2) == "r ") {
                try {
                    double speed = std::stod(input.substr(2));
                    controller->set_all_motors_speed(-speed);
                    std::cout << "Reverse speed set to " << speed << "%" << std::endl;
                } catch (const std::exception&) {
                    std::cout << "Invalid speed value" << std::endl;
                }
            } else {
                std::cout << "Unknown command" << std::endl;
            }
            
            std::cout << "> ";
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    controller->stop();
    std::cout << "System shutdown complete" << std::endl;
    return 0;
}