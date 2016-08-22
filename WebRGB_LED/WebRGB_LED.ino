// RGB LED Lighting System
// Dan Nixon's work (2013) -> www.dan-nixon.com
// Modifications made by Joel Haker 2016

//Includes
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <SD.h>

//Pins
static const int RED_PIN = 44;
static const int GREEN_PIN = 45;
static const int BLUE_PIN = 46;
static const int PWR_PIN = 22;

//Colour Component Enum
static const int R = 0;
static const int G = 1;
static const int B = 2;

//Transition Mode Enum
static const int NO_EXEC = 0;
static const int INSTANT = 1;
static const int FADE_DIRECT = 2;

//Default Values
static const int DEFAULT_TRANSITION = INSTANT;
static const int DEFAULT_TIME = 5;

static int OFF[3] = {0, 0, 0};
static int FULL_WHITE[3] = {100, 100, 100};

//Variables
int currentColour[3];
int lastWebColour[3];
int lastUsedTransition;
int lastUsedTime;

// Switch Variables
int switchVal;
int lastSwitchVal;

//Rainbow variables
int colorVal[3] = { 0, 0, 0 };
int rnbwVal = -1;
int lastRnbwVal = 0;
int colorWheelBreakVal = -1;

// Color arrays
int red[3]  = { 100, 0, 0 };
int green[3]  = { 0, 100, 0 };
int blue[3]    = { 0, 0, 100 };
int cyan[3]  = { 0, 100, 100 };
int magenta[3]   = { 100, 0, 100 };
int yellow[3] = { 100, 100, 0 };

// SD Card variable
File webFile;

// Webserver
static uint8_t MAC[] = {0x90, 0xA2, 0xDA, 0x0E, 0x03, 0xF7};
static uint8_t IP[] = {192, 168, 1, 50};
WebServer webserver("", 80);

//HTML for web front end UI
P(failedToReadrgbindx) = "<h1>Web file Boo!!!</h1>";

//Serves web front end to control light from a web browser
void webUI(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  lastRnbwVal == 0; // make Rainbow Loop stop
  server.httpSuccess();
  if (type == WebServer::GET) {
    webFile = SD.open("rgbindx.htm");// open web page file
    if (webFile) {
      while (webFile.available()) {
        server.write(webFile.read()); // send web page to client
      }
      webFile.close();
    }
    else {
      server.printP(failedToReadrgbindx);
    }
  }
}

//Provides back end to control lights from front end and other apps
void webBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  lastRnbwVal == 0; // make Rainbow Loop stop
  char name[16];
  char value[16];
  server.httpSuccess();

  if (type == WebServer::POST) {
    int colour[3];
    int ttime = DEFAULT_TIME;
    int ttransition = DEFAULT_TRANSITION;
    while (server.readPOSTparam(name, 16, value, 16)) {
      Serial.print(name);
      Serial.print(" = ");
      Serial.println(value);

      if (strcmp(name, "r") == 0) colour[R] = atoi(value);
      if (strcmp(name, "g") == 0) colour[G] = atoi(value);
      if (strcmp(name, "b") == 0) colour[B] = atoi(value);
      if (strcmp(name, "trans") == 0) ttransition = atoi(value);
      if (strcmp(name, "time") == 0) ttime = atoi(value);
    }
    Serial.println("-----------------------------------------");

    if (ttransition != NO_EXEC) {
      lastWebColour[R] = colour[R];
      lastWebColour[G] = colour[G];
      lastWebColour[B] = colour[B];
      lightChange(colour, ttransition, ttime);
    }
  }
  if ((type == WebServer::POST) || (type == WebServer::GET)) {
    server.println("<?xml version='1.0'?>");
    server.println("<xml>");

    server.println("<currentColour>");
    server.print("<r>");
    server.print(currentColour[R]);
    server.println("</r>");
    server.print("<g>");
    server.print(currentColour[G]);
    server.println("</g>");
    server.print("<b>");
    server.print(currentColour[B]);
    server.println("</b>");
    server.println("</currentColour>");

    server.print("<lastTime>");
    server.print(lastUsedTime);
    server.println("</lastTime>");

    server.print("<pwr>");
    server.print(lastSwitchVal);
    server.println("</pwr>");

    server.print("<rnbw>");
    server.print(lastRnbwVal);
    server.println("</rnbw>");

    server.print("<exec>");
    if (lastUsedTransition == NO_EXEC) {
      server.print("0");
    }
    else if (lastUsedTransition == INSTANT) {
      server.print("1");
    }
    else {
      server.print("2");
    }
    server.println("</exec>");

    server.println("</xml>");
  }
}

