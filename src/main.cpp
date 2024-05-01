#pragma region VEXcode Generated Robot Configuration
// Make sure all required headers are included.
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <iostream>


#include "vex.h"

using namespace vex;

// Brain should be defined by default
brain Brain;


// START V5 MACROS
#define waitUntil(condition)                                                   \
  do {                                                                         \
    wait(5, msec);                                                             \
  } while (!(condition))

#define repeat(iterations)                                                     \
  for (int iterator = 0; iterator < iterations; iterator++)
// END V5 MACROS


// Robot configuration code.
motor SpinMotor = motor(PORT3, ratio18_1, false);

distance DistanceSensor = distance(PORT4);
motor ClawMotor = motor(PORT1, ratio18_1, true);

motor ArmMotor = motor(PORT2, ratio18_1, true);

optical OpticalSensor = optical(PORT5);
limit LimitArm = limit(Brain.ThreeWirePort.A);
limit LimitSpin = limit(Brain.ThreeWirePort.B);
bumper BumperUser = bumper(Brain.ThreeWirePort.C);
led LEDGREEN = led(Brain.ThreeWirePort.F);
led LEDYELLOW = led(Brain.ThreeWirePort.G);
led LEDRED = led(Brain.ThreeWirePort.H);



// Helper to make playing sounds from the V5 in VEXcode easier and
// keeps the code cleaner by making it clear what is happening.
void playVexcodeSound(const char *soundName) {
  printf("VEXPlaySound:%s\n", soundName);
  wait(5, msec);
}

#pragma endregion VEXcode Generated Robot Configuration

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       {author}                                                  */
/*    Created:      {date}                                                    */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// Include the V5 Library
#include "vex.h"
  
// Allows for easier use of the VEX Library
using namespace vex;

#define spinGearRatio  (84.0 / 12.0)
#define armGearRatio   (84.0 / 12.0)
#define clawGearRatio  (36.0 / 12.0)

#define ARMMAXANGLE   (60.0 * armGearRatio)
#define CLAWMAXANGLE  (86.0 * clawGearRatio)

int greenState = 0;
int yellowState = 0;
int redState = 0;

#define SPINCW directionType::fwd
#define SPINCCW directionType::rev
#define ARMUP directionType::fwd
#define ARMDOWN directionType::rev
#define CLAWOPEN directionType::fwd
#define CLAWCLOSE directionType::rev

void LEDControlThread() {
  LEDRED.off();
  LEDGREEN.off();
  LEDYELLOW.off();
  int phase = 0;

  while (true) {
    if (greenState == 0 || (greenState == 2 && phase == 0)) LEDGREEN.off();
    else if (greenState == 1 || (greenState == 2 && phase == 1)) LEDGREEN.on();

    if (yellowState == 0 || (yellowState == 2 && phase == 0)) LEDYELLOW.off();
    else if (yellowState == 1 || (yellowState == 2 && phase == 1)) LEDYELLOW.on();

    if (redState == 0 || (redState == 2 && phase == 0)) LEDRED.off();
    else if (redState == 1 || (redState == 2 && phase == 1)) LEDRED.on();

    phase = (phase == 0) ? 1 : 0;
    wait(1, seconds);
  }
}

void BrainPrintInit()
{
  Brain.Screen.setFillColor(orange);
  Brain.Screen.clearScreen(orange);
  Brain.Screen.setFont(prop40);
  Brain.Screen.setCursor(3, 7);
  Brain.Screen.print("Initializing");
}

void BrainPrintReady()
{
  Brain.Screen.setFillColor(green);
  Brain.Screen.clearScreen(green);
  Brain.Screen.setFont(prop40);
  Brain.Screen.setCursor(3, 7);
  Brain.Screen.print("Ready");
}

void BrainPrintError()
{
  Brain.Screen.setFillColor(red);
  Brain.Screen.clearScreen(red);
  Brain.Screen.setFont(prop40);
  Brain.Screen.setCursor(3, 7);
  Brain.Screen.print("Error");
}

void OnInit()
{
  BrainPrintInit();
  yellowState = 2;
  redState = 0;
  greenState = 0;
}

