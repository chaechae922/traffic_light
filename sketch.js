let port;
let connectBtn;
let redSlider, yellowSlider, greenSlider;
let redTime = 2000, yellowTime = 500, greenTime = 2000;
let brightness = 0;
let mode = "unknown";
let ledRed = 0, ledYellow = 0, ledGreen = 0;

function setup() {
  createCanvas(700, 500);
  textSize(16);

  // 시리얼 포트 초기화
  port = createSerial();
  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600); // 이전에 사용했던 포트 자동 연결
  }

  // 아두이노 연결 버튼 생성
  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(10, height - 40);
  connectBtn.mousePressed(togglePort);

  // 빨간불 슬라이더 생성 및 이벤트 설정
  redSlider = createSlider(100, 5000, redTime, 100);
  redSlider.position(10, 20);
  redSlider.input(() => {
    redTime = redSlider.value();
    sendCommand(`redTime=${redTime}`);
    if (ledRed === 1 && mode === "normal") {
      sendCommand(`apply=red`);
    }
  });

  // 노란불 슬라이더 생성 및 이벤트 설정
  yellowSlider = createSlider(100, 5000, yellowTime, 100);
  yellowSlider.position(10, 60);
  yellowSlider.input(() => {
    yellowTime = yellowSlider.value();
    sendCommand(`yellowTime=${yellowTime}`);
    if (ledYellow === 1 && mode === "normal") {
      sendCommand(`apply=yellow`);
    }
  });

  // 초록불 슬라이더 생성 및 이벤트 설정
  greenSlider = createSlider(100, 5000, greenTime, 100);
  greenSlider.position(10, 100);
  greenSlider.input(() => {
    greenTime = greenSlider.value();
    sendCommand(`greenTime=${greenTime}`);
    if (ledGreen === 1 && mode === "normal") {
      sendCommand(`apply=green`);
    }
  });
}

function draw() {
  background(230);

  // 시리얼 데이터 읽기 (최대 10줄까지)
  let count = 0;
  while (port.available() > 0 && count < 10) {
    const data = port.readUntil("\n").trim();
    if (data.length > 0) parseSerialData(data);
    count++;
  }

  // 텍스트로 현재 상태 표시
  fill(0);
  text(`Mode: ${mode}`, 70, 160);
  text(`Brightness: ${brightness}`, 70, 180);
  text(`LED(R/Y/G): ${ledRed}, ${ledYellow}, ${ledGreen}`, 80, 200);
  text(`Red Time: ${redTime}`, 350, 30);
  text(`Yellow Time: ${yellowTime}`, 350, 70);
  text(`Green Time: ${greenTime}`, 350, 110);

  // 각 색상별 LED 시각화
  drawLED(width / 2, 250, ledRed, "RED", color(255, 0, 0));
  drawLED(width / 2, 330, ledYellow, "YELLOW", color(255, 255, 0));
  drawLED(width / 2, 410, ledGreen, "GREEN", color(0, 255, 0));
}

// LED 원형 시각화 함수
function drawLED(x, y, on, label, ledColor) {
  fill(on ? ledColor : "gray");  // 켜지면 색상, 꺼지면 회색
  stroke(0);
  ellipse(x, y, 60);             // 원형 LED
  fill(0);
  noStroke();
  textAlign(CENTER);
  text(label, x, y + 40);        // 라벨 표시
}

// 아두이노 포트 연결/해제
function togglePort() {
  if (!port.opened()) port.open(9600);
  else port.close();
}

// 시리얼 명령 전송 함수
function sendCommand(cmd) {
  if (port.opened()) {
    port.write(`cmd:${cmd};\n`);
  }
}

// 시리얼로 받은 상태 문자열 파싱
function parseSerialData(data) {
  let parts = data.split(",");
  parts.forEach(p => {
    const idx = p.indexOf(":");
    if (idx === -1) return;

    let key = p.substring(0, idx).trim();
    let val = p.substring(idx + 1).trim();

    // 키값에 따라 각 변수 갱신
    if (key === "mode") mode = val;
    else if (key === "brightness") brightness = int(val);
    else if (key === "red") ledRed = int(val);
    else if (key === "yellow") ledYellow = int(val);
    else if (key === "green") ledGreen = int(val);
  });
}