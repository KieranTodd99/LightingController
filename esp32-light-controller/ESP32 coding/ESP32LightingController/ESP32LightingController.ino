// Load Wi-Fi library
#include<Adafruit_NeoPixel.h>
#include<WiFi.h>
#include<array>
#include "DHT.h"
#define DHTPIN 14
#define DHTTYPE DHT22
#define pirPin 34
#include "time.h"

DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "BT-MQAK2M";
const char* password = "HMNGChXAvhcLx9";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 3600;

bool webActive = false;

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

const int freq = 5000;
const int resolution = 8;

class LEDprofile{
  public:
  String trigger;
  int shr;
  int smin;
  int ehr;
  int emin;
  float degC;
  bool below = true;
  
  String id;
  int leng;
  int *brightnes;
  int lengPos = 0;
  int duration;
  int pinOut;
  int count = 0;
  int LEDchannel;
  void arrayInitalise(int l){
    leng = l;
    brightnes = new int[leng];
  }  
};

class AssignLEDprofile {
  public:
  String trigger;
  int shr;
  int smin;
  int ehr;
  int emin;
  int degC;
  bool below = true;
  
  String id;
  int lengPos = 0;
  int count = 0;
  int duration;
  int pin;
  int leng;
  int index;
  int *red;
  int *green;
  int *blue;
  bool active = true;
  void arrayInitalise(int l){
    leng = l;
    red = new int[leng];
    green = new int[leng];
    blue = new int[leng];
  }  
};

class ringLight{
  public:
  // if true all LEDs on ring light activate at the same time, if false each LED activate individually
  bool thisOrThat;
  String trigger;
  int shr;
  int smin;
  int ehr;
  int emin;
  int degC;
  bool below = true;
  
  int pos = 0;
  int leng;
  AssignLEDprofile* LEDs;
  void arrayInitalise(int l){
        leng = l;
        LEDs = new AssignLEDprofile[leng];
  }
  void addElement(AssignLEDprofile temp){
    LEDs[pos] = temp;
    pos++;
  }
};

LEDprofile* inactiveLEDs= new LEDprofile[20];
LEDprofile* activeLEDs= new LEDprofile[20];
ringLight* inactiveRingLEDs = new ringLight[10];
ringLight* activeRingLEDs = new ringLight[10];
int profileCount = 0;
int inprofileCount = 0;
int ringProfileCount = 0;
int inringProfileCount = 0;

Adafruit_NeoPixel pixel(8, 17, NEO_GRB + NEO_KHZ800);

void inactiveToActive (int index){
  activeLEDs[profileCount] = inactiveLEDs[index];
  profileCount++;
  index++;
  if (inprofileCount == 1){
    inactiveLEDs = new LEDprofile[20];
  }
  else {
    while (index < inprofileCount){
      inactiveLEDs[index-1] = inactiveLEDs[index];
      index++;
    }
  }
  inprofileCount--;
}

void activeToInactive (int index){
  inactiveLEDs[inprofileCount] = activeLEDs[index];
  inprofileCount++;
  index++;
  if (profileCount == 1){
    activeLEDs= new LEDprofile[20];
  }
  else{
    while (index < profileCount){
      activeLEDs[index-1] = activeLEDs[index];
      index++;
    } 
  }
  profileCount--;
}

void colourintoactive(int index){
  activeRingLEDs[ringProfileCount] = inactiveRingLEDs[index];
  ringProfileCount++;
  index++;
  if (inringProfileCount == 1){
    inactiveRingLEDs = new ringLight[10];
  }
  else{
    while (index < inringProfileCount){
      inactiveRingLEDs[index-1] = inactiveRingLEDs [index];
      index++;
    }
  }
  inringProfileCount--;
}

void colouractivetoin(int index){
  inactiveRingLEDs[inringProfileCount] = activeRingLEDs[index];
  inringProfileCount++;
  index++;
  if (ringProfileCount == 1){
    activeRingLEDs = new ringLight[10];
  }
  else{
    while (index < ringProfileCount){
      activeRingLEDs[index-1] = activeRingLEDs [index];
      index++;
    }
  }
  ringProfileCount--;
}

int findProfile (int pin){
  int pos = 0;
  while (pos < 20){
    if  (inactiveLEDs[pos].pinOut == pin){
      return pos;
      Serial.println("found profile");
      break;
    }else{pos++;}
  }
  Serial.println("Error could not find");
  return 21;
}

