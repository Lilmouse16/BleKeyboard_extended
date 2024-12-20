#pragma once
#include <Arduino.h>

namespace Utils {
    class TimeConversion {
    public:
        // Time components structure
        struct TimeComponents {
            uint16_t hours;
            uint8_t minutes;
            uint8_t seconds;
            uint16_t milliseconds;
            
            String toString(bool includeMs = false) const {
                char buffer[16];
                if (includeMs) {
                    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u.%03u",
                            hours, minutes, seconds, milliseconds);
                } else {
                    snprintf(buffer, sizeof(buffer), "%02u:%02u:%02u",
                            hours, minutes, seconds);
                }
                return String(buffer);
            }
        };

        // Convert milliseconds to time components
        static TimeComponents fromMillis(uint32_t milliseconds) {
            TimeComponents tc;
            
            tc.hours = milliseconds / (60UL * 60UL * 1000UL);
            milliseconds %= (60UL * 60UL * 1000UL);
            
            tc.minutes = milliseconds / (60UL * 1000UL);
            milliseconds %= (60UL * 1000UL);
            
            tc.seconds = milliseconds / 1000UL;
            tc.milliseconds = milliseconds % 1000UL;
            
            return tc;
        }

        // Convert time components to milliseconds
        static uint32_t toMillis(const TimeComponents& tc) {
            return (tc.hours * 60UL * 60UL * 1000UL) +
                   (tc.minutes * 60UL * 1000UL) +
                   (tc.seconds * 1000UL) +
                   tc.milliseconds;
        }

        // Parse time string in format "HH:MM:SS.mmm" or "MM:SS.mmm"
        static TimeComponents parseTimeString(const String& timeStr) {
            TimeComponents tc = {0};
            
            int firstColon = timeStr.indexOf(':');
            int secondColon = timeStr.indexOf(':', firstColon + 1);
            int dot = timeStr.indexOf('.');
            
            if (dot < 0) dot = timeStr.length();
            
            if (secondColon > 0) {
                // Format: HH:MM:SS.mmm
                tc.hours = timeStr.substring(0, firstColon).toInt();
                tc.minutes = timeStr.substring(firstColon + 1, secondColon).toInt();
                tc.seconds = timeStr.substring(secondColon + 1, dot).toInt();
            } else {
                // Format: MM:SS.mmm
                tc.hours = 0;
                tc.minutes = timeStr.substring(0, firstColon).toInt();
                tc.seconds = timeStr.substring(firstColon + 1, dot).toInt();
            }
            
            if (dot < timeStr.length()) {
                tc.milliseconds = timeStr.substring(dot + 1).toInt();
            }
            
            return tc;
        }

        // Format milliseconds as duration string
        static String formatDuration(uint32_t milliseconds, bool includeMs = false) {
            return fromMillis(milliseconds).toString(includeMs);
        }

        // Calculate time difference
        static uint32_t timeDiff(const TimeComponents& start, 
                                const TimeComponents& end) {
            return toMillis(end) - toMillis(start);
        }

        // Convert between different time units
        static uint32_t secondsToMillis(float seconds) {
            return static_cast<uint32_t>(seconds * 1000.0f);
        }

        static uint32_t minutesToMillis(float minutes) {
            return secondsToMillis(minutes * 60.0f);
        }

        static uint32_t hoursToMillis(float hours) {
            return minutesToMillis(hours * 60.0f);
        }

        static float millisToSeconds(uint32_t millis) {
            return millis / 1000.0f;
        }

        static float millisToMinutes(uint32_t millis) {
            return millisToSeconds(millis) / 60.0f;
        }

        static float millisToHours(uint32_t millis) {
            return millisToMinutes(millis) / 60.0f;
        }

        // Time arithmetic utilities
        static TimeComponents addTime(const TimeComponents& tc, uint32_t millisToAdd) {
            uint32_t totalMs = toMillis(tc) + millisToAdd;
            return fromMillis(totalMs);
        }

        static TimeComponents subtractTime(const TimeComponents& tc, 
                                         uint32_t millisToSubtract) {
            uint32_t totalMs = toMillis(tc);
            if (millisToSubtract > totalMs) {
                return {0, 0, 0, 0};
            }
            return fromMillis(totalMs - millisToSubtract);
        }

        // Validation utilities
        static bool isValidTimeComponents(const TimeComponents& tc) {
            return tc.minutes < 60 && 
                   tc.seconds < 60 && 
                   tc.milliseconds < 1000;
        }

        static bool isValidTimeString(const String& timeStr) {
            TimeComponents tc = parseTimeString(timeStr);
            return isValidTimeComponents(tc);
        }

        // Calculate average time
        static TimeComponents calculateAverage(const TimeComponents* times, 
                                            size_t count) {
            if (count == 0) return {0};
            
            uint64_t totalMs = 0;
            for (size_t i = 0; i < count; i++) {
                totalMs += toMillis(times[i]);
            }
            
            return fromMillis(totalMs / count);
        }
    };
}
