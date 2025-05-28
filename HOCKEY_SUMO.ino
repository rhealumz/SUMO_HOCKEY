#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi Credentials
const char* ssid = "LUMAYAG";
const char* password = "12345678";

WebServer server(80);

// Motor pins
const int IN1 = 14;
const int IN2 = 27;
const int IN3 = 26;
const int IN4 = 25;
const int ENA = 33;
const int ENB = 32;

// Sensor pins
const int backIRPin = 35;
const int trigPin = 13;
const int echoPin = 12;

// Battery monitoring pin
const int batteryPin = 34;  // Can be adjusted if needed

// Bot states
int motorSpeed = 100;
bool powerOn = true;
bool isAutoMode = false;

void setupMotors() {
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
}

void setupSensors() {
  pinMode(backIRPin, INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

// Movement functions
void moveForward() {
  if (!powerOn) return;
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  if (!powerOn) return;
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, motorSpeed);
}

void turnLeft() {
  if (!powerOn) return;
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, motorSpeed);
}

void turnRight() {
  if (!powerOn) return;
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, motorSpeed); analogWrite(ENB, 0);
}

void stopMotors() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

void forwardLeft() {
  if (!powerOn) return;
  // Slightly slower right motor to curve left
  analogWrite(ENA, motorSpeed / 2);  // Left motor slower
  analogWrite(ENB, motorSpeed);      // Right motor full
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void forwardRight() {
  if (!powerOn) return;
  analogWrite(ENA, motorSpeed);      // Left motor full
  analogWrite(ENB, motorSpeed / 2);  // Right motor slower
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void backwardLeft() {
  if (!powerOn) return;
  analogWrite(ENA, motorSpeed / 2);  // Left motor slower
  analogWrite(ENB, motorSpeed);      // Right motor full
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void backwardRight() {
  if (!powerOn) return;
  analogWrite(ENA, motorSpeed);      // Left motor full
  analogWrite(ENB, motorSpeed / 2);  // Right motor slower
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}


float readBatteryVoltage() {
  int sensorValue = analogRead(batteryPin);
  float voltage = sensorValue * (3.3 / 4095.0);
  return voltage * 2;  // Adjust for voltage divider
}

long readUltrasonicDistance() {
  digitalWrite(trigPin, LOW); delayMicroseconds(2);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  return (duration / 2) * 0.0343; // cm
}

void handleBattery() {
  float voltage = readBatteryVoltage();
  server.send(200, "application/json", "{\"voltage\": " + String(voltage) + "}"); 
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
  <title>Mini Sumo Bot Controller</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Quicksand:wght@500&display=swap');

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }

    body {
      font-family: 'Quicksand', sans-serif;
      background: linear-gradient(135deg, #f7f7f7, #e0e0e0);
      color: #333;
      height: 100vh;
      overflow: hidden;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      position: relative;
      background-size: 400% 400%;
      animation: gradientBackground 8s ease infinite;
    }

    h1 {
      font-size: 2.5rem;
      text-align: center;
      margin-bottom: 10px;
      color: #444;
      text-shadow: 0 0 10px rgba(0, 0, 0, 0.2);
    }

    #status, #battery {
      margin: 5px 0;
      font-size: 1rem;
      color: #555;
      text-shadow: 0 0 5px rgba(0, 0, 0, 0.1);
    }

    #modeButton {
      margin: 20px 0;
      padding: 12px 30px;
      font-size: 1.1rem;
      background: #7c4dff;
      color: #ffffff;
      border: 2px solid #7c4dff;
      border-radius: 12px;
      cursor: pointer;
      transition: all 0.3s ease-in-out;
      position: relative;
    }

    #modeButton:hover {
      background-color: #5e3eae;
      box-shadow: 0 0 12px #5e3eae;
    }

    input[type=range] {
  width: 250px;
  margin-top: 8px;
  appearance: none;
  height: 10px;
  background: #ccc;
  border-radius: 5px;
  outline: none;
  transition: background 0.3s;
}

input[type=range]::-webkit-slider-thumb {
  appearance: none;
  width: 20px;
  height: 20px;
  background: #7c4dff;
  border-radius: 50%;
  cursor: pointer;
  box-shadow: 0 0 5px rgba(124, 77, 255, 0.5);
}

    .controller {
      display: grid;
      grid-template-columns: repeat(3, 100px);
      grid-template-rows: repeat(3, 100px);
      gap: 20px;
      margin-top: 20px;
    }

    .btn {
      width: 100px;
      height: 100px;
      font-size: 2rem;
      color: #ffffff;
      background: rgba(124, 77, 255, 0.3);
      border-radius: 15px;
      display: flex;
      align-items: center;
      justify-content: center;
      cursor: pointer;
      transition: 0.3s;
      box-shadow: 0 0 10px rgba(124, 77, 255, 0.4);
    }

    .btn:hover {
      transform: scale(1.1);
      background: rgba(124, 77, 255, 0.5);
      box-shadow: 0 0 20px rgba(124, 77, 255, 0.5);
    }

    footer {
      margin-top: 30px;
      font-size: 0.9rem;
      color: #888;
    }

    /* Modern background animation */
    .bg-lines {
      position: absolute;
      width: 100%;
      height: 100%;
      background: radial-gradient(circle, rgba(124, 77, 255, 0.2) 30%, rgba(106, 125, 233, 0.1) 70%);
      animation: movingLines 10s linear infinite;
      z-index: 0;
      pointer-events: none;
    }

    @keyframes movingLines {
      0% {
        background-position: 0 0;
      }
      50% {
        background-position: 100% 100%;
      }
      100% {
        background-position: 0 0;
      }
    }

    @keyframes gradientBackground {
      0% { background-position: 0% 50%; }
      50% { background-position: 100% 50%; }
      100% { background-position: 0% 50%; }
    }
  </style>
</head>
<body>
  <div class="bg-lines"></div>

  <h1>Hockey Bot Controller</h1>
  <div id="status">Status: Connecting...</div>
  <div id="battery">Battery Voltage: -- V</div>
  <div id="distance">Ultrasonic Distance: -- cm</div>
  <div id="backIR">Back IR: --</div>

<button id="modeButton" onclick="toggleMode()">Switch to Auto Mode</button>


  <div class="controller">
    <div></div>
    <div class="btn" onclick="fetch('/forward')">⬆️</div>
    <div></div>
    <div class="btn" onclick="fetch('/left')">⬅️</div>
    <div class="btn" onclick="fetch('/stop')">⏹️</div>
    <div class="btn" onclick="fetch('/right')">➡️</div>
    <div></div>
    <div class="btn" onclick="fetch('/backward')">⬇️</div>
    <div></div>
  </div>

  <div style="margin: 20px 0; text-align: center;">
    <label for="speedRange" style="font-size: 1.1rem; color: #555;">Speed: <span id="speedValue">128</span></label><br>
    <input type="range" id="speedRange" min="0" max="255" value="128" oninput="adjustSpeed(this.value)">
  </div>
  
  <footer>Created by DEGAMO • Powered by ESP32</footer>

  <script>
    function toggleMode() {
      fetch('/toggleMode');
      let button = document.getElementById("modeButton");
      if (button.innerText === "Switch to Auto Mode") {
        button.innerText = "Switch to Manual Mode";
      } else {
        button.innerText = "Switch to Auto Mode";
      }
    }

    function updateStatus() {
      fetch('/')
        .then(() => {
          document.getElementById("status").innerText = "Status: ✅ Connected";
        })
        .catch(() => {
          document.getElementById("status").innerText = "Status: ❌ Not Connected";
        });
    }

    function updateBattery() {
      fetch('/battery')
        .then(response => response.json())
        .then(data => {
          document.getElementById("battery").innerText = "Battery Voltage: " + data.voltage.toFixed(2) + " V";
        });
    }

    function updateUltrasonic() {
  fetch('/ultrasonic')
    .then(response => response.json())
    .then(data => {
      document.getElementById("distance").innerText = "Ultrasonic Distance: " + data.distance + " cm";
    });
}

function updateBackIR() {
  fetch('/backir')
    .then(response => response.json())
    .then(data => {
      document.getElementById("backIR").innerText = "Back IR: " + (data.detected ? "🚫 Edge Detected" : "✅ Safe");
    });
}

function adjustSpeed(value) {
  document.getElementById("speedValue").innerText = value;
  fetch(`/setspeed?value=${value}`);
}

    setInterval(updateStatus, 3000);
    setInterval(updateBattery, 5000);
    setInterval(updateUltrasonic, 1000);
    setInterval(updateBackIR, 1000);
    window.onload = updateStatus;
  </script>
</body>
</html>

  )rawliteral");
}

