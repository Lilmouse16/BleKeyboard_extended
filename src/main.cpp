#define DEBUG_MODE
#ifdef DEBUG_MODE
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
#endif

#include "hardware.h"
#include "keyboard.h"
#include "human_simulator.h"

Hardware hardware;
Keyboard keyboard;
HumanSimulator simulator(keyboard, hardware);  // Pass both keyboard and hardware references

int currentClip = 1;
bool connectionAnnounced = false;

void setup() {
    Serial.begin(115200);
    Serial.println("\n=== ESP32 Human-like Typer Starting ===");
    
    hardware.init();
    keyboard.init();
    
    if (!SPIFFS.begin(true)) {
        Serial.println("ERROR: SPIFFS Mount Failed");
        return;
    }
    
    simulator.init();
    Serial.println("Ready! Press button to start/pause/resume");
}

void loop() {
    if (keyboard.isConnected()) {
        if (!connectionAnnounced) {
            Serial.println("\n=== Bluetooth Connected ===");
            connectionAnnounced = true;
            hardware.setLedPattern(Hardware::Pattern::ALL_ON);
        }
        
        hardware.handleButton();
        
        // Debug state information
        static unsigned long lastDebugTime = 0;
        if (millis() - lastDebugTime > 1000) {  // Print debug every second
            Serial.printf("States - Paused: %d, SectionComplete: %d, CurrentClip: %d\n", 
                         hardware.isPaused(), hardware.isSectionComplete(), currentClip);
            lastDebugTime = millis();
        }
        
        if (!hardware.isPaused() && !hardware.isSectionComplete()) {
            Serial.println("Starting to process clip...");
            hardware.setLedPattern(Hardware::Pattern::ALTERNATING);
            simulator.processClip(currentClip);
            
            // Only set section complete if we haven't been paused
            if (!hardware.isPaused()) {
                hardware.setSectionComplete(true);
                hardware.playSound(Hardware::SoundType::SECTION_COMPLETE);
                Serial.printf("Completed processing clip %d\n", currentClip);
                currentClip++;
            }
        } else if (hardware.isPaused() && !hardware.isSectionComplete()) {
            hardware.setLedPattern(Hardware::Pattern::RED_ONLY);
        } else if (hardware.isSectionComplete()) {
            hardware.setLedPattern(Hardware::Pattern::SYNC_FLASH);
        }
        
        // Check if all clips are completed
        if (currentClip > simulator.getTotalClips()) {
            Serial.println("\n=== All Clips Completed ===");
            hardware.setLedPattern(Hardware::Pattern::ALL_ON);
            while(1) delay(1000);  // Stop processing
        }

    } else {
        // Not connected to Bluetooth
        connectionAnnounced = false;
        hardware.setSectionComplete(false);
        hardware.setLedPattern(Hardware::Pattern::BLUE_ONLY);
        Serial.println("Waiting for Bluetooth connection...");
        delay(1000);
    }
}