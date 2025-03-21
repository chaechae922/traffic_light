# 신호등 시스템 회로 및 코드 설명

## 0. 유튜브 동작 영상
https://youtu.be/aK3FVobjxvE?si=4kAf5xY62c8oBQFf

## 1. 개요
본 프로젝트는 Arduino 기반의 신호등 제어 시스템입니다. 
버튼을 이용하여 다양한 모드를 변경할 수 있으며, 가변저항을 사용하여 LED의 밝기를 조절. 
TaskScheduler와 PinChangeInterrupt 라이브러리를 활용하여 신호등의 상태를 관리.
p5.js를 통해 LED 상태와 모드전환을 제어.

## 2. 회로 구성
다음은 Arduino와 연결된 신호등 시스템의 회로 구성도입니다.

![image](https://github.com/user-attachments/assets/0bb31a83-2848-43df-9ad3-0dc18720c3f1)
![image](https://github.com/user-attachments/assets/0734a13d-ee7f-4e89-b45e-8dec34cafe72)

### 2-1. 부품 목록
- **Arduino Uno**
- **LED** (빨강, 노랑, 초록) 각 1개씩
- **저항 (220Ω)** 3개 (각 LED 직렬 연결)
- **푸시 버튼** 3개
- **가변저항** 1개
- **브레드보드 및 점퍼 와이어**

### 2-2. 핀 연결
| 기능 | 핀 번호 | 부품 | 설명 |
|------|--------|------|------|
| 빨간 LED | 5 | LED | 신호등 빨간불 |
| 노란 LED | 6 | LED | 신호등 노란불 |
| 초록 LED | 7 | LED | 신호등 초록불 |
| 버튼 1 | 2 | 푸시 버튼 | 비상 모드 (빨간불 고정) |
| 버튼 2 | 3 | 푸시 버튼 | 모든 LED 깜빡임 모드 |
| 버튼 3 | 4 | 푸시 버튼 | 신호등 ON/OFF |
| 가변저항 | A0 | 가변저항 | LED 밝기 조절 |

### 2-3. 회로 구성 원리
1. **LED 출력**: 각 LED는 220Ω 저항과 함께 Arduino의 디지털 핀(5, 6, 7)에 연결
2. **버튼 입력**: 버튼은 기본적으로 `INPUT_PULLUP` 설정을 사용하여 내부 풀업 저항을 활성화. 따라서 버튼이 눌리지 않았을 때 HIGH, 눌렸을 때 LOW 신호 발생.
3. **가변저항 입력**: 가변저항은 A0 핀에 연결, `analogRead()`를 이용해 0~1023 범위의 값을 읽고 LED 밝기로 변환됩니다.

### 3. 코드 설명
1. 라이브러리:
   - TaskScheduler: 모드전환을 비동기적으로 동작하도록 설계.
   - PinChangeInterrupt: 버튼 입력 시 빠른 모드전환 구현.
2. p5.js:
   - 웹 UI를 통해 아두이노 신호등 시스템을 제어하고 상태를 시각화
   - 슬라이더 입력 → sendCommand()로 아두이노에 명령 전송
   - 아두이노 → 시리얼로 현재 상태 전송 (mode, brightness, red, yellow, green)
   - parseSerialData()로 파싱해서 화면에 반영
   - draw()에서 LED 상태 및 텍스트 출력
     
### 3-1. 주요 기능
1. **기본 신호등 모드**
   - 빨간불 2초 → 노란불 0.5초 → 초록불 2초 → 1초내 초록불 깜빡임(3회) → 노란불 0.5초 → 반복
2. **버튼 1 (비상 빨간불 모드)**
   - 버튼을 누르면 신호등을 중단하고 빨간불 고정
   - 다시 누르면 신호등 모드 재개
3. **버튼 2 (모든 LED 깜빡임 모드)**
   - 버튼을 누르면 모든 LED가 500ms 간격으로 깜빡임
   - 다시 누르면 신호등 모드 재개
4. **버튼 3 (신호등 전체 ON/OFF)**
   - 버튼을 누르면 신호등 기능이 중단되며 모든 LED가 꺼짐
   - 다시 누르면 신호등 모드 재개

## 4. 설치 및 실행 방법
3. **코드 업로드**
   - Arduino IDE에서 제공된 코드를 Arduino 보드에 업로드합니다.
4. **회로 연결**
   - 위의 회로 구성도를 참고하여 배선을 완료합니다.
5. **작동 확인**
   - 버튼을 눌러 모드를 변경하고, 가변저항을 조절하여 LED 밝기 확인
  
## 5. p5.js
1. Normal Mode
   
![image](https://github.com/user-attachments/assets/9b15d050-2bf1-4f00-9a6a-47e9691703f9)

2. Emergency Mode

![image](https://github.com/user-attachments/assets/e8b574ab-611c-4e57-b1e0-cb8dff2c4ade)

3. Blink Mode

![image](https://github.com/user-attachments/assets/0f1c09be-a1f5-4b76-bbde-fb337a290295)

4. On/Off Mode

![image](https://github.com/user-attachments/assets/a743e8a2-453d-4794-8cf3-27feedf20652)





