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

  port = createSerial();
  let usedPorts = usedSerialPorts();
  if (usedPorts.length > 0) {
    port.open(usedPorts[0], 9600);
  }

  connectBtn = createButton("Connect to Arduino");
  connectBtn.position(10, height - 40);
  connectBtn.mousePressed(togglePort);

  redSlider = createSlider(100, 5000, redTime, 100);
  redSlider.position(10, 20);
  redSlider.input(() => {
    redTime = redSlider.value();
    sendCommand(`redTime=${redTime}`);
    if (ledRed === 1 && mode === "normal") {
      sendCommand(`apply=red`);
    }
  });

  yellowSlider = createSlider(100, 5000, yellowTime, 100);
  yellowSlider.position(10, 60);
  yellowSlider.input(() => {
    yellowTime = yellowSlider.value();
    sendCommand(`yellowTime=${yellowTime}`);
    if (ledYellow === 1 && mode === "normal") {
      sendCommand(`apply=yellow`);
    }
  });

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