#include "human_simulator.h"

void HumanSimulator::init() {
    // Initialize behavioral state
    behavior = BehaviorState{
        .fatigueLevel = 0.0f,
        .alertnessLevel = 1.0f,
        .confidenceLevel = 0.8f,
        .consecutiveErrors = 0,
        .wordsWithoutBreak = 0,
        .lastBreakTime = 0
    };

    // Configure speed settings
    speedConfig = SpeedConfig{
        .baseWPM = Constants::Typing::BASE_WPM,
        .minSpeedFactor = Constants::Timing::MIN_SPEED_MULTIPLIER,
        .maxSpeedFactor = Constants::Timing::MAX_SPEED_MULTIPLIER,
        .fatigueImpact = Constants::HumanBehavior::FATIGUE_SPEED_IMPACT
    };

    // Configure behavior settings
    behaviorConfig = BehaviorConfig{
        .typoChance = Constants::HumanBehavior::TYPO_CHANCE,
        .correctionChance = 1.0f - Constants::HumanBehavior::UNCORRECTED_TYPO_CHANCE,
        .thinkingFrequency = Constants::HumanBehavior::THINKING_PAUSE_CHANCE / 100.0f,
        .recoveryRate = Constants::HumanBehavior::RECOVERY_RATE,
        .maxWordsBeforeBreak = 20
    };

    sessionStartTime = millis();
    lastActivityTime = sessionStartTime;
    isPaused = true;
    reset();
}

void HumanSimulator::reset() {
    currentWord = "";
    wordsInBurst = 0;
    totalClips = 0;
    countClips();
    
    // Reset performance metrics
    metrics = {
        .averageWPM = 0.0f,
        .currentWPM = 0.0f,
        .errorRate = 0.0f,
        .correctionRate = 0.0f,
        .speedCompliance = 1.0f,
        .timeUtilization = 0.0f
    };

    Serial.println("Human simulator reset complete");
}

void HumanSimulator::loadTask(const String& videoId) {
    taskInfo.videoId = videoId;
    countClips();
    
    if (totalClips > 0) {
        auto parseResult = Analysis::TextParser::parseFile();
        if (!parseResult.isValid) {
            hardware.setError(true, "Failed to parse video times");
            return;
        }

        auto durationAnalysis = Timing::DurationCalculator::analyze(parseResult);
        taskInfo.totalDurationMs = durationAnalysis.totalMillis;
        
        // Initialize progress tracker with duration analysis
        progressTracker.reset(new Timing::ProgressTracker(durationAnalysis));
        
        // Configure speed adjuster
        Timing::SpeedConfig speedCfg;
        speedCfg.baseWPM = speedConfig.baseWPM;
        speedCfg.minSpeedFactor = speedConfig.minSpeedFactor;
        speedCfg.maxSpeedFactor = speedConfig.maxSpeedFactor;
        speedAdjuster.reset(new Timing::SpeedAdjuster(speedCfg));
        
        Serial.printf("Task loaded: %s, Duration: %.1f seconds, Target AHT: %.1f minutes\n",
                     videoId.c_str(),
                     taskInfo.totalDurationMs / 1000.0f,
                     taskInfo.targetAHT);
    }
}

void HumanSimulator::processClip(int clipNumber) {
    if (!validateClipNumber(clipNumber)) return;

    Serial.printf("\n=== Processing Clip %d ===\n", clipNumber);
    taskInfo.currentClip = clipNumber;

    // Start progress tracking
    progressTracker->start();
    hardware.updateProgress(progressTracker->getSnapshot());

    // Navigate to clip
    navigateToClip(clipNumber);
    if (isPaused) return;

    // Process clip content
    String content = readClipContent(clipNumber);
    if (content.length() > 0 && !isPaused) {
        std::vector<Analysis::TimeFrame> frames = parseTimeframes(content);
        for (const auto& frame : frames) {
            processTimeframe(frame);
            if (isPaused) break;
        }
    }

    // Update completion status
    if (!isPaused) {
        hardware.setSectionComplete(true);
        hardware.playSound(Hardware::SoundType::SECTION_COMPLETE);
        logProgress();
    }
}