void OnError()
{
  yellowState = 0;
  greenState = 0;
  redState = 2;
  BrainPrintError();
  while (true) {wait(1, seconds);}
}

void OnReady()
{
  yellowState = 0;
  greenState = 2;
  redState = 0;
  BrainPrintReady();
}

bool bLimitSpinPressed = false;

void OnLimitSpinPressed()
{
  bLimitSpinPressed = true;
}

void OnLimitSpinReleased()
{
  bLimitSpinPressed = false;
}

bool bSpinInitialized = false;

// return true if error
bool SpinInit(bool full=false)
{
  SpinMotor.setStopping(hold);
  SpinMotor.setVelocity(30, percent);
  LimitSpin.pressed(OnLimitSpinPressed);
  LimitSpin.released(OnLimitSpinReleased);

  if ((full && LimitSpin.pressing()) || !LimitSpin.pressing()) {
    bool bReleaseFirst = false;
    if (LimitSpin.pressing()) {
      bReleaseFirst = true;
      bLimitSpinPressed = true;
    }
    SpinMotor.spinFor(SPINCW, (360.0 + 90.0) * spinGearRatio, degrees, false);
    if (bReleaseFirst) {
      while (bLimitSpinPressed) {
        if (SpinMotor.isDone()) return true;
        wait(10, msec);
      }
    }
    while (!bLimitSpinPressed) {
      if (SpinMotor.isDone()) return true;
      wait(5, msec);
    }
    SpinMotor.stop();
  }

  SpinMotor.setPosition(0.0, degrees);
  SpinMotor.stop(coast);
  bSpinInitialized = true;
  return false;
}

void OnArmLimitPressed()
{
  ArmMotor.stop();
}

bool bArmInitialized = false;
bool bArmDown = false;

// returns true on error
bool ArmInit()
{
    ArmMotor.setVelocity(25, percent);
    ArmMotor.setTimeout(4, seconds);
    ArmMotor.setStopping(coast);
    if (!LimitArm.pressing()) {
      printf("Moving ARM\n");

      ArmMotor.setMaxTorque(25, percent);
      ArmMotor.spinFor(ARMDOWN, ARMMAXANGLE, degrees, false);
      uint32_t startTime = timer::system();
      while(!LimitArm.pressing() && (timer::system() - startTime < 4000)) {
        wait(10, msec);
      }
      ArmMotor.stop();
      printf("ARM Stopped\n");
    }
    wait(500, msec);
    if (!LimitArm.pressing()) 
    {
      printf("ARM Error\n");
      return true;
    }
    ArmMotor.setPosition(0, degrees);
    ArmMotor.setMaxTorque(50, percent);
    LimitArm.pressed(OnArmLimitPressed);
    bArmInitialized = true;
    bArmDown = true;
    return false;
}

bool ArmTo(float angle) {
  if (!bArmInitialized) return true;
  if (angle > ARMMAXANGLE) angle = ARMMAXANGLE;
  if (angle < 10.0 * armGearRatio) angle = 10.0 * armGearRatio;
  ArmMotor.setStopping(hold);
  ArmMotor.spinToPosition(angle, degrees, true);
  ArmMotor.stop();
  bArmDown = false;
  return false;
}

bool ArmUp(float angle=ARMMAXANGLE, bool wait=true)
{
  if (!bArmInitialized) return true;
  // if (!bArmDown) return false;
  if (angle > ARMMAXANGLE) angle = ARMMAXANGLE;
  ArmMotor.setStopping(hold);
  ArmMotor.spinToPosition(angle, degrees, wait);
  ArmMotor.stop();
  bArmDown = false;
  return false;
}

bool ArmDown(bool wait=true)
{
  if (!bArmInitialized) return true;
  if (bArmDown) return false;
  ArmMotor.setStopping(coast);
  ArmMotor.spinToPosition(0.0, degrees, wait);
  ArmMotor.stop();
  float temp = ArmMotor.temperature(percent);
  float pos = ArmMotor.position(degrees);
  printf("arm: %f, pos %f\n", temp, pos);
  bArmDown = true;
  return false;
}


