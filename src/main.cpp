#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


// Replace with your network credentials
const char* ssid     = "BeetsDeBeer";
const char* password = "831126jac";


ESP8266WebServer server(80);

String hubState = "PC";
const int switchPin = 5;  // D1 on Wemos D1 mini
const int monitorPin = 4; // D2 on Wemos D1 mini for monitoring channel state


void updateHubState() {
  String newState = digitalRead(monitorPin) == HIGH ? "PC" : "Mac";
  if (newState != hubState) {
    hubState = newState;
    Serial.println("Hub state changed to: " + hubState);
  }
}

void switchHub() {
  digitalWrite(switchPin, LOW);
  delay(100);
  digitalWrite(switchPin, HIGH);
  delay(500);
  updateHubState();
}

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family: Arial; text-align: center; color: #ffffff; background:#000000;} #switchButton{background-color: #4CAF50; border: none; color: white; padding: 15px 32px; text-align: center; text-decoration: none; display: inline-block; text-decoration: none; font-size: 16px; margin: 4px 2px; cursor: pointer;}</style>";
  html += "<script>function updateState() {fetch('/state').then(response => response.text()).then(state => {document.getElementById('state').innerText = state;document.getElementById('switchButton').innerText = 'Switch to ' + (state === 'PC' ? 'Mac' : 'PC');});} setInterval(updateState, 5000);</script>";
  html += "</head><body>";
  html += "<h1>USB Hub Switch</h1>";
  html += "<p>Current State: <span id='state'>" + hubState + "</span></p>";
  html += String("<a href='/switch' id='switchButton'>Switch to ") + (hubState == "PC" ? "Mac" : "PC") + "</a>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleSwitch() {
  switchHub();
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleState() {
  updateHubState();
  server.send(200, "text/plain", hubState);
}

void setup() {
  Serial.begin(115200);
  pinMode(switchPin, OUTPUT);
  digitalWrite(switchPin, HIGH);
  pinMode(monitorPin, INPUT);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Initialize hubState based on current channel
  updateHubState();

  server.on("/", handleRoot);
  server.on("/switch", handleSwitch);
  server.on("/state", handleState);
  server.begin();
}

void loop() {
  updateHubState();
  server.handleClient();
}