void HumanSimulator::processTimeframe(const Analysis::TimeFrame& frame) {
    progressTracker->start();

    switch (frame.type) {
        case Analysis::TimeFrame::Type::CAMERA_MOVEMENT:
            // Handle camera movement
            break;
        case Analysis::TimeFrame::Type::CAMERA_TRANSITION:
            // Handle transition
            break;
        case Analysis::TimeFrame::Type::TYPING:
            if (!frame.content.isEmpty()) {
                typeText(frame.content);
            }
            break;
        default:
            break;
    }
}

void HumanSimulator::typeText(const String& text) {
    if (text.isEmpty()) return;

    currentWord = "";
    wordsInBurst = 0;
    
    for (char c : text) {
        if (isPaused) return;

        // Handle word boundaries
        if (c == ' ' || c == '\n') {
            if (!currentWord.isEmpty()) {
                handleWord(currentWord);
                currentWord = "";
                wordsInBurst++;
                
                // Check for natural breaks
                if (wordsInBurst >= behaviorConfig.maxWordsBeforeBreak) {
                    simulateThinking();
                    wordsInBurst = 0;
                }
            }

            // Handle possible double space
            if (random(100) < Constants::HumanBehavior::DOUBLE_SPACE_CHANCE * 100) {
                keyboard.type(" ");
                simulateTypingDelay();
            }
            
            keyboard.type(String(c));
            simulateTypingDelay();
            
        } else {
            currentWord += c;
        }

        // Update simulation state
        applyFatigue();
        updatePerformanceMetrics();
        adjustTypingSpeed();
        
        // Handle natural pauses
        handleNaturalPauses(text);
    }

    // Handle final word
    if (!currentWord.isEmpty() && !isPaused) {
        handleWord(currentWord);
    }
}

void HumanSimulator::handleWord(const String& word) {
    // Calculate typo probability
    float typoChance = behaviorConfig.typoChance;
    typoChance *= (1.0f + behavior.fatigueLevel);  // Increase with fatigue
    typoChance *= (2.0f - behavior.alertnessLevel);  // Decrease with alertness

    if (random(100) < typoChance * 100) {
        makeTypo(word);
    } else {
        typeWordNormally(word);
    }

    // Update behavioral state
    behavior.wordsWithoutBreak++;
    updateAlertness();
}

void HumanSimulator::makeTypo(const String& word) {
    int wordLen = word.length();
    int typoPos = random(wordLen);
    
    // Type up to typo
    for (int i = 0; i < typoPos; i++) {
        if (isPaused) return;
        keyboard.type(String(word[i]));
        simulateTypingDelay();
    }
    
    // Make typo
    char wrongChar = getRandomTypo(word[typoPos]);
    keyboard.type(String(wrongChar));
    
    // Decide whether to correct
    bool shouldCorrect = decideCorrectionStrategy(word, typoPos);
    
    if (shouldCorrect && !isPaused) {
        correctTypo(word, typoPos);
    } else {
        // Continue with remaining characters
        keyboard.type(word.substring(typoPos + 1));
    }

    // Update error tracking
    behavior.consecutiveErrors++;
    metrics.errorRate = (behavior.consecutiveErrors * 1.0f) / metrics.averageWPM;
}

void HumanSimulator::correctTypo(const String& word, int typoPos) {
    delay(Constants::Typing::CORRECTION_DELAY);
    keyboard.pressKey(KEY_BACKSPACE);
    delay(Constants::Typing::CORRECTION_DELAY);
    keyboard.type(String(word[typoPos]));
    
    // Complete the word
    for (int i = typoPos + 1; i < word.length(); i++) {
        if (isPaused) return;
        keyboard.type(String(word[i]));
        simulateTypingDelay();
    }

    metrics.correctionRate++;
}

bool HumanSimulator::decideCorrectionStrategy(const String& word, int typoPos) {
    // Base correction probability
    float correctionProb = behaviorConfig.correctionChance;
    
    // Adjust based on word length
    if (word.length() < Constants::HumanBehavior::UNCORRECTED_TYPO_THRESHOLD) {
        correctionProb += 0.2f;
    }
    
    // Adjust based on position in word
    if (typoPos < 2) {
        correctionProb += 0.15f;  // More likely to correct start of word
    }
    
    // Adjust based on alertness
    correctionProb *= behavior.alertnessLevel;
    
    return random(100) < correctionProb * 100;
}

