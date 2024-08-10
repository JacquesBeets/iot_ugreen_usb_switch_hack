#include <Arduino.h>
#include <ESP8266WiFi.h>


// Replace with your network credentials
const char* ssid     = "BeetsDeBeer";
const char* password = "831126jac";


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current state of the USB hub (PC or Mac)
String hubState = "PC";

// Assign output variable to GPIO pin
const int switchPin = 5;  // D1 on Wemos D1 mini
const int monitorPin = 4; // D2 on Wemos D1 mini for monitoring channel state

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void updateHubState() {
  String newState = digitalRead(monitorPin) == HIGH ? "PC" : "Mac";
  if (newState != hubState) {
    hubState = newState;
    Serial.println("Hub state changed to: " + hubState);
  }
}

void setup() {
  Serial.begin(115200);
  // Initialize the output variable as output
  pinMode(switchPin, OUTPUT);
  pinMode(monitorPin, INPUT);
  // Set output to HIGH (button "released")
  digitalWrite(switchPin, HIGH);

  // Initialize hubState based on current channel
  hubState = digitalRead(monitorPin) == HIGH ? "PC" : "Mac";

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void switchHub() {
  digitalWrite(switchPin, LOW);  // "Press" the button
  delay(100);  // Hold for 100ms
  digitalWrite(switchPin, HIGH);  // "Release" the button
  updateHubState();  // Update the state after switching
}

void loop(){
  updateHubState();  // Check for manual changes
  
  WiFiClient client = server.accept();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /switch") >= 0) {
              switchHub();
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>USB Hub Switch</h1>");
            
            client.println("<p>Current State: " + hubState + "</p>");
            String buttonText = String("<p><a href=\"/switch\"><button class=\"button\">Switch to ") + 
                                (hubState == "PC" ? "Mac" : "PC") + "</button></a></p>";
            client.println(buttonText);
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
  }
}