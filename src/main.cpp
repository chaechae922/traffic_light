#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// 핀 정의 (LED, 버튼, 가변저항)
#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define POTENTIOMETER A0

// 모드 및 밝기 변수
bool emergencyMode = false;
bool blinkMode = false;
bool normalMode = true;
bool toggleMode = false;
int brightness = 255;

// 신호등 지속 시간 변수
int redDuration = 2000;
int yellowDuration = 500;
int greenDuration = 2000;

// 버튼 입력 플래그
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

Scheduler runner;

void updateBrightness() {
    brightness = map(analogRead(POTENTIOMETER), 0, 1023, 0, 255);
}

void emergencyModeTaskCallback();
void blinkModeTaskCallback();
void normalModeTaskCallback();

Task tUpdateBrightness(100, TASK_FOREVER, &updateBrightness);
Task tEmergencyMode(0, TASK_FOREVER, &emergencyModeTaskCallback);
Task tBlinkMode(500, TASK_FOREVER, &blinkModeTaskCallback);
Task tNormalMode(0, TASK_FOREVER, &normalModeTaskCallback);

void emergencyModeTaskCallback() {
    if (emergencyMode) {
        analogWrite(RED_LED, brightness);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
    }
}

bool blinkState = false;
void blinkModeTaskCallback() {
    if (blinkMode) {
        blinkState = !blinkState;
        analogWrite(RED_LED, blinkState ? brightness : 0);
        analogWrite(YELLOW_LED, blinkState ? brightness : 0);
        analogWrite(GREEN_LED, blinkState ? brightness : 0);
    }
}

int normalModeStep = 0;
void normalModeTaskCallback() {
    if (!normalMode) return;
    switch (normalModeStep) {
        case 0:
            analogWrite(RED_LED, brightness);
            tNormalMode.setInterval(redDuration);
            break;
        case 1:
            analogWrite(RED_LED, 0);
            analogWrite(YELLOW_LED, brightness);
            tNormalMode.setInterval(yellowDuration);
            break;
        case 2:
            analogWrite(YELLOW_LED, 0);
            analogWrite(GREEN_LED, brightness);
            tNormalMode.setInterval(greenDuration);
            break;
        case 3:
            analogWrite(GREEN_LED, 0);
            tNormalMode.setInterval(167);
            break;
        case 4:
            analogWrite(GREEN_LED, brightness);
            tNormalMode.setInterval(167);
            break;
        case 5:
            analogWrite(GREEN_LED, 0);
            tNormalMode.setInterval(167);
            break;
        case 6:
            analogWrite(GREEN_LED, brightness);
            tNormalMode.setInterval(167);
            break;
        case 7:
            analogWrite(GREEN_LED, 0);
            tNormalMode.setInterval(167);
            break;
        case 8:
            analogWrite(GREEN_LED, brightness);
            tNormalMode.setInterval(167);
            break;
        case 9:
            analogWrite(GREEN_LED, 0);
            analogWrite(YELLOW_LED, brightness);
            tNormalMode.setInterval(yellowDuration);
            break;
        case 10:
            analogWrite(YELLOW_LED, 0);
            normalModeStep = 0;
            break;
    }
}

void button1ISR() { button1Pressed = true; }
void button2ISR() { button2Pressed = true; }
void button3ISR() { button3Pressed = true; }

String serialInput = "";

void parseSerialCommand(String input);

void handleSerialInput() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            serialInput.trim();
            if (serialInput.startsWith("cmd:")) {
                parseSerialCommand(serialInput);
            } else {
                int spaceIdx = serialInput.indexOf(' ');
                if (spaceIdx != -1) {
                    String key = serialInput.substring(0, spaceIdx);
                    String val = serialInput.substring(spaceIdx + 1);
                    int timeVal = val.toInt();

                    if (key == "RED") {
                        redDuration = timeVal;
                        if (normalMode && digitalRead(RED_LED)) {
                            tNormalMode.setInterval(redDuration);
                        }
                    } else if (key == "YELLOW") {
                        yellowDuration = timeVal;
                        if (normalMode && digitalRead(YELLOW_LED)) {
                            tNormalMode.setInterval(yellowDuration);
                        }
                    } else if (key == "GREEN") {
                        greenDuration = timeVal;
                        if (normalMode && digitalRead(GREEN_LED)) {
                            tNormalMode.setInterval(greenDuration);
                        }
                    }
                }
            }
            serialInput = "";
        } else {
            serialInput += c;
        }
    }
}

