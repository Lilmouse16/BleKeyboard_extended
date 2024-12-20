#pragma once
#include <Arduino.h>

#ifndef DEBUG_PRINT
    #define DEBUG_PRINT(x)
#endif

#ifndef DEBUG_PRINTLN
    #define DEBUG_PRINTLN(x)
#endif

namespace AHT {
    constexpr uint32_t MILLIS_PER_MINUTE = 60000;

    // Add this struct before TimeDistributor
    struct TimeAllocation {
        uint32_t totalMillis;       // Total allocated time
        uint32_t typingMillis;      // Time allocated for typing
        float typingSpeedFactor;    // Speed adjustment factor
    };

    // Detailed breakdown of activity timings
    struct ActivityTimings {
        uint32_t typingMillis;          // Pure typing time
        uint32_t navigationMillis;       // Tab navigation time
        uint32_t thinkingPauseMillis;   // Natural pauses
        uint32_t correctionMillis;       // Time for typo corrections
        uint32_t transitionMillis;       // Time between clips
        float baseWPM;                  // Base words per minute
        float adjustedWPM;              // Speed-adjusted WPM
    };

    // Progress tracking for time distribution
    struct TimeProgress {
        uint32_t elapsedTotal;          // Total time elapsed
        uint32_t elapsedTyping;         // Time spent typing
        uint32_t elapsedNavigation;     // Time spent navigating
        uint32_t elapsedThinking;       // Time spent in pauses
        uint32_t elapsedCorrections;    // Time spent correcting
        uint32_t elapsedTransitions;    // Time between clips
        float completionPercent;        // Overall progress
        float speedCompliance;          // How well maintaining target speed
    };

    class TimeDistributor {
    public:
        TimeDistributor(const TimeAllocation& allocation, float wordsPerClip)
            : totalAllocation(allocation), estimatedWords(wordsPerClip) {
            calculateActivityTimings();
        }

        // Get initial timing calculations
        ActivityTimings getTimings() const {
            return timings;
        }

        // Update progress and get current status
        TimeProgress updateProgress(uint32_t newTypingTime, 
                                  uint32_t newNavigationTime,
                                  uint32_t newThinkingTime,
                                  uint32_t newCorrectionTime,
                                  uint32_t newTransitionTime) {
            TimeProgress progress;
            
            // Update elapsed times
            progress.elapsedTyping = newTypingTime;
            progress.elapsedNavigation = newNavigationTime;
            progress.elapsedThinking = newThinkingTime;
            progress.elapsedCorrections = newCorrectionTime;
            progress.elapsedTransitions = newTransitionTime;
            
            // Calculate total elapsed
            progress.elapsedTotal = newTypingTime + newNavigationTime + 
                                  newThinkingTime + newCorrectionTime + 
                                  newTransitionTime;
            
            // Calculate completion percentage
            progress.completionPercent = (float)progress.elapsedTotal / 
                                       totalAllocation.totalMillis * 100.0f;
            
            // Calculate speed compliance
            float expectedTypingProgress = (float)progress.elapsedTotal / 
                                         totalAllocation.totalMillis;
            float actualTypingProgress = (float)progress.elapsedTyping / 
                                       timings.typingMillis;
            
            progress.speedCompliance = actualTypingProgress / expectedTypingProgress;
            
            return progress;
        }

        // Get speed adjustment factor based on current progress
        float getSpeedAdjustment(const TimeProgress& progress) {
            if (progress.speedCompliance < 0.95f) {
                // Falling behind, speed up
                return timings.adjustedWPM * 1.1f;
            } else if (progress.speedCompliance > 1.05f) {
                // Going too fast, slow down
                return timings.adjustedWPM * 0.9f;
            }
            return timings.adjustedWPM;
        }

    private:
        TimeAllocation totalAllocation;
        ActivityTimings timings;
        float estimatedWords;

        void calculateActivityTimings() {
            DEBUG_PRINTLN("Calculating activity timings...");
            DEBUG_PRINT("Total millis: ");
            DEBUG_PRINTLN(totalAllocation.totalMillis);
            
            float totalMinutes = totalAllocation.totalMillis / (float)MILLIS_PER_MINUTE;
            timings.baseWPM = estimatedWords / totalMinutes;
            timings.adjustedWPM = timings.baseWPM * 
                                 totalAllocation.typingSpeedFactor;

            // Allocate time for different activities
            timings.typingMillis = totalAllocation.typingMillis * 0.70f;  // 70% pure typing
            timings.navigationMillis = totalAllocation.typingMillis * 0.10f; // 10% navigation
            timings.thinkingPauseMillis = totalAllocation.typingMillis * 0.10f; // 10% thinking
            timings.correctionMillis = totalAllocation.typingMillis * 0.05f; // 5% corrections
            timings.transitionMillis = totalAllocation.typingMillis * 0.05f; // 5% transitions
        }
    };
}