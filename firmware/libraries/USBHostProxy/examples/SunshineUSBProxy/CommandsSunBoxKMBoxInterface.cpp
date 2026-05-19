#include "CommandsSunBoxKMBoxInterface.h"
#include "SunBoxStartup.h"
#include <stdio.h>

CommandsSunBoxKMBoxInterface::CommandsSunBoxKMBoxInterface()
    : commandBuffer(""), lastCharTime(0), autoReleaseButtons(0), dataAvailable(false) {
    currentState.clear();
}

void CommandsSunBoxKMBoxInterface::begin() {
    // Initialize if needed
}

void CommandsSunBoxKMBoxInterface::processSerial(Stream& serial) {
    while (serial.available()) {
        char c = serial.read();
        lastCharTime = millis();
        
        // Handle line endings
        if (c == '\n' || c == '\r' || c == ';' || c == '!') {
            if (commandBuffer.length() > 0) {
                processCommand(commandBuffer, &serial);
                commandBuffer = "";
            }
        } else if (c >= 32 && c <= 126) {  // Printable ASCII
            commandBuffer += c;
            
            // Prevent buffer overflow
            if (commandBuffer.length() > 100) {
                commandBuffer = "";
            }
        }
    }
    
    // Process command after timeout (100ms)
    if (commandBuffer.length() > 0 && (millis() - lastCharTime > 100)) {
        processCommand(commandBuffer, &serial);
        commandBuffer = "";
    }
}

void CommandsSunBoxKMBoxInterface::processCommand(const String& command, Stream* responseSerial) {
    bool debug_enabled = SunBoxStartup::isDebugEnabled();
    bool shouldSendOk = false;
    
    // Parse KMBox B+ commands
    // Format: km.command(args)
    
    if (command.startsWith("km.move(")) {
        int x = 0, y = 0, steps = 0;
        // sscanf might be picky with spaces, but usually KMBox commands are compact
        if (sscanf(command.c_str(), "km.move(%d,%d,%d)", &x, &y, &steps) >= 2 ||
            sscanf(command.c_str(), "km.move(%d, %d, %d)", &x, &y, &steps) >= 2) {
            currentState.x += (int16_t)x;
            currentState.y += (int16_t)y;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.click(")) {
        int button = -1;
        if (sscanf(command.c_str(), "km.click(%d)", &button) == 1) {
            uint8_t mask = 0;
            if (button == 0) mask = 0x01;      // Left
            else if (button == 1) mask = 0x04; // Middle
            else if (button == 2) mask = 0x02; // Right
            else if (button == 3) mask = 0x08; // Side 1 (Back)
            else if (button == 4) mask = 0x10; // Side 2 (Forward)
            
            if (mask) {
                currentState.buttons |= mask;
                autoReleaseButtons |= mask;
                dataAvailable = true;
                shouldSendOk = true;
            }
        }
    } else if (command.startsWith("km.left(")) {
        int state = 0;
        if (sscanf(command.c_str(), "km.left(%d)", &state) == 1) {
            if (state) currentState.buttons |= 0x01;
            else currentState.buttons &= ~0x01;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.right(")) {
        int state = 0;
        if (sscanf(command.c_str(), "km.right(%d)", &state) == 1) {
            if (state) currentState.buttons |= 0x02;
            else currentState.buttons &= ~0x02;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.middle(")) {
        int state = 0;
        if (sscanf(command.c_str(), "km.middle(%d)", &state) == 1) {
            if (state) currentState.buttons |= 0x04;
            else currentState.buttons &= ~0x04;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.side1(")) {
        int state = 0;
        if (sscanf(command.c_str(), "km.side1(%d)", &state) == 1) {
            if (state) currentState.buttons |= 0x08;
            else currentState.buttons &= ~0x08;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.side2(")) {
        int state = 0;
        if (sscanf(command.c_str(), "km.side2(%d)", &state) == 1) {
            if (state) currentState.buttons |= 0x10;
            else currentState.buttons &= ~0x10;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command.startsWith("km.wheel(")) {
        int v = 0;
        if (sscanf(command.c_str(), "km.wheel(%d)", &v) == 1) {
            currentState.wheel += (int8_t)v;
            dataAvailable = true;
            shouldSendOk = true;
        }
    } else if (command == "km.ver" || command == "km.ver()" || 
               command == "km.id" || command == "km.id()" ||
               command == "km.info" || command == "km.info()" ||
               command.startsWith("km.ver ") || command.startsWith("km.id ")) {
        // Handshake/Identification response
        if (responseSerial) {
            // Using kmbox_b+ (streamcheats) to satisfy both standard loaders and custom identification
            responseSerial->println("kmbox_b+ (streamcheats)");
        }
    } else if (command.startsWith("km.mask(")) {
        shouldSendOk = true;
    } else if (command.startsWith("km.beep(")) {
        shouldSendOk = true;
    } else if (command.startsWith("km.reset()")) {
        currentState.clear();
        autoReleaseButtons = 0;
        dataAvailable = true;
        shouldSendOk = true;
    } else if (debug_enabled) {
        Serial4.print("I: Unknown KMBox command: ");
        Serial4.println(command);
    }
    
    // Acknowledge commands if needed
    if (shouldSendOk && responseSerial) {
        responseSerial->println("ok");
    }
}

void CommandsSunBoxKMBoxInterface::reset() {
    // Release any "one-shot" buttons from km.click()
    if (autoReleaseButtons) {
        currentState.buttons &= ~autoReleaseButtons;
        autoReleaseButtons = 0;
    }
    
    // Clear relative movement
    currentState.x = 0;
    currentState.y = 0;
    currentState.wheel = 0;
    
    dataAvailable = false;
}

MouseState CommandsSunBoxKMBoxInterface::getMouseState() const {
    return currentState;
}