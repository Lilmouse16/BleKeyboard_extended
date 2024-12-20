#pragma once
#include <Arduino.h>

// Forward declarations
namespace Analysis {
    class TextParser;
    struct TimeFrame;
}

namespace AHT {
    enum class ActivityType {
        TYPING,
        NAVIGATION,
        THINKING,
        TRANSITION
    };
}

namespace Constants {
    namespace Debug {
        const bool ENABLE_SERIAL_DEBUG = true;
        const bool ENABLE_DETAILED_TIMING = true;
        const bool ENABLE_SPEED_DEBUG = true;
        const int DEBUG_UPDATE_INTERVAL = 1000;
    }

    namespace Hardware {
        const int BUTTON_PIN = 0;      // G22 for external button OR 0 for built-in BOOT button
        const int BUZZER_PIN = 19;      // G19 for external buzzer 
        const int BLUE_LED = 2;         // External blue LED (GPIO 2)
        const int RED_LED = 13;         // External red LED 
        
        // LED timing constants
        const int DATA_FLICKER_SPEED = 100;
        const int COMPLETE_FLICKER_SPEED = 500;
        const int PROGRESS_UPDATE_INTERVAL = 250;
        const int ERROR_BLINK_SPEED = 200;
        const int SUCCESS_BLINK_SPEED = 300;
        
        // Sound constants
        const bool BUZZER_ENABLED = true;
        const int SECTION_COMPLETE_BEEP = 200;
        const int ERROR_BEEP = 100;
        const int SUCCESS_BEEP = 50;
        const int SPEED_WARNING_BEEP = 75;
        const int PROGRESS_BEEP = 25;
        
        // Button constants
        const int DEBOUNCE_DELAY = 200;
        const int LONG_PRESS_DURATION = 1000;
        const int DOUBLE_PRESS_WINDOW = 300;
    }

    namespace Navigation {
        const int FIRST_CLIP_TAB_COUNT = 0;   //normally 16
        const int NEXT_CLIP_TAB_COUNT = 0;     //normally 5
        const int CLIP_DELAY = 1000;
        const int MIN_TAB_DELAY = 140;
        const int MAX_TAB_DELAY = 400;
    }

    namespace Typing {
        const int BASE_WPM = 65;
        const int BASE_CHAR_DELAY = (60 * 1000) / (BASE_WPM * 5);
        const int WORD_PAUSE = BASE_CHAR_DELAY * 2.5;
        const int SENTENCE_PAUSE = BASE_CHAR_DELAY * 3;
        const int CORRECTION_DELAY = BASE_CHAR_DELAY / 2;
        const float MIN_SPEED_MULTIPLIER = 0.5f;
        const float MAX_SPEED_MULTIPLIER = 2.0f;
        const float SPEED_ADJUSTMENT_STEP = 0.1f;
    }

    namespace HumanBehavior {
        const float TYPO_CHANCE = 0.15f;
        const float DOUBLE_SPACE_CHANCE = 0.02f;
        const float UNCORRECTED_TYPO_CHANCE = 0.066f;
        const int UNCORRECTED_TYPO_THRESHOLD = 6;
        const float FATIGUE_FACTOR = 0.05f;
        const float MAX_FATIGUE_LEVEL = 0.3f;
        const float RECOVERY_RATE = 0.05f;
        const float FATIGUE_SPEED_IMPACT = 0.2f;
        const int THINKING_PAUSE_CHANCE = 15;
        const int MIN_THINKING_PAUSE = 800;
        const int MAX_THINKING_PAUSE = 2000;
        const int MAX_WORDS_BEFORE_BREAK = 20;
    }

    namespace AHT {
        const float DEFAULT_TYPING_PERCENTAGE = 80.0f;
        const float RESERVED_TIME_PERCENTAGE = 20.0f;
        const float PROGRESS_WARNING_THRESHOLD = 10.0f;
        const float SPEED_WARNING_THRESHOLD = 15.0f;
        
        // Difficulty weights
        const uint8_t TIME_DENSITY_WEIGHT = 20;
        const uint8_t COMPLEXITY_WEIGHT = 30;
        const uint8_t CAMERA_ACTIONS_WEIGHT = 30;
        const uint8_t TEXT_LENGTH_WEIGHT = 20;
    }

    namespace Timing {
        constexpr float MIN_SPEED_MULTIPLIER = 0.5f;
        constexpr float MAX_SPEED_MULTIPLIER = 1.5f;
    }
}