void HumanSimulator::simulateTypingDelay() {
    if (isPaused) return;
    
    // Calculate base delay
    float speedFactor = speedAdjuster->getCurrentSpeedFactor();
    int baseDelay = Constants::Typing::BASE_CHAR_DELAY / speedFactor;
    
    // Apply modifiers
    float fatigueModifier = 1.0f + (behavior.fatigueLevel * speedConfig.fatigueImpact);
    float alertnessModifier = 0.8f + (behavior.alertnessLevel * 0.4f);
    
    // Add natural variance
    int finalDelay = baseDelay * fatigueModifier * alertnessModifier;
    finalDelay += random(-finalDelay/4, finalDelay/4);
    
    delay(max(finalDelay, Constants::Typing::BASE_CHAR_DELAY/2));
}

void HumanSimulator::simulateThinking() {
    if (isPaused) return;
    
    if (random(100) < behaviorConfig.thinkingFrequency * 100) {
        int thinkingTime = random(
            Constants::HumanBehavior::MIN_THINKING_PAUSE,
            Constants::HumanBehavior::MAX_THINKING_PAUSE
        );
        
        delay(thinkingTime);
        behavior.lastBreakTime = millis();
        behavior.wordsWithoutBreak = 0;
        
        // Recovery during break
        behavior.fatigueLevel = max(0.0f, 
            behavior.fatigueLevel - behaviorConfig.recoveryRate);
    }
}

void HumanSimulator::handleNaturalPauses(const String& text) {
    // Pause at punctuation
    if (text.indexOf('.') >= 0 || text.indexOf('!') >= 0 || text.indexOf('?') >= 0) {
        delay(Constants::Typing::SENTENCE_PAUSE);
    }
    // Pause at commas
    else if (text.indexOf(',') >= 0) {
        delay(Constants::Typing::WORD_PAUSE);
    }
}

void HumanSimulator::navigateToClip(int clipNumber) {
    int tabCount = (clipNumber == 1) ? 
        Constants::Navigation::FIRST_CLIP_TAB_COUNT :
        Constants::Navigation::NEXT_CLIP_TAB_COUNT;
    
    keyboard.navigateWithSpeed(tabCount, 
        behavior.alertnessLevel * speedAdjuster->getCurrentSpeedFactor());
}

void HumanSimulator::updatePerformanceMetrics() {
    auto snapshot = progressTracker->getSnapshot();
    
    metrics.currentWPM = keyboard.getTypingStats().currentWPM;
    metrics.averageWPM = keyboard.getTypingStats().averageWPM;
    metrics.speedCompliance = snapshot.compliance.speedDeviation;
    metrics.timeUtilization = snapshot.compliance.timeUtilization;
    
    // Update hardware display
    hardware.updateProgress(snapshot);
}

void HumanSimulator::adjustTypingSpeed() {
    auto progress = progressTracker->getSnapshot();
    auto adjustment = speedAdjuster->updateSpeed(progress);
    
    float speedAdjustment = adjustment.speedFactor;
    speedAdjustment *= (1.0f - (behavior.fatigueLevel * speedConfig.fatigueImpact));
    
    // Update current speed
    metrics.currentWPM = speedConfig.baseWPM * speedAdjustment;
}

void HumanSimulator::countClips() {
    File file = SPIFFS.open("/text.txt", "r");
    if (!file) {
        Serial.println("ERROR: Failed to open text.txt");
        return;
    }

    totalClips = 0;
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.indexOf("Clip #") >= 0) {
            totalClips++;
        }
    }
    
    file.close();
    Serial.printf("Found %d total clips\n", totalClips);
}

