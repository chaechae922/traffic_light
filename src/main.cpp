#include <Arduino.h>
#include <TaskScheduler.h>       // 여러 작업(task) 스케줄링 라이브러리
#include <PinChangeInterrupt.h>  // 모든 핀에서 인터럽트 사용 가능하게 하는 라이브러리

// 핀 설정 (LED 3개, 버튼 3개, 가변저항 1개)
#define RED_LED 5
#define YELLOW_LED 6
#define GREEN_LED 7
#define BUTTON1 2
#define BUTTON2 3
#define BUTTON3 4
#define POTENTIOMETER A0

// 모드 플래그 변수
bool emergencyMode = false;  // 비상 모드 (빨간불만 점등)
bool blinkMode = false;      // 깜빡이 모드 (모든 불이 깜빡임)
bool normalMode = true;      // 일반 모드 (신호등 순서대로)
bool toggleMode = false;     // 비상모드 전환용 토글
int brightness = 255;        // LED 밝기 (0~255)

// 신호등 각 색상 지속 시간 (기본값)
int redDuration = 2000;
int yellowDuration = 500;
int greenDuration = 2000;

// 버튼 인터럽트 플래그
volatile bool button1Pressed = false;
volatile bool button2Pressed = false;
volatile bool button3Pressed = false;

// TaskScheduler 인스턴스 생성
Scheduler runner;

// 가변저항으로 밝기 업데이트
void updateBrightness() {
    brightness = map(analogRead(POTENTIOMETER), 0, 1023, 0, 255);
}

// 모드별 작업 콜백 함수 선언
void emergencyModeTaskCallback();
void blinkModeTaskCallback();
void normalModeTaskCallback();

// 작업(Task) 생성
Task tUpdateBrightness(100, TASK_FOREVER, &updateBrightness);      // 밝기 갱신
Task tEmergencyMode(0, TASK_FOREVER, &emergencyModeTaskCallback);  // 비상 모드
Task tBlinkMode(500, TASK_FOREVER, &blinkModeTaskCallback);        // 깜빡이 모드
Task tNormalMode(0, TASK_FOREVER, &normalModeTaskCallback);        // 일반 신호등 모드

// 비상모드: 빨간불만 켜짐
void emergencyModeTaskCallback() {
    if (emergencyMode) {
        analogWrite(RED_LED, brightness);
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
    }
}

bool blinkState = false;
// 깜빡이 모드: 모든 불이 동시에 깜빡임
void blinkModeTaskCallback() {
    if (blinkMode) {
        blinkState = !blinkState;
        analogWrite(RED_LED, blinkState ? brightness : 0);
        analogWrite(YELLOW_LED, blinkState ? brightness : 0);
        analogWrite(GREEN_LED, blinkState ? brightness : 0);
    }
}

// 일반 모드: 신호등 순서대로 점등
int normalModeStep = 0;
void normalModeTaskCallback() {
    if (!normalMode) return;

    switch (normalModeStep) {
        case 0: // 빨간불 켜기
            analogWrite(RED_LED, brightness);
            tNormalMode.setInterval(redDuration);
            break;
        case 1: // 노란불로 전환
            analogWrite(RED_LED, 0);
            analogWrite(YELLOW_LED, brightness);
            tNormalMode.setInterval(yellowDuration);
            break;
        case 2: // 초록불 켜기
            analogWrite(YELLOW_LED, 0);
            analogWrite(GREEN_LED, brightness);
            tNormalMode.setInterval(greenDuration);
            break;
        case 3: // 깜빡임 시작 (초록불 깜빡임 효과)
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
        case 9: // 노란불로 전환
            analogWrite(GREEN_LED, 0);
            analogWrite(YELLOW_LED, brightness);
            tNormalMode.setInterval(yellowDuration);
            break;
        case 10: // 모두 꺼짐 → 빨간불로 돌아감
            analogWrite(YELLOW_LED, 0);
            normalModeStep = -1;  // 다음 주기에 0부터 시작
            break;
    }
    normalModeStep++;
}

// 버튼 인터럽트 핸들러
void button1ISR() { button1Pressed = true; }
void button2ISR() { button2Pressed = true; }
void button3ISR() { button3Pressed = true; }

String serialInput = "";

// 시리얼 명령 처리
void parseSerialCommand(String input);

// 시리얼 입력 처리 (슬라이더나 버튼 클릭 등)
void handleSerialInput() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n') {
            serialInput.trim();
            if (serialInput.startsWith("cmd:")) {
                parseSerialCommand(serialInput);  // 명령어 처리
            } else {
                // 단일 키-값 설정 처리
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

// 시리얼 명령 파싱 및 적용 (cmd: 형태)
void parseSerialCommand(String input) {
    input = input.substring(4); // "cmd:" 제거
    int idx;
    while ((idx = input.indexOf(";")) != -1) {
        String token = input.substring(0, idx);
        input = input.substring(idx + 1);
        int eq = token.indexOf("=");
        if (eq == -1) continue;
        String key = token.substring(0, eq);
        String val = token.substring(eq + 1);

        // 키별로 기능 수행
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

// 현재 상태를 시리얼로 전송
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

// 초기 설정
void setup() {
    Serial.begin(9600);

    // 핀 모드 설정
    pinMode(RED_LED, OUTPUT);
    pinMode(YELLOW_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(BUTTON1, INPUT_PULLUP);
    pinMode(BUTTON2, INPUT_PULLUP);
    pinMode(BUTTON3, INPUT_PULLUP);

    // 인터럽트 연결
    attachPCINT(digitalPinToPCINT(BUTTON1), button1ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON2), button2ISR, FALLING);
    attachPCINT(digitalPinToPCINT(BUTTON3), button3ISR, FALLING);

    // Task 등록
    runner.init();
    runner.addTask(tUpdateBrightness);
    runner.addTask(tEmergencyMode);
    runner.addTask(tBlinkMode);
    runner.addTask(tNormalMode);

    // Task 실행 설정
    tUpdateBrightness.enable();
    tEmergencyMode.enable();
    tBlinkMode.disable();
    tNormalMode.enable();
}

// 메인 루프
void loop() {
    handleSerialInput();  // 시리얼 처리
    runner.execute();     // Task 실행
    sendStateToSerial();  // 상태 전송

    // 버튼1: 비상 모드 토글
    if (button1Pressed) {
        button1Pressed = false;
        toggleMode = !toggleMode;
        if (toggleMode) {
            // 비상모드 켜기
            tEmergencyMode.enable();
            tBlinkMode.disable();
            tNormalMode.disable();
            emergencyMode = true;
            analogWrite(RED_LED, brightness);
            analogWrite(YELLOW_LED, 0);
            analogWrite(GREEN_LED, 0);
        } else {
            // 비상모드 끄기
            emergencyMode = false;
            analogWrite(RED_LED, 0);
            tNormalMode.enable();
        }
    }

    // 버튼2: 깜빡이 모드 토글
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

    // 버튼3: 일반 모드 On/Off 토글
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