//Switches power off and on to the LED Lights
void switchBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  lastRnbwVal == 0; // make Rainbow Loop stop
  char name[16];
  char value[16];
  server.httpSuccess();

  if (type == WebServer::POST) {
    while (server.readPOSTparam(name, 16, value, 16)) {
      Serial.print(name);
      Serial.print(" = ");
      Serial.println(value);
      if (strcmp(name, "pwr") == 0) switchVal = atoi(value);
    }
    Serial.println("-----------------------------------------");

    if (lastSwitchVal != switchVal) {
      if (switchVal == 1) {
        digitalWrite(PWR_PIN, HIGH);
        lastSwitchVal = switchVal;
      }
      else if (switchVal == 0) {
        digitalWrite(PWR_PIN, LOW);
        lastSwitchVal = switchVal;
      }
      else {
        Serial.println("///////////////");
        Serial.println("Something went wrong with comparing switch values.");
        Serial.println("///////////////");
      }
    }
    else if (lastSwitchVal == switchVal) {
      Serial.println("------------");
      Serial.println("Switch values are the same.");
      Serial.println("----------");
    }
  }

  if ((type == WebServer::POST) || (type == WebServer::GET)) {
    server.println("<?xml version='1.0'?>");
    server.println("<xml>");

    server.println("<currentColour>");
    server.print("<r>");
    server.print(currentColour[R]);
    server.println("</r>");
    server.print("<g>");
    server.print(currentColour[G]);
    server.println("</g>");
    server.print("<b>");
    server.print(currentColour[B]);
    server.println("</b>");
    server.println("</currentColour>");

    server.print("<lastTime>");
    server.print(lastUsedTime);
    server.println("</lastTime>");

    server.print("<pwr>");
    server.print(lastSwitchVal);
    server.println("</pwr>");

    server.print("<rnbw>");
    server.print(lastRnbwVal);
    server.println("</rnbw>");

    server.print("<exec>");
    if (lastUsedTransition == NO_EXEC) {
      server.print("0");
    }
    else if (lastUsedTransition == INSTANT) {
      server.print("1");
    }
    else {
      server.print("2");
    }
    server.println("</exec>");

    server.println("</xml>");
  }
}

//Provides back end to control lights from front end and other apps
void rnbwBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  char name[16];
  char value[16];
  server.httpSuccess();
  int cwtime = 5; // Default Time...

  if (type == WebServer::POST) {
    while (server.readPOSTparam(name, 16, value, 16)) {
      Serial.print(name);
      Serial.print(" = ");
      Serial.println(value);

      if (strcmp(name, "rnbw") == 0) rnbwVal = atoi(value);
      if (strcmp(name, "rtimetype") == 0) int rTimeType = atoi(value);
      if (strcmp(name, "rtime") == 0) cwtime = atoi(value);
    }
    Serial.println("-----------------------------------------");
  }
  lastUsedTime = cwtime;
  if (lastRnbwVal != rnbwVal) {
    if (rnbwVal == 1) {
      lastRnbwVal = 1;
      colorWheelBreakVal = 1;
    }
    else if (rnbwVal == 0) {
      lastRnbwVal = 0;
    }
    else {
      Serial.println("///////////////");
      Serial.println("Something went wrong with comparing RnBw values.");
      Serial.println("///////////////");
    }
  }
  else if (lastSwitchVal == switchVal) {
    Serial.println("------------");
    Serial.println("RnBw values are the same.");
    Serial.println("----------");
  }

  if ((type == WebServer::POST) || (type == WebServer::GET)) {
    server.println("<?xml version='1.0'?>");
    server.println("<xml>");

    server.println("<currentColour>");
    server.print("<r>");
    server.print(currentColour[R]);
    server.println("</r>");
    server.print("<g>");
    server.print(currentColour[G]);
    server.println("</g>");
    server.print("<b>");
    server.print(currentColour[B]);
    server.println("</b>");
    server.println("</currentColour>");

    server.print("<lastTime>");
    server.print(lastUsedTime);
    server.println("</lastTime>");

    server.print("<pwr>");
    server.print(lastSwitchVal);
    server.println("</pwr>");

    server.print("<rnbw>");
    server.print(lastRnbwVal);
    server.println("</rnbw>");

    server.print("<exec>");
    if (lastUsedTransition == NO_EXEC) {
      server.print("0");
    }
    else if (lastUsedTransition == INSTANT) {
      server.print("1");
    }
    else {
      server.print("2");
    }
    server.println("</exec>");

    server.println("</xml>");
  }
}// End of rnbwBackend command

// **Start of rnbwBackend() Methods**
int calculateStep(int prevValue, int endValue) {
  int step = endValue - prevValue;  //  What's the overall gap?
  if (step) {             //  If its non-zero,
    step = 1020 / step;       //  divide by 1020
  }
  return step;
} // End of calculateStep()

int calculateVal(int step, int val, int i) {
  if ((step) && i % step == 0) {  //  If step is non-zero and its time to change a value,
    if (step > 0) {       //  increment the value if step is positive...
      val += 1;
    }
    else if (step < 0) {    //  ...or decrement it if step is negative
      val -= 1;
    }
  }
  // Defensive driving: make sure val stays in the range 0-255
  if (val > 255) {
    val = 255;
  }
  else if (val < 0) {
    val = 0;
  }
  return val;
}

