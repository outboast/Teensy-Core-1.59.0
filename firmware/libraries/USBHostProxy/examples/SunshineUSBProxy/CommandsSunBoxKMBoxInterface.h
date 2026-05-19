#ifndef _COMMANDS_SUNBOX_KMBOX_INTERFACE_H_
#define _COMMANDS_SUNBOX_KMBOX_INTERFACE_H_

#include <Arduino.h>
#include "HIDMouseDescriptorHandler.h"

class CommandsSunBoxKMBoxInterface {
public:
    CommandsSunBoxKMBoxInterface();
    
    void begin();
    
    // Process serial data for KMBox protocol
    void processSerial(Stream& serial);
    
    // Process complete command (for routing)
    void processCommand(const String& command, Stream* responseSerial = nullptr);
    
    // Check if data is available
    bool hasData() const { return dataAvailable; }
    
    // Reset data flag
    void reset();
    
    // Get mouse state
    MouseState getMouseState() const;
    
private:
    // Command buffer
    String commandBuffer;
    unsigned long lastCharTime;
    
    // Mouse state
    MouseState currentState;
    uint8_t autoReleaseButtons;
    bool dataAvailable;
};

#endif // _COMMANDS_SUNBOX_KMBOX_INTERFACE_H_