int charToInt (char tens, char units){
  int counter = 0;
  int finalInt = 0;
  String numbers = "0123456789";
  while (counter <= 5){
    if (tens == numbers.charAt(counter)){
      finalInt = counter*10;
      counter = 3;
    }else{counter++;}
  }
  counter = 0;
  while (counter <= 9){
    if (units == numbers.charAt(counter)){
      finalInt = finalInt+counter;
      counter = 10;
    }else{counter++;}
  }
  return finalInt;
}

void setup() {
  Serial.begin(115200);
  // Print local IP address and start web server

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

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  server.begin();
  
  ledcSetup(0, freq, resolution);
  ledcSetup(1, freq, resolution);
  ledcAttachPin(12, 0);
  ledcAttachPin(13, 1);

  LEDprofile prof16;
  prof16.arrayInitalise(6);
  prof16.brightnes[0] = 0;
  prof16.brightnes[1] = 50;
  prof16.brightnes[2] = 255;
  prof16.brightnes[3] = 255;
  prof16.brightnes[4] = 50;
  prof16.brightnes[5] = 0;
  prof16.duration = 1;
  prof16.pinOut = 12;
  prof16.LEDchannel = 0;
  prof16.id = "Gradual";
  prof16.trigger = "Sensor";
  inactiveLEDs[1]= prof16;
  inprofileCount++;

  LEDprofile prof17;
  prof17.arrayInitalise(9);
  prof17.brightnes[0] = 0;
  prof17.brightnes[1] = 50;
  prof17.brightnes[2] = 100;
  prof17.brightnes[3] = 150;
  prof17.brightnes[4] = 255;
  prof17.brightnes[5] = 150;
  prof17.brightnes[6] = 100;
  prof17.brightnes[7] = 50;
  prof17.brightnes[8] = 0;
  prof17.duration = 2;
  prof17.pinOut = 13;
  prof17.LEDchannel = 1;
  prof17.id = "flash";
  prof17.trigger = "Time";
  prof17.shr = 12;
  prof17.smin = 05;
  prof17.ehr = 17;
  prof17.emin = 00;
  inactiveLEDs[0]= prof17;
  inprofileCount++;
  
  ringLight rL1;
  rL1.trigger = "Sensor";
  rL1.arrayInitalise(8);
  
  AssignLEDprofile LED1;
  LED1.arrayInitalise(8);
  LED1.index = 0;
  LED1.duration = 10; 
  LED1.red = new int[150, 150, 100, 0, 0, 0, 0, 0];
  LED1.green = new int[0, 0, 100, 150, 100, 0, 0, 0];
  LED1.blue = new int[0, 0, 0, 0, 100, 150, 100, 0];
  rL1.addElement(LED1);

  AssignLEDprofile LED2;
  LED2.arrayInitalise(8);
  LED2.index = 1;
  LED2.duration = 10; 
  LED2.red = new int[0, 150, 150, 100, 0, 0, 0, 0];
  LED2.green = new int[0, 0, 0, 100, 150, 100, 0, 0];
  LED2.blue = new int[0, 0, 0, 0, 0, 100, 150, 100];
  rL1.addElement(LED2);

  AssignLEDprofile LED3;
  LED3.arrayInitalise(8);
  LED3.index = 2;
  LED3.duration = 10; 
  LED3.red = new int[0, 0, 150, 150, 100, 0, 0, 0];
  LED3.green = new int[0, 0, 0, 0, 100, 150, 100, 0];
  LED3.blue = new int[100, 0, 0, 0, 0, 0, 100, 150];
  rL1.addElement(LED3);

  AssignLEDprofile LED4;
  LED4.arrayInitalise(8);
  LED4.index = 3;
  LED4.duration = 10; 
  LED4.red = new int[0, 0, 0, 150, 150, 100, 0, 0];
  LED4.green = new int[0, 0, 0, 0, 0, 100, 150, 100];
  LED4.blue = new int[150, 100, 0, 0, 0, 0, 0, 100];
  rL1.addElement(LED4);

  AssignLEDprofile LED5;
  LED5.arrayInitalise(8);
  LED5.index = 4;
  LED5.duration = 10; 
  LED5.red = new int[0, 0, 0, 0, 150, 150, 100, 0];
  LED5.green = new int[100, 0, 0, 0, 0, 0, 100, 150];
  LED5.blue = new int[100, 150, 100, 0, 0, 0, 0, 0];
  rL1.addElement(LED5);

  AssignLEDprofile LED6;
  LED6.arrayInitalise(8);
  LED6.index = 5;
  LED6.duration = 10; 
  LED6.red = new int[0, 0, 0, 0, 0, 150, 150, 100];
  LED6.green = new int[150, 100, 0, 0, 0, 0, 0, 100];
  LED6.blue = new int[0, 100, 150, 100, 0, 0, 0, 0];
  rL1.addElement(LED6);

  AssignLEDprofile LED7;
  LED7.arrayInitalise(8);
  LED7.index = 6;
  LED7.duration = 10; 
  LED7.red = new int[100, 0, 0, 0, 0, 0, 150, 150];
  LED7.green = new int[100, 150, 100, 0, 0, 0, 0, 0];
  LED7.blue = new int[0, 0, 100, 150, 100, 0, 0, 0];
  rL1.addElement(LED7);

  AssignLEDprofile LED8;
  LED8.arrayInitalise(8);
  LED8.index = 7;
  LED8.duration = 10; 
  LED8.red = new int[150, 100, 0, 0, 0, 0, 0, 150];
  LED8.green = new int[0, 100, 150, 100, 0, 0, 0, 0];
  LED8.blue = new int[0, 0, 0, 100, 150, 100, 0, 0];
  rL1.addElement(LED8);
  
  inactiveRingLEDs[0] = rL1;
  inringProfileCount++;

  pixel.begin();
  pixel.setBrightness(64);
  pixel.show();

  dht.begin();
  pinMode(pirPin, INPUT);
}

