#ifndef BLINDSSERVO_H
#define BLINDSSERVO_H

#include <Servo.h>
#include <functional>

#define DEBUG_PRINT_CALLBACK_SIGNATURE std::function<void(String debugPrint)> debugPrintCallback
#define STATUS_CHANGED_CALLBACK_SIGNATURE std::function<void(int servoId)> statusChangedCallback
#define POSITION_CHANGED_CALLBACK_SIGNATURE std::function<void(int servoId)> positionChangedCallback


class BlindsServo {
  public:
    enum blindsStatus { OPEN, CLOSED, OPENING, CLOSING };

    BlindsServo(int id, int servoPin, int minPulseValue, int maxPulseValue, int maxDegree, boolean reversed = false, boolean debug = false);
    BlindsServo();
    ~BlindsServo();

    // Main loop
    void loop(); // Main loop

    // Debug
    void setDebug(bool debug);

    // Callbacks
    void setDebugPrintCallback(DEBUG_PRINT_CALLBACK_SIGNATURE);
    void setStatusChangedCallback(STATUS_CHANGED_CALLBACK_SIGNATURE);
    void setPositionChangedCallback(POSITION_CHANGED_CALLBACK_SIGNATURE);

  public:
    // Getters
    boolean isOpening();
    boolean isClosing();
    int currentAngleInPercent();
    int getId();
    int getAngle();
    boolean isClosed();
    boolean isMoving();

    blindsStatus getStatus();

    // Setters
    void setStop(); // Stops the movement
    void setOpen();
    void setClose();
    void goToPosition(int position); // 0-100

  private:
    void init(int id, int servoPin, int minPulseValue, int maxPulseValue, int maxDegree, boolean reversed, boolean debug);
    void attach();
    void detach();
    void goToAngle(int angle); // Goes to specific angle
    int angleToServo(int angle);
    void debugPrint(String text);

  private:
    Servo servo;
    boolean attached;

    int id = 0; // id of the servo

    // Previous status
    blindsStatus previousStatus;

    // Position management
    int target = 0;
    int currentPosition = -1; // -1 = unknown value (initial state)
    int previousPosition = -1;
    
    // Servo configuration
    int servoMinPulse;
    int servoMaxPulse;
    int servoPin;
    int servoMaxDegree;
    boolean isReversed = false;

    // Debugging options
    boolean isDebug = false;

    // Callbacks
    DEBUG_PRINT_CALLBACK_SIGNATURE {nullptr};
    STATUS_CHANGED_CALLBACK_SIGNATURE { nullptr };
    POSITION_CHANGED_CALLBACK_SIGNATURE { nullptr };
};

#endif
