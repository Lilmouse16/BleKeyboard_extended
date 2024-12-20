#pragma once
#include "keyboard.h"
#include "hardware.h"
#include "constants.h"
#include "aht/time_distributor.h"
#include "timing/progress_tracker.h"
#include "timing/speed_adjuster.h"
#include <SPIFFS.h>

namespace Analysis {
    struct TimeFrame;  // Forward declaration
}

class HumanSimulator {
public:
    // Task information
    struct TaskInfo {
        uint32_t totalDurationMs;
        int totalClips;
        int currentClip;
        String videoId;
        float targetAHT;
        float difficulty;
    };

    // Behavioral state
    struct BehaviorState {
        float fatigueLevel;
        float alertnessLevel;
        float confidenceLevel;
        int consecutiveErrors;
        int wordsWithoutBreak;
        unsigned long lastBreakTime;
    };

    // Performance metrics
    struct PerformanceMetrics {
        float averageWPM;
        float currentWPM;
        float errorRate;
        float correctionRate;
        float speedCompliance;
        float timeUtilization;
    };

    // Behavioral configurations
    struct SpeedConfig {
        float baseWPM;
        float minSpeedFactor;
        float maxSpeedFactor;
        float fatigueImpact;
    };

    struct BehaviorConfig {
        float typoChance;
        float correctionChance;
        float thinkingFrequency;
        float recoveryRate;
        int maxWordsBeforeBreak;
    };

    HumanSimulator(Keyboard& kb, Hardware& hw) 
        : keyboard(kb)
        , hardware(hw) {}

    // Initialization and setup
    void init();
    void loadTask(const String& videoId);
    void reset();

    // Task processing
    void processClip(int clipNumber);
    void pause();
    void resume();
    bool isComplete() const;

    // Status and metrics
    TaskInfo getTaskInfo() const;
    BehaviorState getBehaviorState() const;
    PerformanceMetrics getPerformanceMetrics() const;

    // Add this method
    int getTotalClips() const { return totalClips; }

private:
    // Core components
    Keyboard& keyboard;
    Hardware& hardware;
    TaskInfo taskInfo;
    BehaviorState behavior;
    PerformanceMetrics metrics;

    // Timing management
    std::unique_ptr<AHT::TimeDistributor> timeDistributor;
    std::unique_ptr<Timing::ProgressTracker> progressTracker;
    std::unique_ptr<Timing::SpeedAdjuster> speedAdjuster;

    // Configuration
    SpeedConfig speedConfig;
    BehaviorConfig behaviorConfig;

    // Text processing
    void typeText(const String& text);
    void handleTypos(const String& word);
    void processTimeframe(const Analysis::TimeFrame& frame);
    void navigateToClip(int clipNumber);

    // Behavior simulation
    void applyFatigue();
    void updateAlertness();
    void simulateThinking();
    void simulateTypingDelay();
    void handleNaturalPauses(const String& text);
    
    // Performance monitoring
    void updatePerformanceMetrics();
    void adjustTypingSpeed();
    void checkProgressCompliance();
    
    // Utility methods
    String readClipContent(int clipNumber);
    void parseClipData(const String& content);
    void validateTimeframes();
    void logProgress();

    // Internal state tracking
    String currentWord;
    int wordsInBurst;
    bool isPaused;
    int totalClips;
    unsigned long sessionStartTime;
    unsigned long lastActivityTime;

    // Add missing method declarations
    void handleWord(const String& word);
    void makeTypo(const String& word);
    void correctTypo(const String& word, int typoPos);
    bool decideCorrectionStrategy(const String& word, int typoPos);
    void countClips();
    bool validateClipNumber(int clipNumber);
    char getRandomTypo(char originalChar);
    void typeWordNormally(const String& word);
    std::vector<Analysis::TimeFrame> parseTimeframes(const String& content);
    uint32_t estimateWordCount() const;
};