String HumanSimulator::readClipContent(int clipNumber) {
    File file = SPIFFS.open("/text.txt", "r");
    if (!file) {
        Serial.println("ERROR: Failed to open text.txt");
        return "";
    }

    String clipContent = "";
    bool isReadingClip = false;
    int currentClip = 0;

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();

        if (line.indexOf("Clip #") >= 0) {
            currentClip++;
            if (currentClip == clipNumber) {
                isReadingClip = true;
                continue;
            } else if (isReadingClip) {
                break;
            }
        } else if (isReadingClip && line.length() > 0) {
            clipContent += line + "\n";
        }
    }
    
    file.close();
    
    if (clipContent.length() > 0) {
        Serial.printf("Found clip content (%d characters)\n", clipContent.length());
    } else {
        Serial.println("WARNING: No content found for clip");
    }
    
    return clipContent;
}

void HumanSimulator::logProgress() {
    if (!Constants::Debug::ENABLE_SERIAL_DEBUG) return;

    auto progress = progressTracker->getSnapshot();
    auto behavior = getBehaviorState();
    auto perf = getPerformanceMetrics();

    Serial.println("\n=== Progress Report ===");
    Serial.printf("Time Elapsed: %lu ms\n", progress.elapsedMillis);
    Serial.printf("Progress: %.1f%%\n", progress.percentComplete);
    Serial.printf("Current WPM: %.1f\n", perf.currentWPM);
    Serial.printf("Average WPM: %.1f\n", perf.averageWPM);
    Serial.printf("Error Rate: %.2f%%\n", perf.errorRate * 100);
    
    Serial.println("\n=== Behavioral State ===");
    Serial.printf("Fatigue: %.2f\n", behavior.fatigueLevel);
    Serial.printf("Alertness: %.2f\n", behavior.alertnessLevel);
    Serial.printf("Confidence: %.2f\n", behavior.confidenceLevel);
    
    Serial.println("\n=== Time Compliance ===");
    Serial.printf("Time Utilization: %.1f%%\n", perf.timeUtilization);
    Serial.printf("Speed Compliance: %.1f%%\n", perf.speedCompliance);
}

bool HumanSimulator::validateClipNumber(int clipNumber) {
    if (clipNumber < 1 || clipNumber > totalClips) {
        Serial.printf("ERROR: Invalid clip number %d (total clips: %d)\n", 
                     clipNumber, totalClips);
        return false;
    }
    return true;
}

void HumanSimulator::pause() {
    isPaused = true;
    progressTracker->pause();
    Serial.println("Simulation paused");
}

void HumanSimulator::resume() {
    isPaused = false;
    progressTracker->resume();
    Serial.println("Simulation resumed");
}

bool HumanSimulator::isComplete() const {
    return taskInfo.currentClip >= totalClips;
}

HumanSimulator::TaskInfo HumanSimulator::getTaskInfo() const {
    return taskInfo;
}

HumanSimulator::BehaviorState HumanSimulator::getBehaviorState() const {
    return behavior;
}

HumanSimulator::PerformanceMetrics HumanSimulator::getPerformanceMetrics() const {
    return metrics;
}

void HumanSimulator::updateAlertness() {
    // Decrease alertness with consecutive errors
    if (behavior.consecutiveErrors > 0) {
        behavior.alertnessLevel = max(0.5f, 
            behavior.alertnessLevel - (behavior.consecutiveErrors * 0.05f));
    }
    
    // Increase alertness after successful typing
    if (behavior.consecutiveErrors == 0 && behavior.wordsWithoutBreak > 0) {
        behavior.alertnessLevel = min(1.0f, 
            behavior.alertnessLevel + 0.02f);
    }
}

char HumanSimulator::getRandomTypo(char originalChar) {
    struct KeyMap {
        char key;
        const char* adjacent;
    };
    
    static const KeyMap keyMaps[] = {
        {'a', "qwsz"},
        {'b', "vghn"},
        {'c', "xdfv"},
        // ... add more mappings
    };
    
    char lowerChar = tolower(originalChar);
    for (const auto& map : keyMaps) {
        if (map.key == lowerChar) {
            int len = strlen(map.adjacent);
            return map.adjacent[random(len)];
        }
    }
    
    return 'a' + random(26);  // Fallback to random letter
}

std::vector<Analysis::TimeFrame> HumanSimulator::parseTimeframes(const String& content) {
    std::vector<Analysis::TimeFrame> frames;
    // Implementation here
    return frames;
}
