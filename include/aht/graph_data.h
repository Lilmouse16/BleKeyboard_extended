#pragma once
#include <Arduino.h>

namespace AHT {
    // Data structure for a point on the AHT graph
    struct GraphPoint {
        uint8_t videoDuration;  // In seconds
        uint16_t lowerBound;    // In minutes
        uint16_t upperBound;    // In minutes
        uint16_t targetAHT;     // In minutes
    };

    // Graph data points from the provided chart
    constexpr GraphPoint CURVE_POINTS[] = {
        //videoSec  lower  upper  target
        {   5,       30,    65,    47  },
        {  10,       60,   130,    95  },
        {  15,       90,   195,   142  },
        {  20,      120,   260,   190  },
        {  25,      150,   325,   237  },
        {  30,      180,   390,   285  },
        {  35,      210,   455,   332  },
        {  40,      240,   520,   380  },
        {  45,      270,   585,   427  }
    };

    constexpr size_t NUM_POINTS = sizeof(CURVE_POINTS) / sizeof(GraphPoint);

    // Minimum and maximum supported video durations
    constexpr uint8_t MIN_DURATION = CURVE_POINTS[0].videoDuration;              // 5 seconds
    constexpr uint8_t MAX_DURATION = CURVE_POINTS[NUM_POINTS - 1].videoDuration; // 45 seconds

    // Time conversion constants
    constexpr uint32_t SECONDS_PER_MINUTE = 60;
    constexpr uint32_t MILLIS_PER_SECOND = 1000;
    constexpr uint32_t MILLIS_PER_MINUTE = MILLIS_PER_SECOND * SECONDS_PER_MINUTE;

    // Default time distribution percentages
    constexpr uint8_t DEFAULT_TYPING_TIME_PERCENT = 80;    // 80% for typing
    constexpr uint8_t DEFAULT_RESERVED_TIME_PERCENT = 20;  // 20% for other activities

    // Difficulty weight factors (total = 100%)
    constexpr uint8_t TIME_DENSITY_WEIGHT = 20;     // 20% for time density
    constexpr uint8_t COMPLEXITY_WEIGHT = 30;       // 30% for timeframe complexity
    constexpr uint8_t CAMERA_ACTIONS_WEIGHT = 30;   // 30% for camera movements/transitions
    constexpr uint8_t TEXT_LENGTH_WEIGHT = 20;      // 20% for text length
}