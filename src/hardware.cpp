#include "hardware.h"

void Hardware::init() {
    // Initialize pins
    pinMode(Constants::Hardware::BUTTON_PIN, INPUT_PULLUP);
    pinMode(Constants::Hardware::BUZZER_PIN, OUTPUT);
    pinMode(Constants::Hardware::BLUE_LED, OUTPUT);
    pinMode(Constants::Hardware::RED_LED, OUTPUT);
    
    // Set initial states
    setPhysicalLed(Constants::Hardware::BUZZER_PIN, false);
    setPhysicalLed(Constants::Hardware::BLUE_LED, true);
    setPhysicalLed(Constants::Hardware::RED_LED, true);

    if (Constants::Debug::ENABLE_SERIAL_DEBUG) {
        Serial.println("Hardware initialized");
        Serial.printf("Button pin: %d\n", Constants::Hardware::BUTTON_PIN);
        Serial.printf("Sound enabled: %d\n", soundEnabled);
    }
}

void Hardware::update() {
    unsigned long currentMillis = millis();

    // Update LED patterns
    switch (currentPattern) {
        case Pattern::PROGRESS_INDICATOR:
            if (currentMillis - lastProgressUpdate >= 
                Constants::Hardware::PROGRESS_UPDATE_INTERVAL) {
                handleProgressPattern();
                lastProgressUpdate = currentMillis;
            }
            break;

        case Pattern::SPEED_INDICATOR:
            handleSpeedPattern();
            break;

        case Pattern::ERROR_PATTERN:
            handleErrorPattern();
            break;

        case Pattern::SUCCESS_PATTERN:
            handleSuccessPattern();
            break;

        // Handle other patterns...
        default:
            break;
    }

    // Debug output
    if (Constants::Debug::ENABLE_SERIAL_DEBUG && 
        currentMillis - lastProgressUpdate >= Constants::Debug::DEBUG_UPDATE_INTERVAL) {
        printDebugInfo();
        lastProgressUpdate = currentMillis;
    }
}

void Hardware::updateProgress(const Timing::ProgressSnapshot& progress) {
    progressBrightness = progress.percentComplete / 100.0f;
    
    if (progress.compliance.timeUtilization > 
        Constants::AHT::PROGRESS_WARNING_THRESHOLD) {
        playSound(SoundType::SPEED_WARNING);
    }

    if (static_cast<int>(progress.percentComplete) % 25 == 0) {
        playSound(SoundType::PROGRESS_MILESTONE);
    }

    if (currentPattern == Pattern::PROGRESS_INDICATOR) {
        handleProgressPattern();
    }
}

void Hardware::updateSpeed(float currentWPM, float targetWPM) {
    speedIndicatorValue = currentWPM / targetWPM;
    
    if (abs(speedIndicatorValue - 1.0f) > 
        Constants::AHT::SPEED_WARNING_THRESHOLD / 100.0f) {
        playSound(SoundType::SPEED_WARNING);
    }

    if (currentPattern == Pattern::SPEED_INDICATOR) {
        handleSpeedPattern();
    }
}

Hardware::ButtonEvent Hardware::detectButtonEvent() {
    int reading = digitalRead(Constants::Hardware::BUTTON_PIN);
    ButtonEvent event = ButtonEvent::NONE;
    unsigned long currentTime = millis();

    // Debounce
    if (reading != lastButtonState) {
        lastDebounceTime = currentTime;
    }

    if ((currentTime - lastDebounceTime) > Constants::Hardware::DEBOUNCE_DELAY) {
        // Button pressed
        if (reading == LOW && lastButtonState == HIGH) {
            buttonPressCount++;
            lastButtonPressTime = currentTime;
            longPressActive = true;
        }
        // Button released
        else if (reading == HIGH && lastButtonState == LOW) {
            if (longPressActive && 
                currentTime - lastButtonPressTime >= 
                Constants::Hardware::LONG_PRESS_DURATION) {
                event = ButtonEvent::LONG_PRESS;
            } else {
                // Process multi-press events
                if (currentTime - lastButtonPressTime < 
                    Constants::Hardware::DOUBLE_PRESS_WINDOW) {
                    switch (buttonPressCount) {
                        case 2:
                            event = ButtonEvent::DOUBLE_PRESS;
                            break;
                        case 3:
                            event = ButtonEvent::TRIPLE_PRESS;
                            break;
                        default:
                            event = ButtonEvent::SINGLE_PRESS;
                    }
                } else {
                    event = ButtonEvent::SINGLE_PRESS;
                }
            }
            resetButtonState();
        }
    }

    lastButtonState = reading;
    return event;
}

// ... (Other implementation methods)

void Hardware::handleProgressPattern() {
    setPhysicalLedBrightness(Constants::Hardware::BLUE_LED, progressBrightness);
    setPhysicalLedBrightness(Constants::Hardware::RED_LED, 1.0f - progressBrightness);
}

