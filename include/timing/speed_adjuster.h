#pragma once
#include <Arduino.h>
#include "timing/progress_tracker.h"
#include "constants.h"

namespace {
    template<typename T>
    T clamp(T value, T min, T max) {
        return value < min ? min : (value > max ? max : value);
    }
}

namespace Timing {
    struct SpeedConfig {
        float baseWPM = Constants::Typing::BASE_WPM;
        float minSpeedFactor = Constants::Typing::MIN_SPEED_MULTIPLIER;
        float maxSpeedFactor = Constants::Typing::MAX_SPEED_MULTIPLIER;
        float fatigueImpact = Constants::HumanBehavior::FATIGUE_SPEED_IMPACT;
    };

    struct SpeedAdjustment {
        float speedFactor;         // Current speed multiplier
        float adjustedWPM;         // Actual WPM after adjustment
        float effectiveFatigue;    // Current effective fatigue
        bool isAtMaxSpeed;         // Whether max speed reached
        bool isAtMinSpeed;         // Whether min speed reached
        bool needsBreak;          // Whether a break is recommended
    };

    class SpeedAdjuster {
    public:
        SpeedAdjuster(const SpeedConfig& config)
            : config(config)
            , currentSpeedFactor(1.0f)
            , currentFatigue(0.0f)
            , lastAdjustmentTime(0)
            , lastFatigueUpdate(0)
            , consecutiveFastPeriods(0)
            , consecutiveSlowPeriods(0) {
        }

        void reset() {
            currentSpeedFactor = 1.0f;
            currentFatigue = 0.0f;
            lastAdjustmentTime = millis();
            lastFatigueUpdate = millis();
            consecutiveFastPeriods = 0;
            consecutiveSlowPeriods = 0;
        }

        SpeedAdjustment updateSpeed(const ProgressSnapshot& progress) {
            uint32_t currentTime = millis();
            updateFatigue(currentTime);
            
            SpeedAdjustment adjustment;
            calculateSpeedAdjustment(progress, adjustment);
            applyFatigueImpact(adjustment);
            enforceSpeedLimits(adjustment);
            updateSpeedTracking(adjustment);
            
            lastAdjustmentTime = currentTime;
            return adjustment;
        }

        float getCurrentFatigue() const { return currentFatigue; }
        float getCurrentSpeedFactor() const { return currentSpeedFactor; }

        void addFatigue(float amount) {
            currentFatigue = std::min(currentFatigue + amount, 1.0f);
        }

        void applyRecovery(float amount) {
            currentFatigue = std::max(0.0f, currentFatigue - amount);
        }

    private:
        SpeedConfig config;
        float currentSpeedFactor;
        float currentFatigue;
        uint32_t lastAdjustmentTime;
        uint32_t lastFatigueUpdate;
        int consecutiveFastPeriods;
        int consecutiveSlowPeriods;

        void updateFatigue(uint32_t currentTime) {
            float deltaSeconds = (currentTime - lastFatigueUpdate) / 1000.0f;
            if (deltaSeconds > 0) {
                float recovery = Constants::HumanBehavior::RECOVERY_RATE * deltaSeconds;
                applyRecovery(recovery);
            }
            lastFatigueUpdate = currentTime;
        }

        void calculateSpeedAdjustment(const ProgressSnapshot& progress, 
                                    SpeedAdjustment& adjustment) {
            float targetAdjustment = 1.0f;

            // Adjust for progress
            if (progress.isBehindSchedule) {
                targetAdjustment *= 1.1f;
                consecutiveSlowPeriods++;
                consecutiveFastPeriods = 0;
            } else if (progress.isAheadOfSchedule) {
                targetAdjustment *= 0.9f;
                consecutiveFastPeriods++;
                consecutiveSlowPeriods = 0;
            } else {
                consecutiveFastPeriods = 0;
                consecutiveSlowPeriods = 0;
            }

            // Smooth the transition
            float maxChange = 0.1f;
            float delta = targetAdjustment - currentSpeedFactor;
            float change = clamp(delta, -maxChange, maxChange);
            currentSpeedFactor += change;
        }

        void applyFatigueImpact(SpeedAdjustment& adjustment) {
            float fatigueModifier = 1.0f - (currentFatigue * config.fatigueImpact);
            adjustment.speedFactor = currentSpeedFactor * fatigueModifier;
            adjustment.effectiveFatigue = currentFatigue;
        }

        void enforceSpeedLimits(SpeedAdjustment& adjustment) {
            adjustment.speedFactor = clamp(
                adjustment.speedFactor,
                config.minSpeedFactor,
                config.maxSpeedFactor
            );

            adjustment.adjustedWPM = config.baseWPM * adjustment.speedFactor;
            adjustment.isAtMaxSpeed = adjustment.speedFactor >= config.maxSpeedFactor;
            adjustment.isAtMinSpeed = adjustment.speedFactor <= config.minSpeedFactor;
        }

        void updateSpeedTracking(SpeedAdjustment& adjustment) {
            // Recommend breaks based on sustained high speed or fatigue
            adjustment.needsBreak = 
                (consecutiveFastPeriods >= 5) ||
                (currentFatigue > Constants::HumanBehavior::MAX_FATIGUE_LEVEL * 0.8f);
        }
    };
}