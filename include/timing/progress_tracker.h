#pragma once
#include <Arduino.h>
#include "timing/duration_calculator.h"
#include "aht/calculator.h"

namespace Timing {
    struct ProgressSnapshot {
        // Overall Progress
        float percentComplete = 0.0f;      // 0-100%
        uint32_t elapsedMillis = 0;        // Total time elapsed
        uint32_t estimatedRemaining = 0;   // Estimated time to completion
        float currentSpeed = 0.0f;         // Current WPM

        // Component Progress
        struct {
            float typing = 0.0f;           // % of typing complete
            float navigation = 0.0f;       // % of navigation complete
            float thinking = 0.0f;         // % of thinking pauses used
            float transitions = 0.0f;      // % of transitions complete
        } components;

        // Target Compliance
        struct {
            float speedDeviation = 0.0f;   // % deviation from target speed
            float timeUtilization = 0.0f;  // % of allocated time used
            float etaDeviation = 0.0f;     // % deviation from target completion
        } compliance;

        // Status Flags
        bool isBehindSchedule = false;
        bool isAheadOfSchedule = false;
        bool needsSpeedAdjustment = false;
    };

    class ProgressTracker {
    public:
        ProgressTracker(const DurationAnalysis& duration)
            : videoDuration(duration)
            , activityProgress{}
            , startTime(0)
            , lastPauseTime(0)
            , totalPausedTime(0)
            , isRunning(false) {
        }

        void start() {
            if (!isRunning) {
                startTime = millis();
                isRunning = true;
            }
        }

        void pause() {
            if (isRunning) {
                lastPauseTime = millis();
                isRunning = false;
            }
        }

        void resume() {
            if (!isRunning && lastPauseTime > 0) {
                totalPausedTime += millis() - lastPauseTime;
                lastPauseTime = 0;
                isRunning = true;
            }
        }

        void updateActivity(AHT::ActivityType activity, uint32_t millisUsed) {
            if (!isRunning) return;

            switch (activity) {
                case AHT::ActivityType::TYPING:
                    activityProgress.typingMillis += millisUsed;
                    break;
                case AHT::ActivityType::NAVIGATION:
                    activityProgress.navigationMillis += millisUsed;
                    break;
                case AHT::ActivityType::THINKING:
                    activityProgress.thinkingMillis += millisUsed;
                    break;
                case AHT::ActivityType::TRANSITION:
                    activityProgress.transitionMillis += millisUsed;
                    break;
            }
        }

        void updateWordsTyped(uint32_t count) {
            activityProgress.wordsTyped = count;
        }

        ProgressSnapshot getSnapshot() const {
            ProgressSnapshot snapshot;
            if (!isRunning || startTime == 0) return snapshot;

            calculateElapsedTime(snapshot);
            calculateProgress(snapshot);
            calculateCompliance(snapshot);
            calculateETA(snapshot);
            updateStatusFlags(snapshot);

            return snapshot;
        }

    private:
        struct ActivityProgress {
            uint32_t typingMillis = 0;
            uint32_t navigationMillis = 0;
            uint32_t thinkingMillis = 0;
            uint32_t transitionMillis = 0;
            uint32_t wordsTyped = 0;
        };

        const DurationAnalysis& videoDuration;
        ActivityProgress activityProgress;

        uint32_t startTime;
        uint32_t lastPauseTime;
        uint32_t totalPausedTime;
        bool isRunning;

        void calculateElapsedTime(ProgressSnapshot& snapshot) const {
            uint32_t currentTime = millis();
            snapshot.elapsedMillis = currentTime - startTime - totalPausedTime;
        }

        void calculateProgress(ProgressSnapshot& snapshot) const {
            // Overall progress
            snapshot.percentComplete = std::min(100.0f, 
                (float)snapshot.elapsedMillis / videoDuration.totalMillis * 100.0f);

            // Component progress
            snapshot.components.typing = calculateComponentProgress(
                activityProgress.typingMillis, videoDuration.typingMillis);
            
            snapshot.components.navigation = calculateComponentProgress(
                activityProgress.navigationMillis, 
                videoDuration.totalMillis - videoDuration.typingMillis);
            
            snapshot.components.thinking = calculateComponentProgress(
                activityProgress.thinkingMillis, 
                videoDuration.totalMillis * 0.1f);  // 10% for thinking
            
            snapshot.components.transitions = calculateComponentProgress(
                activityProgress.transitionMillis,
                videoDuration.totalMillis * 0.05f);  // 5% for transitions
        }

        float calculateComponentProgress(uint32_t used, uint32_t allocated) const {
            return allocated > 0 ? 
                   std::min(100.0f, (float)used / allocated * 100.0f) : 0.0f;
        }

        void calculateCompliance(ProgressSnapshot& snapshot) const {
            // Speed compliance
            float expectedWords = activityProgress.wordsTyped * 
                                (snapshot.elapsedMillis / (float)videoDuration.totalMillis);
            snapshot.compliance.speedDeviation = 
                ((float)activityProgress.wordsTyped - expectedWords) / expectedWords * 100.0f;

            // Time utilization
            float expectedElapsed = videoDuration.totalMillis * 
                                  (snapshot.percentComplete / 100.0f);
            snapshot.compliance.timeUtilization = 
                snapshot.elapsedMillis / expectedElapsed * 100.0f;
        }

        void calculateETA(ProgressSnapshot& snapshot) const {
            if (snapshot.percentComplete <= 0) {
                snapshot.estimatedRemaining = videoDuration.totalMillis;
                return;
            }

            float completionRate = snapshot.percentComplete / snapshot.elapsedMillis;
            uint32_t estimatedTotal = static_cast<uint32_t>(100.0f / completionRate);
            snapshot.estimatedRemaining = 
                std::max(0UL, static_cast<unsigned long>(estimatedTotal - snapshot.elapsedMillis));

            float targetRemaining = videoDuration.totalMillis - snapshot.elapsedMillis;
            snapshot.compliance.etaDeviation = 
                (snapshot.estimatedRemaining - targetRemaining) / targetRemaining * 100.0f;
        }

        void updateStatusFlags(ProgressSnapshot& snapshot) const {
            snapshot.isBehindSchedule = snapshot.compliance.timeUtilization > 110.0f;
            snapshot.isAheadOfSchedule = snapshot.compliance.timeUtilization < 90.0f;
            snapshot.needsSpeedAdjustment = 
                std::abs(snapshot.compliance.speedDeviation) > 10.0f;
        }
    };
}
