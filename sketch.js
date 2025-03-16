let serial;
let portButton;
let brightness = 255;
let mode = "Normal";
let redState = false;
let yellowState = false;
let greenState = false;

function setup() {
    createCanvas(200, 400);
    
    // 시리얼 포트 연결 버튼 생성
    portButton = createButton("Connect to Arduino");
    portButton.position(10, 10);
    portButton.mousePressed(connectSerial);
}

async function connectSerial() {
    if (!navigator.serial) {
        alert("Web Serial API is not supported in this browser. Please use Chrome or Edge.");
        return;
    }

    try {
        const port = await navigator.serial.requestPort(); // 사용자에게 포트 선택 창 표시
        await port.open({ baudRate: 9600 });

        serial = port;
        console.log("Connected to Arduino");

        // 시리얼 데이터 읽기 시작
        readSerialData();
    } catch (err) {
        console.error("Serial connection failed:", err);
    }
}

async function readSerialData() {
    if (!serial) return;

    const reader = serial.readable.getReader();
    
    try {
        while (true) {
            const { value, done } = await reader.read();
            if (done) break;

            let data = new TextDecoder().decode(value).trim();
            console.log("Received:", data);

            processSerialData(data);
        }
    } catch (err) {
        console.error("Error reading serial data:", err);
    } finally {
        reader.releaseLock();
    }
}

function processSerialData(data) {
    let parts = data.split(", ");
    for (let part of parts) {
        if (part.includes("Mode: ")) mode = part.split(": ")[1];
        else if (part.includes("Brightness: ")) brightness = int(part.split(": ")[1]);
        else if (part.includes("Red: ")) redState = part.split(": ")[1] === "1";
        else if (part.includes("Yellow: ")) yellowState = part.split(": ")[1] === "1";
        else if (part.includes("Green: ")) greenState = part.split(": ")[1] === "1";
    }

    // UI 업데이트
    document.getElementById("brightnessValue").innerText = brightness;
    document.getElementById("modeIndicator").innerText = mode;
    updateTrafficLight();
}

function updateTrafficLight() {
    document.getElementById("redLight").style.backgroundColor = redState ? "red" : "gray";
    document.getElementById("yellowLight").style.backgroundColor = yellowState ? "yellow" : "gray";
    document.getElementById("greenLight").style.backgroundColor = greenState ? "green" : "gray";
}