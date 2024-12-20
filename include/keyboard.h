#pragma once
#include <BleKeyboard.h>
#include "constants.h"

class Keyboard {
public:
    // Statistics structure
    struct TypingStats {
        uint32_t charactersTyped = 0;
        uint32_t wordsTyped = 0;
        float currentWPM = 0;
        float averageWPM = 0;
    };

    void init();
    bool isConnected();
    
    // Typing functions
    void type(const String& text, float speedMultiplier = 1.0f);
    void pressKey(uint8_t key);
    void releaseKey(uint8_t key);
    
    // Navigation
    void navigate(int tabCount);
    void navigateWithSpeed(int tabCount, float speedMultiplier);
    void simulateTabDelay();
    
    // Speed control
    void setBaseSpeed(float wpm);
    void adjustSpeed(float multiplier);
    
    // Statistics
    TypingStats getTypingStats() const;
    void resetStats();

private:
    BleKeyboard bleKeyboard{"PRO X TSL", "Logitech", 100};
    TypingStats stats;
    float currentSpeedMultiplier = 1.0f;
    float baseWPM = Constants::Typing::BASE_WPM;
    unsigned long lastTypeTime = 0;

    void updateStats(char c);
    int calculateDelay() const;
};