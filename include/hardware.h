#pragma once
#include "constants.h"
#include <Arduino.h>
#include "timing/progress_tracker.h"
#include "aht/calculator.h"
#include "keyboard.h"

class Hardware {
public:
    enum class Pattern {
        ALL_OFF,               // All LEDs off
        ALL_ON,               // All LEDs on
        ALTERNATING,          // Alternating pattern
        RED_ONLY,            // Only red LED
        BLUE_ONLY,           // Only blue LED
        SYNC_FLASH,          // Synchronized flashing
        PROGRESS_INDICATOR,   // Shows progress through brightness
        SPEED_INDICATOR,     // Shows typing speed status
        ERROR_PATTERN,       // Indicates error conditions
        SUCCESS_PATTERN      // Indicates successful completion
    };

    enum class SoundType {
        SECTION_COMPLETE,
        ERROR,
        SUCCESS,
        SPEED_WARNING,
        PROGRESS_MILESTONE
    };

    enum class ButtonEvent {
        NONE,
        SINGLE_PRESS,
        DOUBLE_PRESS,
        LONG_PRESS,
        TRIPLE_PRESS
    };

    struct LedStatus {
        bool redOn;
        bool blueOn;
        float redBrightness;
        float blueBrightness;
        Pattern currentPattern;
    };

    // Core functions
    void init();
    void update();  // New: Regular update function
    ButtonEvent handleButton();
    bool isButtonPressed();
    
    // LED control
    void setLedPattern(Pattern pattern);
    void setLedBrightness(float red, float blue);
    LedStatus getLedStatus() const;
    
    // Sound control
    void playSound(SoundType type);
    void enableSound(bool enable);
    
    // Progress indication
    void updateProgress(const Timing::ProgressSnapshot& progress);
    void updateSpeed(float currentWPM, float targetWPM);
    void showError(const String& message);
    void showSuccess(const String& message);
    void showAHTStatus(const AHT::CalculationResult& aht);

    // Status getters
    bool isPaused() const { return paused; }
    bool isSectionComplete() const { return sectionComplete; }
    bool hasError() const { return error; }
    String getLastError() const { return lastError; }

    // Status setters
    void setPaused(bool state) { paused = state; }
    void setSectionComplete(bool complete) { sectionComplete = complete; }
    void setError(bool hasError, const String& message = "") {
        error = hasError;
        lastError = message;
    }

    // Add these declarations
    void reset();  // Add this line to declare the reset method
    
private:
    // Status flags
    bool paused = true;
    bool sectionComplete = false;
    bool error = false;
    bool soundEnabled = true;
    String lastError;

    // LED state
    Pattern currentPattern = Pattern::ALL_OFF;
    bool ledState = false;
    float progressBrightness = 0.0f;
    float speedIndicatorValue = 0.0f;

    // Timing
    unsigned long lastLedToggle = 0;
    unsigned long lastProgressUpdate = 0;
    unsigned long lastDebounceTime = 0;

    // Button state
    int lastButtonState = HIGH;
    unsigned long lastButtonPressTime = 0;
    int buttonPressCount = 0;
    bool longPressActive = false;

    // Private helpers
    void handleProgressPattern();
    void handleSpeedPattern();
    void handleErrorPattern();
    void handleSuccessPattern();
    ButtonEvent detectButtonEvent();
    void resetButtonState();

    // LED control helpers
    void setPhysicalLed(uint8_t pin, bool state);
    void setPhysicalLedBrightness(uint8_t pin, float brightness);
    void pulseLed(uint8_t pin, int duration, int pulseCount);

    // Debug helpers
    void printDebugInfo();
    void printButtonEvent(ButtonEvent event);
    void printLedStatus();
};