bool bClawInitialized = false;
bool bClawOpen = false;

bool ClawOpen(bool wait=true)
{
    if (!bClawInitialized) return true;
    if (bClawOpen) return false;
    ClawMotor.setStopping(coast);
    ClawMotor.spinToPosition(CLAWMAXANGLE, degrees, wait);
    ClawMotor.stop();
    float temp = ClawMotor.temperature(percent);
    float pos = ClawMotor.position(degrees);
    printf("claw: %f, pos %f\n", temp, pos);
    bClawOpen = true;
    return false;
}

bool ClawClose(bool wait=true)
{
  if (!bClawInitialized) return true;
  if (!bClawOpen) return false;
  ClawMotor.setStopping(hold);
  ClawMotor.spinToPosition(0, degrees, wait);
  ClawMotor.stop();
  bClawOpen = false;
  return false;
}

bool ClawInit()
{
    ClawMotor.setStopping(hold);
    ClawMotor.setVelocity(50, percent);
    ClawMotor.setTimeout(1, seconds);
    ClawMotor.spinFor(CLAWCLOSE, CLAWMAXANGLE, degrees, true);
    ClawMotor.stop();
    ClawMotor.setTimeout(1, seconds);
    ClawMotor.setPosition(0, degrees);
    bClawInitialized = true;
    ClawOpen();    
    return false;
}

bool bBumperPressed = false;

void ResetBumper()
{
  bBumperPressed = false;
}

void OnBumperPressed()
{
  bBumperPressed = true;
}

struct tSensorLog {
    int32_t time;
    float pos;
    float distance;
    int size;
    int coldist;
    int colhue;
    int color;
};

#define MAXLOG 20000
tSensorLog LOGARRAY[MAXLOG];
int32_t logId = 0;
bool bSensorSample = false;

void sensorChanged(float pos, float distance, int size, int coldist, int colhue, int color)
{
    if (logId < MAXLOG) {
        LOGARRAY[logId].time = vex::timer::system();
        LOGARRAY[logId].pos = pos;
        LOGARRAY[logId].distance = distance;
        logId++;
    } else if (logId == MAXLOG) {
        printf("log buffer overflow\n");
        logId++;
    }
}

int SensorSampler()
{
  while (true) {
    float currentDistance = DistanceSensor.objectDistance(inches);
    float currentPosition = SpinMotor.position(degrees);
    if (bSensorSample) sensorChanged(currentPosition, currentDistance, 0, 0, 0, 0);
 
    this_thread::sleep_for(5);
  }
  return 0;
}

void DumpLog()
{
    if (!Brain.SDcard.isInserted()) {
        printf("No SDCARD\n");
    }
    if (logId >= MAXLOG) logId = MAXLOG;
    std::string str = "";
    char tempStr[100];
    str += "time, deg, torque\n";
    for (int i = 0; i < logId; i++) {
        sprintf(&(tempStr[0]), "%ld, %f, %f\n", LOGARRAY[i].time, LOGARRAY[i].pos, LOGARRAY[i].distance);
        str += tempStr;
        // str += LOGARRAY[i].time + LOGARRAY[i].val + "\n";
    }

    const char *cStr = str.c_str();
    int len = strlen(cStr);

    if (Brain.SDcard.isInserted()) {
        int saved = Brain.SDcard.savefile("data.csv", (uint8_t *) cStr, len);

        printf("%d of %d bytes written to file\n", saved, len);
        if (Brain.SDcard.exists("data.csv")) {
            printf("File size: %ld\n", Brain.SDcard.size("data.csv"));        }
    } else {
        printf("%s", cStr);
    }
    // printf("%s", str.c_str());
    // Brain.SDcard.savefile
}

