#pragma once
#include <Arduino.h>
#include "analysis/text_parser.h"

namespace TimeAnalysis {
    struct DurationAnalysis {
        uint32_t totalMillis = 0;         // Total video duration
        uint32_t effectiveMillis = 0;     // Duration excluding gaps
        uint32_t overlapMillis = 0;       // Total overlap time
        uint32_t gapMillis = 0;           // Total gap time
        float utilizationPercent = 0.0f;  // Effective/Total ratio
        
        struct TimeRange {
            uint32_t startMillis;
            uint32_t endMillis;
            
            uint32_t duration() const {
                return endMillis - startMillis;
            }
        };

        std::vector<TimeRange> gaps;      // List of timing gaps
        std::vector<TimeRange> overlaps;  // List of overlapping sections
    };

    class DurationCalculator {
    public:
        static DurationAnalysis analyze(const Analysis::TextParser::ParseResult& parseResult) {
            DurationAnalysis analysis;
            if (parseResult.clips.empty()) return analysis;

            findGlobalTimeRange(parseResult, analysis);
            analyzeTimeframes(parseResult, analysis);
            calculateUtilization(analysis);
            
            return analysis;
        }

        static bool validateTiming(const Analysis::TextParser::ParseResult& parseResult, 
                                 String& errorMessage) {
            if (parseResult.clips.empty()) {
                errorMessage = "No clips found";
                return false;
            }

            return validateClipSequence(parseResult, errorMessage) && 
                   validateTimeframes(parseResult, errorMessage);
        }

    private:
        static void findGlobalTimeRange(
            const Analysis::TextParser::ParseResult& parseResult,
            DurationAnalysis& analysis) {
            
            uint32_t globalStart = UINT32_MAX;
            uint32_t globalEnd = 0;

            for (const auto& clip : parseResult.clips) {
                for (const auto& frame : clip.timeframes) {
                    uint32_t startMs = frame.startTime.toMillis();
                    uint32_t endMs = frame.endTime.toMillis();
                    
                    globalStart = std::min(globalStart, startMs);
                    globalEnd = std::max(globalEnd, endMs);
                }
            }

            analysis.totalMillis = globalEnd - globalStart;
        }

        static void analyzeTimeframes(
            const Analysis::TextParser::ParseResult& parseResult,
            DurationAnalysis& analysis) {
            
            std::vector<std::pair<uint32_t, int>> events;  // time, +1/-1 for start/end
            
            // Build timeline of events
            for (const auto& clip : parseResult.clips) {
                for (const auto& frame : clip.timeframes) {
                    events.push_back(std::make_pair(frame.startTime.toMillis(), 1));
                    events.push_back(std::make_pair(frame.endTime.toMillis(), -1));
                }
            }

            // Sort events chronologically
            std::sort(events.begin(), events.end());

            // Analyze timeline
            int activeFrames = 0;
            uint32_t lastTime = events[0].first;
            uint32_t coveredTime = 0;
            DurationAnalysis::TimeRange currentRange = {0, 0};

            for (const auto& event : events) {
                uint32_t currentTime = event.first;
                
                if (activeFrames > 0) {
                    coveredTime += currentTime - lastTime;
                }

                // Handle overlaps
                if (activeFrames > 1) {
                    analysis.overlapMillis += currentTime - lastTime;
                    if (currentRange.startMillis == 0) {
                        currentRange.startMillis = lastTime;
                    }
                } else if (activeFrames == 1 && currentRange.startMillis > 0) {
                    currentRange.endMillis = lastTime;
                    analysis.overlaps.push_back(currentRange);
                    currentRange = {0, 0};
                }

                // Handle gaps
                if (activeFrames == 0 && lastTime > events[0].first) {
                    DurationAnalysis::TimeRange gap = {lastTime, currentTime};
                    analysis.gaps.push_back(gap);
                    analysis.gapMillis += currentTime - lastTime;
                }

                activeFrames += event.second;
                lastTime = currentTime;
            }

            analysis.effectiveMillis = coveredTime;
        }

        static void calculateUtilization(DurationAnalysis& analysis) {
            analysis.utilizationPercent = analysis.totalMillis > 0 ? 
                (float)analysis.effectiveMillis / analysis.totalMillis * 100.0f : 0;
        }

        static bool validateClipSequence(
            const Analysis::TextParser::ParseResult& parseResult,
            String& errorMessage) {
            
            uint32_t lastEndTime = 0;
            int expectedClipNum = 1;

            for (const auto& clip : parseResult.clips) {
                if (clip.number != expectedClipNum) {
                    errorMessage = "Invalid clip numbering sequence";
                    return false;
                }

                uint32_t clipStartTime = UINT32_MAX;
                uint32_t clipEndTime = 0;

                for (const auto& frame : clip.timeframes) {
                    uint32_t startMs = frame.startTime.toMillis();
                    uint32_t endMs = frame.endTime.toMillis();

                    if (endMs <= startMs) {
                        errorMessage = "Invalid timeframe duration in clip " + 
                                     String(clip.number);
                        return false;
                    }

                    clipStartTime = std::min(clipStartTime, startMs);
                    clipEndTime = std::max(clipEndTime, endMs);
                }

                if (lastEndTime > 0 && clipStartTime < lastEndTime) {
                    errorMessage = "Clip " + String(clip.number) + 
                                 " overlaps with previous clip";
                    return false;
                }

                lastEndTime = clipEndTime;
                expectedClipNum++;
            }

            return true;
        }

        static bool validateTimeframes(
            const Analysis::TextParser::ParseResult& parseResult,
            String& errorMessage) {
            
            for (const auto& clip : parseResult.clips) {
                for (const auto& frame : clip.timeframes) {
                    if (frame.endTime.toMillis() <= frame.startTime.toMillis()) {
                        errorMessage = "Invalid timeframe in clip " + 
                                     String(clip.number);
                        return false;
                    }
                }
            }

            return true;
        }
    };
}

namespace AHT {
    struct TimeAllocation;  // Forward declaration
    
    struct CalculationResult {
        bool isValid;
        float targetMinutes;
        float lowerBoundMinutes;
        float upperBoundMinutes;
    };

    class Calculator {
    public:
        static CalculationResult calculate(float durationSeconds);
        static TimeAllocation calculateTimeAllocation(float targetMinutes, float typingPercentage);
    };
}
