#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define POTENTIOMETER A0

bool redMode = false;
bool blinkMode = false;
bool normalMode = true;
int brightness = 255;

Scheduler runner;

void updateBrightness() {
    brightness = map(analogRead(POTENTIOMETER), 0, 1023, 0, 255);
}

void redModeTask() {
    if (redMode) {
        analogWrite(RED_LED, brightness);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
    }
}

void blinkModeTask() {
    if (blinkMode) {
        analogWrite(RED_LED, brightness);
        analogWrite(YELLOW_LED, brightness);
        analogWrite(GREEN_LED, brightness);
        delay(500);
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        delay(500);
    }
}

void normalModeTask() {
    if (normalMode) {
        analogWrite(RED_LED, brightness);
        delay(2000);
        digitalWrite(RED_LED, LOW);
        analogWrite(YELLOW_LED, brightness);
        delay(500);
        digitalWrite(YELLOW_LED, LOW);
        analogWrite(GREEN_LED, brightness);
        delay(2000);
        for (int i = 0; i < 3; i++) {
            digitalWrite(GREEN_LED, LOW);
            delay(167);
            analogWrite(GREEN_LED, brightness);
            delay(167);
        }
        digitalWrite(GREEN_LED, LOW);
        analogWrite(YELLOW_LED, brightness);
        delay(500);
        digitalWrite(YELLOW_LED, LOW);
    }
}

void button1ISR() {
    redMode = !redMode;
    blinkMode = false;
    normalMode = !redMode;
}

void button2ISR() {
    blinkMode = !blinkMode;
    redMode = false;
    normalMode = !blinkMode;
}

void button3ISR() {
    normalMode = !normalMode;
}

Task tUpdateBrightness(100, TASK_FOREVER, &updateBrightness);
Task tRedMode(0, TASK_FOREVER, &redModeTask);
Task tBlinkMode(0, TASK_FOREVER, &blinkModeTask);
Task tNormalMode(0, TASK_FOREVER, &normalModeTask);

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);

    attachPCINT(digitalPinToPCINT(BUTTON1), button1ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON2), button2ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON3), button3ISR, FALLING);

    runner.init();
    runner.addTask(tUpdateBrightness);
    runner.addTask(tRedMode);
    runner.addTask(tBlinkMode);
    runner.addTask(tNormalMode);

    tUpdateBrightness.enable();
    tRedMode.enable();
    tBlinkMode.enable();
    tNormalMode.enable();
}

void loop() {
    runner.execute();
}