void Hardware::handleSpeedPattern() {
    if (speedIndicatorValue < 0.9f) {
        // Too slow - pulse red
        pulseLed(Constants::Hardware::RED_LED, 500, 2);
        setPhysicalLed(Constants::Hardware::BLUE_LED, true);
    } else if (speedIndicatorValue > 1.1f) {
        // Too fast - pulse blue
        pulseLed(Constants::Hardware::BLUE_LED, 500, 2);
        setPhysicalLed(Constants::Hardware::RED_LED, true);
    } else {
        // Good speed - both LEDs on
        setPhysicalLed(Constants::Hardware::BLUE_LED, false);
        setPhysicalLed(Constants::Hardware::RED_LED, false);
    }
}

// ... (Remaining implementation methods)

// LED Control Methods
void Hardware::setPhysicalLed(uint8_t pin, bool state) {
    digitalWrite(pin, state ? LOW : HIGH);  // LEDs are active LOW
}

void Hardware::setPhysicalLedBrightness(uint8_t pin, float brightness) {
    // For digital pins, we simulate brightness with PWM if available
    // or use a threshold for non-PWM pins
    brightness = constrain(brightness, 0.0f, 1.0f);
    if (brightness >= 0.5f) {
        digitalWrite(pin, LOW);  // ON
    } else {
        digitalWrite(pin, HIGH); // OFF
    }
}

void Hardware::pulseLed(uint8_t pin, int duration, int pulseCount) {
    static unsigned long pulseStartTime = 0;
    static int currentPulse = 0;
    static bool pulseState = false;
    
    unsigned long currentTime = millis();
    
    // Reset pulse sequence if starting new one
    if (currentPulse == 0 || currentTime - pulseStartTime > duration) {
        pulseStartTime = currentTime;
        pulseState = true;
        currentPulse = 0;
    }
    
    // Calculate pulse timing
    int pulseDuration = duration / (pulseCount * 2);
    if (currentTime - pulseStartTime > pulseDuration) {
        pulseState = !pulseState;
        pulseStartTime = currentTime;
        if (!pulseState) currentPulse++;
    }
    
    // Stop after completing pulses
    if (currentPulse >= pulseCount) {
        setPhysicalLed(pin, true);  // LED off
        return;
    }
    
    setPhysicalLed(pin, !pulseState);
}

// Error and Success Pattern Handlers
void Hardware::handleErrorPattern() {
    static unsigned long lastErrorToggle = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastErrorToggle >= Constants::Hardware::ERROR_BLINK_SPEED) {
        ledState = !ledState;
        setPhysicalLed(Constants::Hardware::RED_LED, !ledState);
        setPhysicalLed(Constants::Hardware::BLUE_LED, true);
        lastErrorToggle = currentTime;
    }
}

void Hardware::handleSuccessPattern() {
    static unsigned long lastSuccessToggle = 0;
    static int successPhase = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastSuccessToggle >= Constants::Hardware::SUCCESS_BLINK_SPEED) {
        switch (successPhase) {
            case 0: // Both ON
                setPhysicalLed(Constants::Hardware::BLUE_LED, false);
                setPhysicalLed(Constants::Hardware::RED_LED, false);
                break;
            case 1: // Both OFF
                setPhysicalLed(Constants::Hardware::BLUE_LED, true);
                setPhysicalLed(Constants::Hardware::RED_LED, true);
                break;
        }
        
        successPhase = (successPhase + 1) % 2;
        lastSuccessToggle = currentTime;
    }
}

