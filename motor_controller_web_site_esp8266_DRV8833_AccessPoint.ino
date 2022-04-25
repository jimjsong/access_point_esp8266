
// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Motor A
int pwmA = 16;
int in1A = 5;

// Motor B
int pwmB = 4;
int in1B = 0;

// Setting PWM properties
const int freq = 30000;
int dutyCycleA = 205; 
int dutyCycleB = 205; 


// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

int direction = 0; //0 - forward, 1 - backward

#ifndef APSSID
#define APSSID "ESPap"
#define APPSK  "12345678"
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;


void stopMotors() {
 
  direction = 0;
  dutyCycleA = 0;
  dutyCycleB = 0;
  
  digitalWrite(in1A, LOW);
  digitalWrite(in1B, LOW);
}

void setForward() {
  if (direction == 0) {
    stopMotors();
    delay(200);
  }
  direction = 1;
  digitalWrite(in1A, HIGH);
  digitalWrite(in1B, HIGH);
}
  
void setBackward() {
  if (direction == 1) {
    stopMotors();
    delay(200);
  }
  direction = 0;
  digitalWrite(in1A, LOW);
  digitalWrite(in1B, LOW);
}

void setup() {
  delay(1000);
  // Initialize the output variables as outputs
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point");
  // Set all the motor control pins to outputs
  pinMode(in1A, OUTPUT);
  pinMode(in1B, OUTPUT);
  stopMotors();
  analogWriteFreq(freq);

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("HTTP server started");
}


void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
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
            if (header.indexOf("GET /ST") >= 0) {
              Serial.println("STOP");
              stopMotors();
            } else if (header.indexOf("GET /FW") >= 0) {
              setForward();
              dutyCycleB = 205; // Right Motor Speed should be between 200 and 255
              dutyCycleA = 205; // Left Motor Speed should be between 200 and 255
            } else if (header.indexOf("GET /FL") >= 0) {
              if (dutyCycleA < 255)
                dutyCycleA += 10; // Right Motor Speed should be between 200 and 255
            } else if (header.indexOf("GET /BL") >= 0) {
              if(dutyCycleA > 10) 
                dutyCycleA -= 10;
            } else if (header.indexOf("GET /FR") >= 0) {
              if (dutyCycleB < 255)
                dutyCycleB += 10; // Right Motor Speed should be between 200 and 255
            } else if (header.indexOf("GET /BR") >= 0) {
              if(dutyCycleB > 10) 
                dutyCycleB -= 10;
            } else if (header.indexOf("GET /LE") >= 0) {
              Serial.println(direction);
              dutyCycleB = 185; 
              dutyCycleA = 225;
            } else if (header.indexOf("GET /RI") >= 0) {
              Serial.println(direction);
              dutyCycleB = 225; 
              dutyCycleA = 185;
            } else if (header.indexOf("GET /BA") >= 0) {
              dutyCycleB = 185; 
              dutyCycleA = 185;
              setBackward();
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; width: 100px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            
            String valueString = "<tr><td><center>" + String(dutyCycleA) + "</center></td><td> &nbsp; </td><td><center>" + String(dutyCycleB) + "</center></td></tr>";
            client.println("<body><h1>ESP8266 Web Server</h1>");
            client.println("<center><table>");
            client.println(valueString);
            client.println("<tr><td><a href=\"/FL\"><button class=\"button\">+</button></a></td><td><a href=\"/FW\"><button class=\"button\">FWD</button></a></td><td><a href=\"/FR\"><button class=\"button\">+</button></a></td></tr>");
            client.println("<tr><td><a href=\"/LE\"><button class=\"button\">LFT</button></a></td><td><a href=\"/ST\"><button class=\"button\">STP</button></a></td><td><a href=\"/RI\"><button class=\"button\">RHT</button></a></td></tr>");
            client.println("<tr><td><a href=\"/BL\"><button class=\"button\">-</button></a></td><td><a href=\"/BA\"><button class=\"button\">BCK</button></a></td><td><a href=\"/BR\"><button class=\"button\">-</button></a></td></tr></table></center></body></html>");
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
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  if (direction == 0 ) {
    analogWrite(pwmA, dutyCycleA);
    analogWrite(pwmB, dutyCycleB);
  } else {
    analogWrite(pwmA, 255 - dutyCycleA);
    analogWrite(pwmB, 255 - dutyCycleB);
  }
    

}