int main() {
  OnInit();

  // Let all sensors and devices initialize
  wait(1, seconds);

  thread ledThread = thread(LEDControlThread);
  thread sensorThread = thread(SensorSampler);

  bool bError = false;
  if (!SpinMotor.installed()) bError = true;
  if (!ClawMotor.installed()) bError = true;
  if (!ArmMotor.installed()) bError = true;
  if (!DistanceSensor.installed()) bError = true;
  if (!OpticalSensor.installed()) bError = true;
  if (bError) OnError();

  if (SpinInit(false)) OnError();
  if (ArmInit()) OnError();
  if (ClawInit()) OnError();
  ArmUp();
  ClawOpen();
  OnReady();
  BumperUser.pressed(OnBumperPressed);

  while(true) {

    ResetBumper();
    while(!bBumperPressed) wait(10, msec);

    SpinMotor.setVelocity(20.0, percent);
    SpinMotor.spinToPosition(0, degrees);

    OpticalSensor.setLightPower(100.0, percent);
    OpticalSensor.setLight(ledState::on);

    SpinMotor.setVelocity(25.0, percent);
    SpinMotor.spinFor(reverse, 90.0 * spinGearRatio, degrees);
    SpinMotor.setVelocity(15.0, percent);  
    SpinMotor.spinFor(forward, 180.0 * spinGearRatio, degrees, false);

    bSensorSample = true;

    bool done = false;
    float minDistance = 15.0;
    float minPosition = 0.0;
    while (!done) {
      if (DistanceSensor.isObjectDetected() /* && DistanceSensor.objectSize() == sizeType::large */ ) {
        float currentDistance = DistanceSensor.objectDistance(inches);
        if (currentDistance < minDistance) {
          minDistance = currentDistance;
          minPosition = SpinMotor.position(degrees);
        }
      }
      wait(0.005,seconds);
      if (SpinMotor.isDone()) done = true;
    }

    bSensorSample = false;
    DumpLog();

    printf("Closest object: %fin, %fdeg\n", minDistance, minPosition);
    bool objectFound = false;
    bool bFoundRed = false;
    bool bFoundBlue = false;
    if (minDistance < 9.0 && minDistance > 6.0) {
      SpinMotor.setVelocity(25.0, percent);
      SpinMotor.spinToPosition(minPosition - (4.0 * spinGearRatio), degrees);
      SpinMotor.stop();
      ArmTo(45.0 * armGearRatio);
      // OpticalSensor.setLightPower(100.0, percent);
      // OpticalSensor.setLight(ledState::on);
      wait(1.0, seconds);
      float currentHue = OpticalSensor.hue();
      color currentColor = OpticalSensor.color();
      printf("%f\n", currentHue);
      if (currentColor == black) printf("black\n");
      else if (currentColor == white) printf("white\n");
      else if (currentColor == red) printf("red\n");
      else if (currentColor == green) printf("green\n");
      else if (currentColor == blue) printf("blue\n");
      else if (currentColor == yellow) printf("yellow\n");
      else if (currentColor == orange) printf("orange\n");
      else if (currentColor == purple) printf("purple\n");
      else if (currentColor == cyan) printf("cyan\n");
      bFoundRed = (currentColor == red) ? true : false;
      bFoundBlue = (currentColor == blue || (currentHue > 247.0 - 20.0 && currentHue < 247.0 + 20.0)) ? true : false;
      if (bFoundRed || bFoundBlue) {
        objectFound = true;
      }
      // OpticalSensor.setLight(ledState::off);
    }
    if (bFoundRed || bFoundBlue) {
      ArmDown();
      ClawClose();
      ArmUp();
      if (bFoundRed) SpinMotor.spinToPosition(-120.0 * spinGearRatio, degrees);
      else if (bFoundBlue) SpinMotor.spinToPosition(120.0 * spinGearRatio, degrees);
      ClawOpen();
      SpinMotor.spinToPosition(0.0 * spinGearRatio, degrees);      
    }
    if (!objectFound) {
      SpinMotor.spinToPosition(0.0, degrees);
      ArmUp();
    }
    SpinMotor.stop(coast);
  }

/*
  SpinMotor.spin(forward, 25.0, percent);

  while(true) {
    if (DistanceSensor.isObjectDetected()) {
      if (DistanceSensor.objectDistance(inches) < 7.0) {
        SpinMotor.stop();
        wait(2, seconds);
        SpinMotor.spin(forward, 25.0, percent);
      }
    }
    wait(0.01,seconds);
  }*/
}