int thirtysec = 150;
float t;


void loop() {
  pixel.clear();
  WiFiClient client = server.available();

  if (client){
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()&& currentTime - previousTime <= timeoutTime){
      currentTime = millis();
      if (client.available()) {
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {  
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            int posth = 0;
            int postw = 0;
            posth = findProfile(13);
            postw = findProfile(12);

            if (header.indexOf("GET /web/on") >= 0) {
              Serial.println("Web on");
              webActive = true;
              while (profileCount > 0){
                activeToInactive(profileCount);
              }
            }
            else if (header.indexOf("GET /web/off") >= 0) {
              Serial.println("Web off");
              webActive = false;
            }
            else if (header.indexOf("GET /pin13/Time") >= 0){
              inactiveLEDs[posth].trigger = "Time";
            }
            else if (header.indexOf("GET /pin13/Temp") >= 0){
              inactiveLEDs[posth].trigger = "Temp";
            }
            else if (header.indexOf("GET /pin13/Const") >= 0){
              inactiveLEDs[posth].trigger = "Const";
            }
            else if (header.indexOf("GET /pin13/Sensor") >= 0){
              inactiveLEDs[posth].trigger = "Sensor";
            }
            else if (header.indexOf("GET /pin12/Time") >= 0){
              inactiveLEDs[postw].trigger = "Time";
            }
            else if (header.indexOf("GET /pin12/Temp") >= 0){
              inactiveLEDs[postw].trigger = "Temp";
            }
            else if (header.indexOf("GET /pin12/Const") >= 0){
              inactiveLEDs[postw].trigger = "Const";
            }
            else if (header.indexOf("GET /pin12/Sensor") >= 0){
              inactiveLEDs[postw].trigger = "Sensor";
            }
            else if (header.indexOf("GET /timeSet13") >= 0){
              inactiveLEDs[posth].shr = charToInt(header.charAt(22),header.charAt(23));
              inactiveLEDs[posth].smin = charToInt(header.charAt(32),header.charAt(33));
              inactiveLEDs[posth].ehr = charToInt(header.charAt(42),header.charAt(43));
              inactiveLEDs[posth].emin = charToInt(header.charAt(52),header.charAt(53));
            }
            else if (header.indexOf("GET /timeSet12") >= 0){
              inactiveLEDs[postw].shr = charToInt(header.charAt(22),header.charAt(23));
              inactiveLEDs[postw].smin = charToInt(header.charAt(32),header.charAt(33));
              inactiveLEDs[postw].ehr = charToInt(header.charAt(42),header.charAt(43));
              inactiveLEDs[postw].emin = charToInt(header.charAt(52),header.charAt(53));
            }
            else if (header.indexOf("GET /tempSet13") >= 0){
              inactiveLEDs[posth].degC = charToInt(header.charAt(22),header.charAt(23));
            }
            else if (header.indexOf("GET /tempSet12") >= 0){
              inactiveLEDs[posth].degC = charToInt(header.charAt(22),header.charAt(23));
            }
            else if (header.indexOf("GET /pin13/temptrig/above") >= 0){
              inactiveLEDs[posth].below = false;
            }
            else if (header.indexOf("GET /pin13/temptrig/below") >= 0){
              inactiveLEDs[posth].below = true;
            }
            else if (header.indexOf("GET /pin12/temptrig/above") >= 0){
              inactiveLEDs[postw].below = false;
            }
            else if (header.indexOf("GET /pin12/temptrig/below") >= 0){
              inactiveLEDs[postw].below = true;
            }
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: left;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");

            client.println("<body><h1>ESP32 Web Server</h1>");
            if (webActive){
              client.println("<p> Web active </p>");
              client.println("<p><a href=\"/web/off\"><button class=\"button button2\">OFF</button></a></p>");
              client.println("<h2>Pin 13</h2>");
              client.println("<p>Set Trigger:</p>");
              client.println("<a href=\"/pin13/Time\">Time</a>");
              client.println("<a href=\"/pin13/Temp\">Tempurature</a>");
              client.println("<a href=\"/pin13/Const\">Constant</a>");
              client.println("<a href=\"/pin13/Sensor\">Sensor</a>");
              if (inactiveLEDs[posth].trigger == "Time"){
                client.println("<form action=\"/timeSet13\">");
                client.println("Start Hour: <input type=\"text\" name=\"input1\">");
                client.println("Start Minute: <input type=\"text\" name=\"input2\">");
                client.println("End Hour: <input type=\"text\" name=\"input3\">");
                client.println("End Minute: <input type=\"text\" name=\"input4\">");
                client.println("<input type=\"submit\" value=\"Submit\">");
                client.println("</form><br>");
              }
              else if (inactiveLEDs[posth].trigger == "Temp"){
                client.println("<form action=\"/tempSet13\">");
                client.println("Start Hour: <input type=\"text\" name=\"input1\">");
                client.println("<input type=\"submit\" value=\"Submit\">");
                client.println("</form><br>");
                client.println("<a href=\"/pin13/temptrig/above\">Above</a>");
                client.println("<a href=\"/pin13/temptrig/below\">Below</a>");                   
              }

              client.println("<h2>Pin 12</h2>");
              client.println("<p>Set Trigger:</p>");
              client.println("<a href=\"/pin12/Time\">Time</a>");
              client.println("<a href=\"/pin12/Temp\">Tempurature</a>");
              client.println("<a href=\"/pin12/Const\">Constant</a>");
              client.println("<a href=\"/pin12/Sensor\">Sensor</a>");
              if (inactiveLEDs[postw].trigger == "Time"){
                client.println("<form action=\"/timeSet12\">");
                client.println("Start Hour: <input type=\"text\" name=\"input5\">");
                client.println("Start Minute: <input type=\"text\" name=\"input6\">");
                client.println("End Hour: <input type=\"text\" name=\"input7\">");
                client.println("End Minute: <input type=\"text\" name=\"input8\">");
                client.println("<input type=\"submit\" value=\"Submit\">");
                client.println("</form><br>");
              }
              else if (inactiveLEDs[postw].trigger == "Temp"){
                client.println("<form action=\"/tempSet12\">");
                client.println("Start Hour: <input type=\"text\" name=\"input5\">");
                client.println("<input type=\"submit\" value=\"Submit\">");
                client.println("</form><br>");
                client.println("<a href=\"/pin12/temptrig/above\">Above</a>");
                client.println("<a href=\"/pin12/temptrig/below\">Below</a>");                   
              }
            }
            else{
              client.println("<p> Web offline </p>");
              client.println("<p><a href=\"/web/on\"><button class=\"button\">ON</button></a></p>");
              
            }
            client.println("</body></html>");
            client.println();
            // Break out of the while loop
            break;
          }else {
            currentLine = "";
          }
        }else if (c != '\r') {  // if you got anything else but a carriage return character,
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
  if (webActive == false){
    //so this takes the size of the array and displays the the x number in the birghtness array of every element before coming out of the while loop
    int pos = 0;
    int len = 2;
    int i = 0;
    while (i < profileCount){
      //current = activeLEDs[pos];
      ledcWrite(activeLEDs[i].LEDchannel, activeLEDs[i].brightnes[activeLEDs[i].lengPos]);
      if (activeLEDs[i].count >= activeLEDs[i].duration){
        activeLEDs[i].lengPos++;
        if (activeLEDs[i].lengPos >= activeLEDs[i].leng){
          activeLEDs[i].lengPos = 0;
          if (activeLEDs[i].trigger == "Const"){;}
          else{
            ledcWrite(activeLEDs[i].LEDchannel, 0);
            activeToInactive(i);
          }
        }
        activeLEDs[i].count = 0;
      }
      activeLEDs[i].count++;
       
      i++;
    }
    
    int j = 0;
    while (j < ringProfileCount){
      i = 0;
      while ( i < activeRingLEDs[j].pos){
        AssignLEDprofile temp;
        if (activeRingLEDs[j].LEDs[i].active){
          if (activeRingLEDs[j].LEDs[i].duration <= activeRingLEDs[j].LEDs[i].count){
            pixel.setPixelColor(activeRingLEDs[j].LEDs[i].index, pixel.Color(activeRingLEDs[j].LEDs[i].red[activeRingLEDs[j].LEDs[i].lengPos], activeRingLEDs[j].LEDs[i].green[activeRingLEDs[j].LEDs[i].lengPos], activeRingLEDs[j].LEDs[i].blue[activeRingLEDs[j].LEDs[i].lengPos]));
            activeRingLEDs[j].LEDs[i].lengPos++;
            if (activeRingLEDs[j].LEDs[i].lengPos > activeRingLEDs[j].LEDs[i].leng){
              activeRingLEDs[j].LEDs[i].lengPos = 0;
              if(activeRingLEDs[j].trigger == "Const"){;}
              else if (i+1 == activeRingLEDs[j].pos){
                pixel.clear();
                colouractivetoin(j);
              }
            }
          }
          else{activeRingLEDs[0].LEDs[i].count++;}
        }
        pixel.show();
        i++;
      }j++;
    }
    
    if (thirtysec < 150){
      thirtysec++;
    }
    else{
      t = dht.readTemperature();
      thirtysec=0;
      }
    i = 0;
    while (i < inprofileCount){
      if (inactiveLEDs[i].trigger == "Time"){
        if (comparetime(inactiveLEDs[i].shr, inactiveLEDs[i].smin, inactiveLEDs[i].ehr, inactiveLEDs[i].emin)){
          inactiveToActive(i);
          Serial.println("Time met");
        }
        else{
          i++;
        }
      }
      else if (inactiveLEDs[i].trigger == "Temp" && t != 100){
        if(inactiveLEDs[i].below == true && t < inactiveLEDs[i].degC){
          t = 100;
          inactiveToActive(i);
          Serial.println("Temp met");
        }
        else if (inactiveLEDs[i].below == false && t > inactiveLEDs[i].degC){
          t = 100;
          inactiveToActive(i);
          Serial.println("Temp met");
        }
        else{i++;}
      }
      else if (inactiveLEDs[i].trigger == "Const"){
        inactiveToActive(i);
      }
      else if (inactiveLEDs[i].trigger == "Sensor"){
        int val = 0;
        val = digitalRead(pirPin);
        if (val == HIGH){
          inactiveToActive(i);
          Serial.println("Motion");
        }
        else {i++;}
      }
      else{i++;}
    }
    
    i = 0;
    while (i < inringProfileCount){
      if (inactiveRingLEDs[i].trigger == "Time"){
        ;
      }
      else if (inactiveRingLEDs[i].trigger == "Temp"){
        if(inactiveRingLEDs[i].below == true && t < inactiveRingLEDs[i].degC){
          t = 100;
          colourintoactive(i);
          Serial.println("Temp met");
        }
        else if (inactiveRingLEDs[i].below == false && t > inactiveRingLEDs[i].degC){
          t = 100;
          colourintoactive(i);
          Serial.println("Temp met");
        }
        else{i++;}
      }
      else if (inactiveRingLEDs[i].trigger == "Const"){
        colourintoactive(i);
      }
      else if (inactiveRingLEDs[i].trigger == "Sensor"){
        int val = 0;
        val = digitalRead(pirPin);
        if (val == HIGH){
          colourintoactive(i);
          Serial.println("Motion");
        }
        else {i++;}
      }
      else{i++;}
    }
    delay(200);
  }
}

bool comparetime(int shr, int smin, int ehr, int emin){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return false;
  }
  int timeh = timeinfo.tm_hour;
  int timem = timeinfo.tm_min;
  if (shr < timeh || (shr == timeh && smin < timem)){
    if (ehr > timeh || (ehr == timeh && emin > timem)){
      return true;
    }
  }
  return false;
}