void applyCurrentSpeed() {
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}


void setup() {
  Serial.begin(115200);
  setupMotors();
  setupSensors();

  WiFi.softAP(ssid, password);
  delay(100);

  Serial.println("AP IP address: " + WiFi.softAPIP().toString());

  // Routes
  server.on("/", handleRoot);
  server.on("/forward", []() { moveForward(); server.send(100, "text/plain", "Forward"); });
  server.on("/backward", []() { moveBackward(); server.send(100, "text/plain", "Backward"); });
  server.on("/left", []() { turnLeft(); server.send(100, "text/plain", "Left"); });
  server.on("/right", []() { turnRight(); server.send(100, "text/plain", "Right"); });
  server.on("/stop", []() { stopMotors(); server.send(100, "text/plain", "Stop"); });
  server.on("/forwardleft", []() { forwardLeft(); server.send(100, "text/plain", "Forward Left"); });
  server.on("/forwardright", []() { forwardRight(); server.send(100, "text/plain", "Forward Right"); });
  server.on("/backwardleft", []() { backwardLeft(); server.send(100, "text/plain", "Backward Left"); });
  server.on("/backwardright", []() { backwardRight(); server.send(100, "text/plain", "Backward Right"); });

  server.on("/battery", handleBattery);
  server.on("/toggleMode", []() {
    isAutoMode = !isAutoMode;
    server.send(200, "text/plain", isAutoMode ? "Auto Mode" : "Manual Mode");
  });

  server.on("/ultrasonic", []() {
    long distance = readUltrasonicDistance();
    server.send(200, "application/json", "{\"distance\": " + String(distance) + "}");
  });
  
  server.on("/irsensors", []() {
    int backIR = digitalRead(backIRPin);
    String backState = (backIR == LOW) ? "black" : "white";
    server.send(200, "application/json", 
      "{\"back\":\"" + backState + "\"}"
    );
  });

  server.on("/speedup", []() {
  motorSpeed += 10;
  if (motorSpeed > 255) motorSpeed = 255;
  server.send(200, "text/plain", "Speed: " + String(motorSpeed));
});

server.on("/speeddown", []() {
  motorSpeed -= 10;
  if (motorSpeed < 0) motorSpeed = 0;
  server.send(200, "text/plain", "Speed: " + String(motorSpeed));
});

server.on("/getspeed", []() {
  server.send(200, "application/json", "{\"speed\": " + String(motorSpeed) + "}");
});


  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  server.handleClient();

  if (isAutoMode) {
    int backIR = digitalRead(backIRPin);

    if (backIR == LOW) {
      // Avoid falling off back
      moveForward();
      delay(300);
      turnLeft();
      delay(300);
    } else {
      // Engage enemy
      long distance = readUltrasonicDistance();
      if (distance < 30) {
        motorSpeed = 255;  // full speed
      } else {
        motorSpeed = 150;  // slower speed
      }
      applyCurrentSpeed();
      moveForward();
    }
  }
}