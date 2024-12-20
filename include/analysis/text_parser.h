#pragma once
#include <Arduino.h>
#include <vector>
#include <SPIFFS.h>
#include "constants.h"

namespace Analysis {
    struct TimeFrame {
        struct TimeStamp {
            uint32_t minutes;
            uint32_t seconds;
            uint32_t milliseconds;
            
            uint32_t toMillis() const {
                return (minutes * 60 + seconds) * 1000 + milliseconds;
            }
            
            static TimeStamp fromString(const String& str);
        };

        enum class Type {
            TYPING,
            CAMERA_MOVEMENT,
            CAMERA_TRANSITION,
            CLIP_BOUNDARY
        };

        TimeStamp startTime;
        TimeStamp endTime;
        Type type;
        String content;
    };

    struct ClipData {
        int number;
        std::vector<TimeFrame> timeframes;
        String mainDescription;
        uint32_t totalDurationMillis;
        int wordCount;
        int charCount;
        int cameraMovements;
        int cameraTransitions;
        int actionDescriptions;
    };

    class TextParser {
    public:
        struct ParseResult {
            String videoId;
            std::vector<ClipData> clips;
            bool isValid;
            String errorMessage;
        };

        static ParseResult parseFile() {
            ParseResult result;
            result.isValid = false;
            
            File file = SPIFFS.open("/text.txt", "r");
            if (!file) {
                result.errorMessage = "Failed to open text.txt";
                return result;
            }

            // Read video ID from first line
            String line = file.readStringUntil('\n');
            if (line.startsWith("Video ")) {
                String idStr = line.substring(6);
                idStr.trim();
                result.videoId = idStr;
            } else {
                result.errorMessage = "Invalid file format: Missing Video ID";
                file.close();
                return result;
            }

            ClipData currentClip;
            bool inClip = false;
            String contentBuffer;

            // Process file line by line
            while (file.available()) {
                line = file.readStringUntil('\n');
                String trimmedLine = line;
                trimmedLine.trim();

                if (trimmedLine.startsWith("Clip #")) {
                    if (inClip) {
                        finalizeClip(currentClip);
                        result.clips.push_back(currentClip);
                    }
                    currentClip = ClipData();
                    inClip = true;
                    parseClipHeader(trimmedLine, currentClip);
                }
                else if (inClip) {
                    if (trimmedLine.indexOf('<') >= 0) {
                        if (!contentBuffer.isEmpty()) {
                            currentClip.mainDescription = contentBuffer;
                            contentBuffer = "";
                        }
                        parseTimeFrame(trimmedLine, currentClip);
                    }
                    else if (!trimmedLine.isEmpty()) {
                        contentBuffer += trimmedLine + "\n";
                    }
                }
            }

            // Handle last clip
            if (inClip) {
                finalizeClip(currentClip);
                result.clips.push_back(currentClip);
            }

            file.close();
            result.isValid = true;
            return result;
        }

    private:
        static void parseClipHeader(const String& line, ClipData& clip) {
            int numStart = line.indexOf('#') + 1;
            int numEnd = line.indexOf('<');
            clip.number = line.substring(numStart, numEnd).toInt();
            
            // Parse boundary timeframe
            TimeFrame boundary;
            boundary.type = TimeFrame::Type::CLIP_BOUNDARY;
            parseTimeStamps(line, boundary);
            clip.timeframes.push_back(boundary);
        }

        static void parseTimeFrame(const String& line, ClipData& clip) {
            TimeFrame frame;
            
            // Determine frame type
            if (line.indexOf("[CM]") >= 0) {
                frame.type = TimeFrame::Type::CAMERA_MOVEMENT;
                clip.cameraMovements++;
            } else if (line.indexOf("[CT]") >= 0) {
                frame.type = TimeFrame::Type::CAMERA_TRANSITION;
                clip.cameraTransitions++;
            } else {
                frame.type = TimeFrame::Type::TYPING;
                clip.actionDescriptions++;
            }

            parseTimeStamps(line, frame);
            
            // Extract content after timeframe
            int contentStart = line.indexOf('>') + 1;
            if (contentStart < line.length()) {
                String contentStr = line.substring(contentStart);
                contentStr.trim();
                frame.content = contentStr;
            }
            
            clip.timeframes.push_back(frame);
        }

        static void parseTimeStamps(const String& line, TimeFrame& frame) {
            int firstStart = line.indexOf('<') + 1;
            int firstEnd = line.indexOf('>');
            int secondStart = line.indexOf('<', firstEnd) + 1;
            int secondEnd = line.indexOf('>', secondStart);

            String startTimeStr = line.substring(firstStart, firstEnd);
            String endTimeStr = line.substring(secondStart, secondEnd);

            frame.startTime = TimeFrame::TimeStamp::fromString(startTimeStr);
            frame.endTime = TimeFrame::TimeStamp::fromString(endTimeStr);
        }

        static void finalizeClip(ClipData& clip) {
            // Calculate total duration
            if (!clip.timeframes.empty()) {
                const auto& firstFrame = clip.timeframes.front();
                const auto& lastFrame = clip.timeframes.back();
                clip.totalDurationMillis = lastFrame.endTime.toMillis() - 
                                         firstFrame.startTime.toMillis();
            }

            // Count words and characters in main description
            clip.wordCount = 0;
            clip.charCount = 0;
            bool inWord = false;

            for (char c : clip.mainDescription) {
                if (isAlphaNumeric(c)) {
                    if (!inWord) {
                        clip.wordCount++;
                        inWord = true;
                    }
                    clip.charCount++;
                } else {
                    inWord = false;
                }
            }
        }
    };
}
