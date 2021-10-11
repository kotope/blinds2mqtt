#include "BlindsServo.h"

BlindsServo::BlindsServo(int id, int servoPin, int minValue, int maxValue, int maxDegree, boolean reversed, boolean debug) {
  init(id, servoPin, minValue, maxValue, maxDegree, reversed, debug);
}

BlindsServo::BlindsServo() {
      
}

BlindsServo::~BlindsServo() {
  servo.detach();
}

void BlindsServo::init(int id, int servoPin, int minPulseValue, int maxPulseValue, int maxDegree, boolean reversed, boolean debug) {
  this->id = id;
  this->servoPin = servoPin;
  this->servoMinPulse = minPulseValue;
  this->servoMaxPulse = maxPulseValue;
  this->servoMaxDegree = maxDegree;
  this->isDebug = debug;
  this->isReversed = reversed;
  attached = false;
}

void BlindsServo::attach() {
  servo.attach(servoPin, servoMinPulse, servoMaxPulse);
  attached = true;
}

void BlindsServo::detach() {
  servo.detach();
  attached = false;
}

void BlindsServo::goToAngle(int angle) {
  debugPrint("Go to angle: " + String(angle));
  
  // Ensure limits
  if(angle > 270 || angle < 0) {
    return;
  }
  
  if (isMoving()) {
    setStop(); // Stop
  }
  
  target = angle;
}


void BlindsServo::setStop() {
  // Cancel current move
  debugPrint("Stop moving!");
  target = currentPosition;

  statusChangedCallback(id);
}

void BlindsServo::setOpen() {
  goToAngle(servoMaxDegree);  
}

void BlindsServo::setClose() {
  goToAngle(0);
}

void BlindsServo::loop() {
  // Do the main loop
  if (currentPosition != target && attached == false) {
    attach();
  }

  // Before move, we take the prev position
  previousPosition = currentPosition;

  if(currentPosition < target){
    servo.writeMicroseconds(angleToServo(currentPosition++));
    statusChangedCallback(id);
  }
  else if(currentPosition > target){
    servo.writeMicroseconds(angleToServo(currentPosition--));
    statusChangedCallback(id);
  }

  // Notify that we have reached the target
  if (currentPosition == target && previousPosition != currentPosition) {
    detach();
    statusChangedCallback(id);
  }
}

boolean BlindsServo::isOpening() {
  return target > currentPosition;
}

boolean BlindsServo::isClosing() {
  return target < currentPosition;
}

int BlindsServo::currentAngleInPercent() {
  return (int)round(((float) currentPosition / (float) servoMaxDegree) * 100);
}
    
int BlindsServo::getId() {
  return id;
}
int BlindsServo::getAngle() {
  return currentPosition;
}

boolean BlindsServo::isClosed() {
  return currentPosition == 0;
}

boolean BlindsServo::isMoving() {
  return currentPosition != target;
}

BlindsServo::blindsStatus BlindsServo::getStatus() {
  if (currentPosition == 0) {
    return BlindsServo::CLOSED;
  } else if(target < currentPosition) {
    return BlindsServo::CLOSING;
  } else if(target > currentPosition) {
    return BlindsServo::OPENING;
  }

  return BlindsServo::OPEN;
}


void BlindsServo::setDebug(bool debug) {
  isDebug = debug;
}

void BlindsServo::debugPrint(String text) {
  if (isDebug) {
    String value = "(" + String(id) + ") " + text;
    debugPrintCallback(value);
  }
}

void BlindsServo::setDebugPrintCallback(DEBUG_PRINT_CALLBACK_SIGNATURE) {
  if (isDebug) {
    this->debugPrintCallback = debugPrintCallback;
  }
}

void BlindsServo::setStatusChangedCallback(STATUS_CHANGED_CALLBACK_SIGNATURE) {
  this->statusChangedCallback = statusChangedCallback;
}

// Privates
int BlindsServo::angleToServo(int angle){
  // Convert from min - max to 0 - 270

  // formula: (degree * (max - min / servoMax)) + offset 
  float result = (angle * (float)((float)(servoMaxPulse - servoMinPulse) / (float)servoMaxDegree)) + servoMinPulse;

  int res = (int)round(result);

  if(isReversed) { // Servo reversing support
    res = (servoMaxPulse + servoMinPulse) - res;
  }

  // Ensure result is valid
  if (res < servoMinPulse || res > servoMaxPulse) {
    return servoMinPulse;
  }

  return res;
}
