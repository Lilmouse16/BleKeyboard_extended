#pragma once
#include "analysis/text_parser.h"

namespace Timing {
    struct DurationAnalysis {
        uint32_t totalMillis;          // Total video duration
        uint32_t effectiveMillis;      // Duration excluding gaps
        uint32_t overlapMillis;        // Total overlap time
        uint32_t gapMillis;            // Total gap time
        uint32_t typingMillis;         // Time spent typing
        float utilizationPercent;      // Effective/Total ratio
        
        struct TimeRange {
            uint32_t startMillis;
            uint32_t endMillis;
        };
        std::vector<TimeRange> gaps;    // List of timing gaps
        std::vector<TimeRange> overlaps; // List of overlapping sections
    };

    class DurationCalculator {
    public:
        static DurationAnalysis analyze(const Analysis::TextParser::ParseResult& parseResult);
        static bool validateTiming(const Analysis::TextParser::ParseResult& parseResult, 
                                 String& errorMessage);
    private:
        // ... rest of the implementation
    };
}