// Sound Control Methods
void Hardware::playSound(SoundType type) {
    if (!soundEnabled || !Constants::Hardware::BUZZER_ENABLED) return;
    
    int duration = 0;
    switch (type) {
        case SoundType::SECTION_COMPLETE:
            duration = Constants::Hardware::SECTION_COMPLETE_BEEP;
            break;
        case SoundType::ERROR:
            duration = Constants::Hardware::ERROR_BEEP;
            break;
        case SoundType::SUCCESS:
            duration = Constants::Hardware::SUCCESS_BEEP;
            break;
        case SoundType::SPEED_WARNING:
            duration = Constants::Hardware::SPEED_WARNING_BEEP;
            break;
        case SoundType::PROGRESS_MILESTONE:
            duration = Constants::Hardware::PROGRESS_BEEP;
            break;
    }

    if (duration > 0) {
        digitalWrite(Constants::Hardware::BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(Constants::Hardware::BUZZER_PIN, LOW);
    }
}

// AHT Status Display
void Hardware::showAHTStatus(const AHT::CalculationResult& aht) {
    if (!aht.isValid) {
        showError("Invalid AHT calculation");
        return;
    }
    
    // Show AHT status using LED pattern
    float progressIndicator = aht.targetMinutes / aht.upperBoundMinutes;
    setPhysicalLedBrightness(Constants::Hardware::BLUE_LED, progressIndicator);
    setPhysicalLedBrightness(Constants::Hardware::RED_LED, 1.0f - progressIndicator);
    
    if (Constants::Debug::ENABLE_SERIAL_DEBUG) {
        Serial.println("\n=== AHT Status ===");
        Serial.printf("Target: %.1f minutes\n", aht.targetMinutes);
        Serial.printf("Range: %.1f - %.1f minutes\n", 
                     aht.lowerBoundMinutes, aht.upperBoundMinutes);
    }
}

// Debug Methods
void Hardware::printDebugInfo() {
    if (!Constants::Debug::ENABLE_SERIAL_DEBUG) return;
    
    Serial.println("\n=== Hardware Status ===");
    Serial.printf("Pattern: %d\n", static_cast<int>(currentPattern));
    Serial.printf("Paused: %d\n", paused);
    Serial.printf("Section Complete: %d\n", sectionComplete);
    Serial.printf("Error State: %d\n", error);
    if (error) Serial.printf("Last Error: %s\n", lastError.c_str());
    
    printLedStatus();
}

void Hardware::printLedStatus() {
    if (!Constants::Debug::ENABLE_SERIAL_DEBUG) return;
    
    LedStatus status = getLedStatus();
    Serial.println("LED Status:");
    Serial.printf("  Red: %s (%.2f)\n", status.redOn ? "ON" : "OFF", status.redBrightness);
    Serial.printf("  Blue: %s (%.2f)\n", status.blueOn ? "ON" : "OFF", status.blueBrightness);
}

void Hardware::printButtonEvent(ButtonEvent event) {
    if (!Constants::Debug::ENABLE_SERIAL_DEBUG) return;
    
    const char* eventNames[] = {
        "NONE",
        "SINGLE_PRESS",
        "DOUBLE_PRESS",
        "LONG_PRESS",
        "TRIPLE_PRESS"
    };
    
    Serial.printf("Button Event: %s\n", eventNames[static_cast<int>(event)]);
}

// Reset Methods
void Hardware::resetButtonState() {
    buttonPressCount = 0;
    longPressActive = false;
    lastButtonPressTime = 0;
}

Hardware::LedStatus Hardware::getLedStatus() const {
    return {
        digitalRead(Constants::Hardware::RED_LED) == LOW,
        digitalRead(Constants::Hardware::BLUE_LED) == LOW,
        progressBrightness,
        speedIndicatorValue,
        currentPattern
    };
}

void Hardware::setLedPattern(Pattern pattern) {
    currentPattern = pattern;
    
    switch (pattern) {
        case Pattern::ALL_OFF:
            setPhysicalLed(Constants::Hardware::RED_LED, false);
            setPhysicalLed(Constants::Hardware::BLUE_LED, false);
            break;
            
        case Pattern::ALL_ON:
            setPhysicalLed(Constants::Hardware::RED_LED, true);
            setPhysicalLed(Constants::Hardware::BLUE_LED, true);
            break;
            
        case Pattern::RED_ONLY:
            setPhysicalLed(Constants::Hardware::RED_LED, true);
            setPhysicalLed(Constants::Hardware::BLUE_LED, false);
            break;
            
        case Pattern::BLUE_ONLY:
            setPhysicalLed(Constants::Hardware::RED_LED, false);
            setPhysicalLed(Constants::Hardware::BLUE_LED, true);
            break;
            
        case Pattern::SYNC_FLASH:
            // Will be handled in update()
            lastLedToggle = millis();
            ledState = false;
            break;
            
        default:
            // Other patterns will be handled in update()
            break;
    }
}

Hardware::ButtonEvent Hardware::handleButton() {
    ButtonEvent event = detectButtonEvent();
    
    if (event != ButtonEvent::NONE) {
        if (Constants::Debug::ENABLE_SERIAL_DEBUG) {
            printButtonEvent(event);
        }
        
        switch (event) {
            case ButtonEvent::SINGLE_PRESS:
                paused = !paused;
                break;
                
            case ButtonEvent::DOUBLE_PRESS:
                sectionComplete = !sectionComplete;
                break;
                
            case ButtonEvent::LONG_PRESS:
                reset();
                break;
                
            case ButtonEvent::TRIPLE_PRESS:
                // Special debug function
                if (Constants::Debug::ENABLE_SERIAL_DEBUG) {
                    printDebugInfo();
                }
                break;
                
            default:
                break;
        }
    }
    
    return event;
}

void Hardware::reset() {
    paused = true;
    sectionComplete = false;
    error = false;
    lastError = "";
    currentPattern = Pattern::ALL_OFF;
    progressBrightness = 0.0f;
    speedIndicatorValue = 0.0f;
    resetButtonState();
    
    // Reset physical outputs
    setPhysicalLed(Constants::Hardware::RED_LED, false);
    setPhysicalLed(Constants::Hardware::BLUE_LED, false);
    
    if (Constants::Debug::ENABLE_SERIAL_DEBUG) {
        Serial.println("Hardware reset complete");
    }
}