void parseSerialCommand(String input) {
    input = input.substring(4);
    int idx;
    while ((idx = input.indexOf(";")) != -1) {
        String token = input.substring(0, idx);
        input = input.substring(idx + 1);
        int eq = token.indexOf("=");
        if (eq == -1) continue;
        String key = token.substring(0, eq);
        String val = token.substring(eq + 1);

        if (key == "brightness") {
            brightness = val.toInt();
        } else if (key == "redTime") {
            redDuration = val.toInt();
            if (normalMode && digitalRead(RED_LED)) tNormalMode.setInterval(redDuration);
        } else if (key == "yellowTime") {
            yellowDuration = val.toInt();
            if (normalMode && digitalRead(YELLOW_LED)) tNormalMode.setInterval(yellowDuration);
        } else if (key == "greenTime") {
            greenDuration = val.toInt();
            if (normalMode && digitalRead(GREEN_LED)) tNormalMode.setInterval(greenDuration);
        } else if (key == "button") {
            int btn = val.toInt();
            if (btn == 1) button1Pressed = true;
            else if (btn == 2) button2Pressed = true;
            else if (btn == 3) button3Pressed = true;
        }
    }
}

void sendStateToSerial() {
    Serial.print("mode:");
    if (emergencyMode) Serial.print("emergency");
    else if (blinkMode) Serial.print("blink");
    else if (normalMode) Serial.print("normal");
    else Serial.print("On/Off");

    Serial.print(",brightness:");
    Serial.print(brightness);
    Serial.print(",red:");
    Serial.print(digitalRead(RED_LED));
    Serial.print(",yellow:");
    Serial.print(digitalRead(YELLOW_LED));
    Serial.print(",green:");
    Serial.println(digitalRead(GREEN_LED));
}

void setup() {
    Serial.begin(9600);
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
    runner.addTask(tEmergencyMode);
    runner.addTask(tBlinkMode);
    runner.addTask(tNormalMode);

    tUpdateBrightness.enable();
    tEmergencyMode.enable();
    tBlinkMode.disable();
    tNormalMode.enable();
}

void loop() {
    handleSerialInput();
    runner.execute();
    sendStateToSerial();

    if (button1Pressed) {
        button1Pressed = false;
        toggleMode = !toggleMode;
        if (toggleMode) {
            tEmergencyMode.disable();
            tBlinkMode.disable();
            tNormalMode.disable();
            emergencyMode = true;
            analogWrite(RED_LED, brightness);
            analogWrite(YELLOW_LED, 0);
            analogWrite(GREEN_LED, 0);
        } else {
            emergencyMode = false;
            analogWrite(RED_LED, 0);
            tNormalMode.enable();
        }
    }

    if (button2Pressed) {
        button2Pressed = false;
        blinkMode = !blinkMode;
        emergencyMode = false;
        normalMode = !blinkMode;
        if (blinkMode) {
            tBlinkMode.enable();
            tEmergencyMode.disable();
            tNormalMode.disable();
        } else {
            digitalWrite(RED_LED, LOW);
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(GREEN_LED, LOW);
            tNormalMode.enable();
        }
    }

    if (button3Pressed) {
        button3Pressed = false;
        normalMode = !normalMode;
        if (normalMode) {
            tNormalMode.enable();
        } else {
            tNormalMode.disable();
            digitalWrite(RED_LED, LOW);
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(GREEN_LED, LOW);
        }
    }
}