let port;
let connectBtn;
let redSlider, yellowSlider, greenSlider;
let redTime = 2000, yellowTime = 500, greenTime = 2000;
let brightness = 0;
let mode = "unknown";
let ledRed = 0, ledYellow = 0, ledGreen = 0;

let video;
let handpose;
let predictions = [];

let lastGesture = "";
let gestureCooldown = 1000; // 1초 쿨다운
let lastGestureTime = 0;

function setup() {
  createCanvas(700, 500);
  textSize(16);

  video = createCapture(VIDEO);
  video.size(640, 480);
  video.hide();

  handpose = ml5.handpose(video, () => {
    console.log("🤖 Handpose model loaded!");
  });

  handpose.on("predict", results => {
    predictions = results;
  });

  port = createSerial();
  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) port.open(usedPorts[0], 9600);

  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(10, height - 40);
  connectBtn.mousePressed(togglePort);

  redSlider = createSlider(100, 5000, redTime, 100);
  redSlider.position(10, 20);
  redSlider.input(() => {
    redTime = redSlider.value();
    sendCommand(`redTime=${redTime}`);
    if (ledRed === 1 && mode === "normal") sendCommand(`apply=red`);
  });

  yellowSlider = createSlider(100, 5000, yellowTime, 100);
  yellowSlider.position(10, 60);
  yellowSlider.input(() => {
    yellowTime = yellowSlider.value();
    sendCommand(`yellowTime=${yellowTime}`);
    if (ledYellow === 1 && mode === "normal") sendCommand(`apply=yellow`);
  });

  greenSlider = createSlider(100, 5000, greenTime, 100);
  greenSlider.position(10, 100);
  greenSlider.input(() => {
    greenTime = greenSlider.value();
    sendCommand(`greenTime=${greenTime}`);
    if (ledGreen === 1 && mode === "normal") sendCommand(`apply=green`);
  });
}

function draw() {
  background(230);
  image(video, 20, 270, 160, 120);
  drawKeypoints();

  let count = 0;
  while (port.available() > 0 && count < 10) {
    const data = port.readUntil("\n").trim();
    if (data.length > 0) parseSerialData(data);
    count++;
  }

  fill(0);
  text(`Mode: ${mode}`, 70, 160);
  text(`Brightness: ${brightness}`, 70, 180);
  text(`LED(R/Y/G): ${ledRed}, ${ledYellow}, ${ledGreen}`, 80, 200);
  text(`Red Time: ${redTime}`, 350, 30);
  text(`Yellow Time: ${yellowTime}`, 350, 70);
  text(`Green Time: ${greenTime}`, 350, 110);

  drawLED(width / 2, 250, ledRed, "RED", color(255, 0, 0));
  drawLED(width / 2, 330, ledYellow, "YELLOW", color(255, 255, 0));
  drawLED(width / 2, 410, ledGreen, "GREEN", color(0, 255, 0));

  handleGestures();
}

function drawKeypoints() {
  if (predictions.length > 0) {
    const hand = predictions[0];
    const scaleX = 160 / video.width;
    const scaleY = 120 / video.height;

    for (let i = 0; i < hand.landmarks.length; i++) {
      const [x, y] = hand.landmarks[i];
      const sx = x * scaleX + 20;
      const sy = y * scaleY + 270;
      fill(255, 0, 0);
      noStroke();
      circle(sx, sy, 6);
    }
  }
}

function drawLED(x, y, on, label, ledColor) {
  fill(on ? ledColor : "gray");
  stroke(0);
  ellipse(x, y, 60);
  fill(0);
  noStroke();
  textAlign(CENTER);
  text(label, x, y + 40);
}

function togglePort() {
  if (!port.opened()) port.open(9600);
  else port.close();
}

function sendCommand(cmd) {
  if (port.opened()) {
    port.write(`cmd:${cmd};\n`);
  }
}

function parseSerialData(data) {
  let parts = data.split(",");
  parts.forEach(p => {
    const idx = p.indexOf(":");
    if (idx === -1) return;
    let key = p.substring(0, idx).trim();
    let val = p.substring(idx + 1).trim();
    if (key === "mode") mode = val;
    else if (key === "brightness") brightness = int(val);
    else if (key === "red") ledRed = int(val);
    else if (key === "yellow") ledYellow = int(val);
    else if (key === "green") ledGreen = int(val);
  });
}

// 👋 제스처 처리
function handleGestures() {
  if (predictions.length === 0) return;

  const hand = predictions[0];
  const landmarks = hand.landmarks;
  const now = millis();

  let gesture = "";

  if (isFist(landmarks)) gesture = "onoff";
  else if (isOpenHand(landmarks)) gesture = "emergency";
  else if (isVSign(landmarks)) gesture = "blink";
  else if (isThumbUp(landmarks)) gesture = "normal";

  if (gesture && gesture !== lastGesture && now - lastGestureTime > gestureCooldown) {
    if (gesture === "onoff") sendCommand("button=3");
    else if (gesture === "emergency") sendCommand("button=1");
    else if (gesture === "blink") sendCommand("button=2");
    else if (gesture === "normal") {
      sendCommand("button=3"); // Off
      setTimeout(() => sendCommand("button=3"), 100); // 다시 On
    }

    lastGesture = gesture;
    lastGestureTime = now;
  }
}

// 제스처 판별 함수들
function isFist(landmarks) {
  const tips = [8, 12, 16, 20];
  const mcps = [5, 9, 13, 17];
  for (let i = 0; i < tips.length; i++) {
    if (landmarks[tips[i]][1] < landmarks[mcps[i]][1]) return false;
  }
  return landmarks[4][0] < landmarks[3][0];
}

function isOpenHand(landmarks) {
  const tips = [4, 8, 12, 16, 20];
  const bases = [2, 5, 9, 13, 17];
  for (let i = 0; i < tips.length; i++) {
    if (landmarks[tips[i]][1] >= landmarks[bases[i]][1]) return false;
  }
  return true;
}

function isVSign(landmarks) {
  const i = landmarks[8];
  const m = landmarks[12];
  const r = landmarks[16];
  return dist(i[0], i[1], m[0], m[1]) > 50 && dist(m[0], m[1], r[0], r[1]) < 30;
}

function isThumbUp(landmarks) {
  const thumbTip = landmarks[4];
  const thumbBase = landmarks[2];
  const indexBase = landmarks[5];
  return thumbTip[1] < thumbBase[1] && thumbTip[1] < indexBase[1];
}