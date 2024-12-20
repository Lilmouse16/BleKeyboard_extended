#pragma once
#include "analysis/metrics_calculator.h"
#include "aht/graph_data.h"

namespace Analysis {
    // Detailed breakdown of difficulty scores
    struct DifficultyScores {
        // Time Density (20%)
        float timeDensityScore;     // Based on chars/sec and words/sec
        
        // Complexity (30%)
        float complexityScore;       // Based on timeframes and overlaps
        
        // Camera Actions (30%)
        float cameraActionScore;     // Based on movements and transitions
        
        // Text Length (20%)
        float textLengthScore;       // Based on word count and distribution

        // Final Scores
        float finalScore;            // Weighted combination
        float normalizedScore;       // 0.0 to 1.0 scale
        
        // Component Details
        struct {
            float charsPerSecond;
            float wordsPerSecond;
            float timeframeOverlap;
            float actionsPerMinute;
            float wordsPerClip;
        } metrics;
    };

    class DifficultyScorer {
    public:
        // Reference values for scoring
        struct ReferenceValues {
            // Time Density
            static constexpr float MIN_CHARS_PER_SEC = 1.0f;
            static constexpr float MAX_CHARS_PER_SEC = 5.0f;
            static constexpr float MIN_WORDS_PER_SEC = 0.2f;
            static constexpr float MAX_WORDS_PER_SEC = 1.0f;

            // Complexity
            static constexpr float MIN_TIMEFRAMES_PER_CLIP = 1.0f;
            static constexpr float MAX_TIMEFRAMES_PER_CLIP = 5.0f;
            static constexpr float MIN_OVERLAP_PERCENT = 0.0f;
            static constexpr float MAX_OVERLAP_PERCENT = 30.0f;

            // Camera Actions
            static constexpr float MIN_ACTIONS_PER_CLIP = 0.0f;
            static constexpr float MAX_ACTIONS_PER_CLIP = 3.0f;
            static constexpr float MIN_TRANSITIONS_PER_MIN = 0.0f;
            static constexpr float MAX_TRANSITIONS_PER_MIN = 4.0f;

            // Text Length
            static constexpr float MIN_WORDS_PER_CLIP = 20.0f;
            static constexpr float MAX_WORDS_PER_CLIP = 200.0f;
            static constexpr float MIN_DESC_DENSITY = 5.0f;
            static constexpr float MAX_DESC_DENSITY = 50.0f;
        };

        static DifficultyScores calculate(const TaskMetrics& metrics) {
            DifficultyScores scores = {};

            // Store raw metrics for reference
            scores.metrics.charsPerSecond = metrics.charsPerSecond;
            scores.metrics.wordsPerSecond = metrics.wordsPerSecond;
            scores.metrics.timeframeOverlap = metrics.timeframeOverlapPercent;
            scores.metrics.actionsPerMinute = metrics.cameraActionDensity * 60.0f;
            scores.metrics.wordsPerClip = metrics.averageWordsPerClip;

            // Calculate component scores
            scores.timeDensityScore = calculateTimeDensityScore(metrics);
            scores.complexityScore = calculateComplexityScore(metrics);
            scores.cameraActionScore = calculateCameraActionScore(metrics);
            scores.textLengthScore = calculateTextLengthScore(metrics);

            // Calculate final weighted score
            scores.finalScore = 
                (scores.timeDensityScore * AHT::TIME_DENSITY_WEIGHT +
                 scores.complexityScore * AHT::COMPLEXITY_WEIGHT +
                 scores.cameraActionScore * AHT::CAMERA_ACTIONS_WEIGHT +
                 scores.textLengthScore * AHT::TEXT_LENGTH_WEIGHT) / 100.0f;

            // Normalize to 0.0 - 1.0 range
            scores.normalizedScore = constrain(scores.finalScore / 10.0f, 0.0f, 1.0f);

            return scores;
        }

    private:
        static float normalizeValue(float value, float min, float max) {
            return constrain((value - min) / (max - min), 0.0f, 1.0f);
        }

        static float calculateTimeDensityScore(const TaskMetrics& metrics) {
            float charScore = normalizeValue(
                metrics.charsPerSecond,
                ReferenceValues::MIN_CHARS_PER_SEC,
                ReferenceValues::MAX_CHARS_PER_SEC
            );

            float wordScore = normalizeValue(
                metrics.wordsPerSecond,
                ReferenceValues::MIN_WORDS_PER_SEC,
                ReferenceValues::MAX_WORDS_PER_SEC
            );

            return (charScore * 0.6f + wordScore * 0.4f) * 10.0f;
        }

        static float calculateComplexityScore(const TaskMetrics& metrics) {
            float timeframeScore = normalizeValue(
                metrics.timeframesPerClip,
                ReferenceValues::MIN_TIMEFRAMES_PER_CLIP,
                ReferenceValues::MAX_TIMEFRAMES_PER_CLIP
            );

            float overlapScore = normalizeValue(
                metrics.timeframeOverlapPercent,
                ReferenceValues::MIN_OVERLAP_PERCENT,
                ReferenceValues::MAX_OVERLAP_PERCENT
            );

            return (timeframeScore * 0.7f + overlapScore * 0.3f) * 10.0f;
        }

        static float calculateCameraActionScore(const TaskMetrics& metrics) {
            float actionScore = normalizeValue(
                metrics.cameraActionsPerClip,
                ReferenceValues::MIN_ACTIONS_PER_CLIP,
                ReferenceValues::MAX_ACTIONS_PER_CLIP
            );

            float transitionScore = normalizeValue(
                metrics.transitionFrequency,
                ReferenceValues::MIN_TRANSITIONS_PER_MIN,
                ReferenceValues::MAX_TRANSITIONS_PER_MIN
            );

            return (actionScore * 0.5f + transitionScore * 0.5f) * 10.0f;
        }

        static float calculateTextLengthScore(const TaskMetrics& metrics) {
            float wordCountScore = normalizeValue(
                metrics.averageWordsPerClip,
                ReferenceValues::MIN_WORDS_PER_CLIP,
                ReferenceValues::MAX_WORDS_PER_CLIP
            );

            float densityScore = normalizeValue(
                metrics.descriptionDensity,
                ReferenceValues::MIN_DESC_DENSITY,
                ReferenceValues::MAX_DESC_DENSITY
            );

            return (wordCountScore * 0.8f + densityScore * 0.2f) * 10.0f;
        }
    };
}