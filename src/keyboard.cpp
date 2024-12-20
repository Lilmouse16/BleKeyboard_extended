#include "keyboard.h"

void Keyboard::init() {
    bleKeyboard.begin();
    resetStats();
}

bool Keyboard::isConnected() {
    return bleKeyboard.isConnected();
}

void Keyboard::type(const String& text, float speedMultiplier) {
    if (!isConnected()) return;
    
    int adjustedDelay = calculateDelay() / speedMultiplier;
    
    for (char c : text) {
        unsigned long currentTime = millis();
        if (currentTime - lastTypeTime < adjustedDelay) {
            delay(adjustedDelay - (currentTime - lastTypeTime));
        }
        
        bleKeyboard.write(c);
        updateStats(c);
        lastTypeTime = millis();
    }
}

void Keyboard::pressKey(uint8_t key) {
    if (isConnected()) {
        bleKeyboard.write(key);
    }
}

void Keyboard::releaseKey(uint8_t key) {
    if (isConnected()) {
        bleKeyboard.release(key);
    }
}

void Keyboard::navigate(int tabCount) {
    navigateWithSpeed(tabCount, 1.0f);
}

void Keyboard::navigateWithSpeed(int tabCount, float speedMultiplier) {
    if (!isConnected()) return;
    
    for (int i = 0; i < tabCount; i++) {
        pressKey(KEY_TAB);
        simulateTabDelay();
    }
}

void Keyboard::simulateTabDelay() {
    delay(random(Constants::Navigation::MIN_TAB_DELAY, 
                 Constants::Navigation::MAX_TAB_DELAY));
}

void Keyboard::setBaseSpeed(float wpm) {
    baseWPM = constrain(wpm, 10.0f, 200.0f);  // Reasonable limits
}

void Keyboard::adjustSpeed(float multiplier) {
    currentSpeedMultiplier = constrain(multiplier, 0.5f, 2.0f);
}

Keyboard::TypingStats Keyboard::getTypingStats() const {
    return stats;
}

void Keyboard::resetStats() {
    stats = TypingStats();
    lastTypeTime = 0;
    currentSpeedMultiplier = 1.0f;
}

void Keyboard::updateStats(char c) {
    stats.charactersTyped++;
    
    if (c == ' ' || c == '\n') {
        stats.wordsTyped++;
        
        // Calculate current WPM
        unsigned long typingTime = millis() - lastTypeTime;
        if (typingTime > 0) {
            float minutes = typingTime / 60000.0f;  // Convert to minutes
            stats.currentWPM = minutes > 0 ? stats.wordsTyped / minutes : 0;
            
            // Update average WPM
            stats.averageWPM = (stats.averageWPM + stats.currentWPM) / 2.0f;
        }
    }
}

int Keyboard::calculateDelay() const {
    return (60 * 1000) / (baseWPM * 5);  // 5 characters per word average
}
