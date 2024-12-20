#pragma once
#include "analysis/text_parser.h"

namespace Analysis {
    // Structure to hold calculated metrics
    struct TaskMetrics {
        // Time Density Metrics
        float charsPerSecond;
        float wordsPerSecond;
        float averageWordLength;
        
        // Complexity Metrics
        float timeframesPerClip;
        float averageTimeframeDuration;
        float timeframeOverlapPercent;
        
        // Camera Action Metrics
        float cameraActionsPerClip;
        float cameraActionDensity;    // Actions per second
        float transitionFrequency;    // Transitions per minute
        
        // Text Length Metrics
        float averageWordsPerClip;
        float totalWords;
        float descriptionDensity;     // Words per timeframe
        
        // Overall Task Metrics
        uint32_t totalDurationMillis;
        int totalClips;
        int totalTimeframes;
    };

    class MetricsCalculator {
    public:
        static TaskMetrics calculate(const TextParser::ParseResult& parseResult) {
            TaskMetrics metrics = {};
            if (parseResult.clips.empty()) return metrics;

            // Initialize counters
            int totalWords = 0;
            int totalChars = 0;
            int totalTimeframes = 0;
            int totalCameraActions = 0;
            uint32_t totalDuration = 0;
            
            // Process each clip
            for (const auto& clip : parseResult.clips) {
                totalWords += clip.wordCount;
                totalChars += clip.charCount;
                totalTimeframes += clip.timeframes.size();
                totalCameraActions += clip.cameraMovements + clip.cameraTransitions;
                totalDuration = max(totalDuration, clip.totalDurationMillis);
            }

            // Store basic totals
            metrics.totalClips = parseResult.clips.size();
            metrics.totalTimeframes = totalTimeframes;
            metrics.totalDurationMillis = totalDuration;
            metrics.totalWords = totalWords;

            // Calculate time density metrics
            float durationSeconds = totalDuration / 1000.0f;
            metrics.charsPerSecond = totalChars / durationSeconds;
            metrics.wordsPerSecond = totalWords / durationSeconds;
            metrics.averageWordLength = totalWords > 0 ? 
                                      (float)totalChars / totalWords : 0;

            // Calculate complexity metrics
            metrics.timeframesPerClip = (float)totalTimeframes / metrics.totalClips;
            metrics.averageTimeframeDuration = calculateAverageTimeframeDuration(parseResult);
            metrics.timeframeOverlapPercent = calculateTimeframeOverlap(parseResult);

            // Calculate camera action metrics
            metrics.cameraActionsPerClip = (float)totalCameraActions / metrics.totalClips;
            metrics.cameraActionDensity = totalCameraActions / durationSeconds;
            metrics.transitionFrequency = calculateTransitionFrequency(parseResult);

            // Calculate text length metrics
            metrics.averageWordsPerClip = (float)totalWords / metrics.totalClips;
            metrics.descriptionDensity = (float)totalWords / totalTimeframes;

            return metrics;
        }

    private:
        static float calculateAverageTimeframeDuration(const TextParser::ParseResult& parseResult) {
            uint32_t totalDuration = 0;
            int count = 0;

            for (const auto& clip : parseResult.clips) {
                for (const auto& timeframe : clip.timeframes) {
                    totalDuration += timeframe.getDurationMillis();
                    count++;
                }
            }

            return count > 0 ? (totalDuration / 1000.0f) / count : 0;
        }

        static float calculateTimeframeOverlap(const TextParser::ParseResult& parseResult) {
            uint32_t totalOverlap = 0;
            uint32_t totalTime = 0;

            for (const auto& clip : parseResult.clips) {
                for (size_t i = 0; i < clip.timeframes.size(); i++) {
                    const auto& current = clip.timeframes[i];
                    totalTime += current.getDurationMillis();

                    // Check for overlap with next timeframe
                    if (i < clip.timeframes.size() - 1) {
                        const auto& next = clip.timeframes[i + 1];
                        if (current.endTime.toMillis() > next.startTime.toMillis()) {
                            totalOverlap += current.endTime.toMillis() - 
                                          next.startTime.toMillis();
                        }
                    }
                }
            }

            return totalTime > 0 ? 
                   (float)totalOverlap / totalTime * 100.0f : 0;
        }

        static float calculateTransitionFrequency(const TextParser::ParseResult& parseResult) {
            int totalTransitions = 0;
            for (const auto& clip : parseResult.clips) {
                totalTransitions += clip.cameraTransitions;
            }

            float durationMinutes = parseResult.clips.back().totalDurationMillis / 
                                  (1000.0f * 60.0f);
            return durationMinutes > 0 ? totalTransitions / durationMinutes : 0;
        }
    };
}