void crossFade(int color[3], int wait) {

  //Variables to check incoming connection
  char buff[64];
  int len = 64;

  // Convert to 0-255
  int rVal = (color[R] * 255) / 100;
  int gVal = (color[G] * 255) / 100;
  int bVal = (color[B] * 255) / 100;

  int stepR = calculateStep(currentColour[R], rVal);
  int stepG = calculateStep(currentColour[G], gVal);
  int stepB = calculateStep(currentColour[B], bVal);

  for (int i = 0; i <= 1020; i++) {
    webserver.processConnection(buff, &len);

    colorVal[0] = calculateVal(stepR, colorVal[R], i);
    colorVal[1] = calculateVal(stepG, colorVal[G], i);
    colorVal[2] = calculateVal(stepB, colorVal[B], i);

    setRGB(colorVal);

    delay(wait); // Pause for 'wait' milliseconds before resuming the loop
  }
  // Update current values for next loop
  currentColour[R] = colorVal[R];
  currentColour[G] = colorVal[G];
  currentColour[B] = colorVal[B];
  delay(5); // Pause for optional 'wait' milliseconds before resuming the loop
}

// **End of rnbwBackend() Methods/Functions**

// **Main Methods/Functions**
// Used to handle a change in lighting
void lightChange(int colour[], int ttransition, int ttime) {
  if (ttransition == NO_EXEC) return;
  int oldColour[3];
  oldColour[R] = currentColour[R];
  oldColour[G] = currentColour[G];
  oldColour[B] = currentColour[B];
  switch (ttransition) {
    case INSTANT:
      setRGB(colour);
      break;
    case FADE_DIRECT:
      fade(oldColour, colour, ttime);
      break;
  }
  lastUsedTime = ttime;
  lastUsedTransition = ttransition;
} // End of lightChange()

// Controls a smooth lighting fade
void fade(int startColour[], int endColour[], int fadeTime) {
  for (int t = 0; t < fadeTime; t++) {
    int colour[3];
    colour[R] = map(t, 0, fadeTime, startColour[R], endColour[R]);
    colour[G] = map(t, 0, fadeTime, startColour[G], endColour[G]);
    colour[B] = map(t, 0, fadeTime, startColour[B], endColour[B]);
    setRGB(colour);
    delay(1);
  }
  setRGB(endColour);
} // End of fade()

// Sets an RGB colour
void setRGB(int colour[3]) {
  if (colour[R] < 0) colour[R] = 0;
  if (colour[R] > 255) colour[R] = 255;
  if (colour[G] < 0) colour[G] = 0;
  if (colour[G] > 255) colour[G] = 255;
  if (colour[B] < 0) colour[B] = 0;
  if (colour[B] > 255) colour[B] = 255;

  analogWrite(RED_PIN, colour[R]);
  analogWrite(GREEN_PIN, colour[G]);
  analogWrite(BLUE_PIN, colour[B]);

  currentColour[R] = colour[R];
  currentColour[G] = colour[G];
  currentColour[B] = colour[B];
}// End of setRGB()

// **End of Main Methods/Functions**

void setup() {
  Serial.begin(115200);

  //SD card init()
  Serial.println("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("ERROR - SD card initialization failed!");
    return;    // init failed
  }
  Serial.println("SUCCESS - SD card initialized.");

  // check for rgbindx.htm file
  if (!SD.exists("rgbindx.htm")) {
    Serial.println("ERROR - Can't find rgbindx.htm file!");
    return;  // can't find rgbindx.htm file
  }
  Serial.println("SUCCESS - Found rgbindx.htm file.");

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);

  switchVal = 1;
  lastSwitchVal = 1;

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  Ethernet.begin(MAC, IP);

  webserver.setDefaultCommand(&webUI);
  webserver.addCommand("index", &webUI);
  webserver.addCommand("service", &webBackend);
  webserver.addCommand("switch", &switchBackend);
  webserver.addCommand("clrWheel", &rnbwBackend);
  webserver.begin();

  lightChange(FULL_WHITE, lastUsedTransition, lastUsedTime);
}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  // Rainbow Magic!!!!
  if (colorWheelBreakVal == 1) {
    int rtime = lastUsedTime;
    if (rtime > 50) {
      rtime = 50;
    }
    else if (rtime < 1) {
      rtime = 1;
    }
    lightChange(OFF, FADE_DIRECT, rtime);
    while (lastRnbwVal == 1) {
      crossFade(red, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
      crossFade(yellow, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
      crossFade(green, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
      crossFade(cyan, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
      crossFade(blue, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
      crossFade(magenta, rtime);
      if (lastRnbwVal == 0) {
        colorWheelBreakVal = 0;
        rtime = 1;
        break;
      }
    }
    lastUsedTime = rtime;
  }
}

