#include <Arduino.h>
#include <TaskScheduler.h>
#include <PinChangeInterrupt.h>

// 핀 정의 (LED, 버튼, 가변저항항)
#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define POTENTIOMETER A0

// 모드 및 밝기 변수
bool redMode = false;     // 빨간불 모드
bool blinkMode = false;   // 모든 LED 깜빡임 모드
bool normalMode = true;   // 정상 신호등 모드
bool toggleMode = false;  // 수동 모드 여부
int brightness = 255;     // LED 밝기 값 (0 ~ 255)

// 버튼 입력 플래그 (인터럽트용)
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

// TaskScheduler 초기화
Scheduler runner;

// 가변저항을 읽어 LED 밝기 조절
void updateBrightness() {
    brightness = map(analogRead(POTENTIOMETER), 0, 1023, 0, 255);
}

// Task 콜백 함수 선언
void redModeTaskCallback();
void blinkModeTaskCallback();
void normalModeTaskCallback();

// Task 정의 (각 모드별 실행할 함수)
Task tUpdateBrightness(100, TASK_FOREVER, &updateBrightness);
Task tRedMode(0, TASK_FOREVER, &redModeTaskCallback);
Task tBlinkMode(500, TASK_FOREVER, &blinkModeTaskCallback);
Task tNormalMode(0, TASK_FOREVER, &normalModeTaskCallback);

// 빨간불 모드 실행 함수
void redModeTaskCallback() {
    if (redMode) {
        analogWrite(RED_LED, brightness);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
    }
}

// 모든 LED 깜빡임 모드 실행 함수
bool blinkState = false;
void blinkModeTaskCallback() {
    if (blinkMode) {
        blinkState = !blinkState;
        analogWrite(RED_LED, blinkState ? brightness : 0);
        analogWrite(YELLOW_LED, blinkState ? brightness : 0);
        analogWrite(GREEN_LED, blinkState ? brightness : 0);
    }
}

// 정상 신호등 모드 실행 함수
int normalModeStep = 0;
void normalModeTaskCallback() {
    if (normalMode) {
        switch (normalModeStep) {
            case 0:
                analogWrite(RED_LED, brightness); // 빨간불 2초
                tNormalMode.setInterval(2000);
                break;
            case 1:
                analogWrite(RED_LED, 0);
                analogWrite(YELLOW_LED, brightness); // 노란불 0.5초
                tNormalMode.setInterval(500);
                break;
            case 2:
                analogWrite(YELLOW_LED, 0);
                analogWrite(GREEN_LED, brightness); // 초록불 2초
                tNormalMode.setInterval(2000);
                break;
            case 3:
                analogWrite(GREEN_LED, 0); // 초록불 깜빡이기 (167ms 간격 -  1초에 3번)
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
                analogWrite(YELLOW_LED, brightness); // 노란불 0.5초
                tNormalMode.setInterval(500);
                break;
            case 10:
                analogWrite(YELLOW_LED, 0);
                normalModeStep = -1; // 다시 빨간불 단계로 돌아감 (초기화)
                break;
        }
        normalModeStep++;
    }
}

// 버튼 인터럽트 핸들러 (각 버튼이 눌리면 실행)
void button1ISR() { button1Pressed = true; }
void button2ISR() { button2Pressed = true; }
void button3ISR() { button3Pressed = true; }

void setup() {
    // 시리얼 통신 시작
    Serial.begin(9600);

    // 핀 모드 설정
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);

    // 버튼 인터럽트 설정 (FALLING 시 버튼 입력 감지)
    attachPCINT(digitalPinToPCINT(BUTTON1), button1ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON2), button2ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON3), button3ISR, FALLING);

    // TaskScheduler에 Task 추가
    runner.init();
    runner.addTask(tUpdateBrightness);
    runner.addTask(tRedMode);
    runner.addTask(tBlinkMode);
    runner.addTask(tNormalMode);

    // 기본 동작 Task 실행
    tUpdateBrightness.enable();
    tRedMode.enable();
    tBlinkMode.disable();
    tNormalMode.enable();
}

void loop() {
    runner.execute();

    // 신호등 상태 출력
    Serial.print("Mode: ");
    if (redMode) Serial.print("Red");
    else if (blinkMode) Serial.print("Blink");
    else if (normalMode) Serial.print("Normal");
    
    Serial.print(", Brightness: ");
    Serial.print(brightness);
    
    Serial.print(", Red: ");
    Serial.print(digitalRead(RED_LED));
    
    Serial.print(", Yellow: ");
    Serial.print(digitalRead(YELLOW_LED));
    
    Serial.print(", Green: ");
    Serial.println(digitalRead(GREEN_LED));

    delay(500); // 500ms마다 송신

    // 1번 버튼: 빨간불 모드
    if (button1Pressed) {
        button1Pressed = false;
        toggleMode = !toggleMode;

        if (toggleMode) {
            tRedMode.disable();
            tBlinkMode.disable();
            tNormalMode.disable();
            analogWrite(RED_LED, brightness);
            analogWrite(YELLOW_LED, 0);
            analogWrite(GREEN_LED, 0);
            Serial.println("Manual Mode: RED_ON, YELLOW_OFF, GREEN_OFF");
        } else {
            analogWrite(RED_LED, 0);
            Serial.println("Manual Mode Off: RED_OFF");
            tNormalMode.enable();
        }
    }

    // 2번 버튼: 모든 LED 깜빡임 모드
    if (button2Pressed) {
        button2Pressed = false;
        blinkMode = !blinkMode;
        redMode = false;
        normalMode = !blinkMode;

        if (blinkMode) {
            tBlinkMode.enable();
            tRedMode.disable();
            tNormalMode.disable();
        } else {
            digitalWrite(RED_LED, LOW);
            digitalWrite(YELLOW_LED, LOW);
            digitalWrite(GREEN_LED, LOW);
            tNormalMode.enable();
        }
    }

    // 3번 버튼: 신호등 시스템 전체 ON/OFF
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