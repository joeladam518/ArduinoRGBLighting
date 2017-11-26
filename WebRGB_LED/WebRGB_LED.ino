// RGB LED Lighting System
// Dan Nixon's work (2013) -> www.dan-nixon.com
// Modifications made by Joel Haker 2016

// Includes
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <EEPROM.h>
#include <SD.h>

// Pins
static const int RED_PIN   = 24;
static const int GREEN_PIN = 25;
static const int BLUE_PIN  = 26;
static const int PWR_PIN   = 27;

// Color Component Enum
static const int R = 0;
static const int G = 1;
static const int B = 2;

// Transition Mode Enum
static const int NO_EXEC = 0;
static const int INSTANT = 1;
static const int FADE_DIRECT = 2;

// Default Values
static const int DEFAULT_TRANSITION = INSTANT;
static const int DEFAULT_TIME = 5;

static int OFF[3] = {0, 0, 0};
static int FULL_WHITE[3] = {100, 100, 100};

// Variables
int currentcolor[3];
int lastWebcolor[3];
int lastUsedTransition;
int lastUsedTime;

// Switch Variables
int switchVal;
int lastSwitchVal;

// Rainbow variables
int colorVal[3] = { 0, 0, 0 };
int rnbwVal = -1;
int lastRnbwVal = 0;
int colorWheelBreakVal = -1;

// Color arrays
int red[3] = { 100, 0, 0 };
int green[3] = { 0, 100, 0 };
int blue[3] = { 0, 0, 100 };
int cyan[3] = { 0, 100, 100 };
int magenta[3] = { 100, 0, 100 };
int yellow[3] = { 100, 100, 0 };

// SD Card variable
File webFile;

// Webserver
static uint8_t MAC[] = {0x90, 0xA2, 0xDA, 0x0E, 0x03, 0xF7};
static uint8_t IP[] = {192, 168, 1, 50};
WebServer webserver("", 80);

// HTML for web front end UI
P(failedToReadrgbindx) = "<h1>Web File Boo!!!</h1>";

// Serves web front end to control light from a web browser
void webUI(WebServer &server, WebServer::ConnectionType type, char *, bool) 
{
    server.httpSuccess();
    lastRnbwVal = 0; // make Rainbow Loop stop

    if (type == WebServer::GET) {
        // Open web page file
        webFile = SD.open("rgbindx.htm");
        
        if (webFile) {
            while (webFile.available()) {
                // Send web page to client
                server.write(webFile.read());
            }
            webFile.close();
        } else {
            server.printP(failedToReadrgbindx);
        }
    }
}

// Provides back end to control lights from front end and other apps
void webBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) 
{
    server.httpSuccess();
    char name[16];
    char value[16];
    lastRnbwVal = 0; // make Rainbow Loop stop

    if (type == WebServer::POST) {
        int color[3];
        int ttime = DEFAULT_TIME;
        int ttransition = DEFAULT_TRANSITION;
        
        while (server.readPOSTparam(name, 16, value, 16)) {
            Serial.print(name);
            Serial.print(" = ");
            Serial.println(value);

            if (strcmp(name, "r") == 0) { color[R] = atoi(value); }
            if (strcmp(name, "g") == 0) { color[G] = atoi(value); }
            if (strcmp(name, "b") == 0) { color[B] = atoi(value); }
            if (strcmp(name, "trans") == 0) { ttransition = atoi(value); }
            if (strcmp(name, "time") == 0) { ttime = atoi(value); }
        }

        Serial.println("-----------------------------------------");

        if (ttransition != NO_EXEC) {
            lastWebcolor[R] = color[R];
            lastWebcolor[G] = color[G];
            lastWebcolor[B] = color[B];
            lightChange(color, ttransition, ttime);
        }
    }

    if ((type == WebServer::POST) || (type == WebServer::GET)) {
        server.println("<?xml version='1.0'?>");
        server.println("<xml>");

        server.println("<currentcolor>");
        server.print("<r>");
        server.print(currentcolor[R]);
        server.println("</r>");
        server.print("<g>");
        server.print(currentcolor[G]);
        server.println("</g>");
        server.print("<b>");
        server.print(currentcolor[B]);
        server.println("</b>");
        server.println("</currentcolor>");

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
        } else if (lastUsedTransition == INSTANT) {
            server.print("1");
        } else {
            server.print("2");
        }
        server.println("</exec>");
        
        server.println("</xml>");
    }
}

// Switches power off and on to the LED Lights
void switchBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) 
{
    char name[16];
    char value[16];
    server.httpSuccess();
    lastRnbwVal = 0; // make Rainbow Loop stop

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
            } else if (switchVal == 0) {
                digitalWrite(PWR_PIN, LOW);
                lastSwitchVal = switchVal;
            } else {
                Serial.println("///////////////");
                Serial.println("Something went wrong with comparing switch values.");
                Serial.println("///////////////");
            }
        }  else if (lastSwitchVal == switchVal) {
            Serial.println("----------");
            Serial.println("Switch values are the same.");
            Serial.println("----------");
        }
    }

    if ((type == WebServer::POST) || (type == WebServer::GET)) {
        server.println("<?xml version='1.0'?>");
        server.println("<xml>");

        server.println("<currentcolor>");
        server.print("<r>");
        server.print(currentcolor[R]);
        server.println("</r>");
        server.print("<g>");
        server.print(currentcolor[G]);
        server.println("</g>");
        server.print("<b>");
        server.print(currentcolor[B]);
        server.println("</b>");
        server.println("</currentcolor>");

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
        } else if (lastUsedTransition == INSTANT) {
            server.print("1");
        } else {
            server.print("2");
        }
        server.println("</exec>");
        
        server.println("</xml>");
    }
}

// Color wheel back end
void rnbwBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) 
{
    server.httpSuccess();
    
    char name[16];
    char value[16];
    int cwtime = 5; // Default Time...

    if (type == WebServer::POST) {
        while (server.readPOSTparam(name, 16, value, 16)) {
            Serial.print(name);
            Serial.print(" = ");
            Serial.println(value);

            if (strcmp(name, "rnbw") == 0) { rnbwVal = atoi(value); }
            if (strcmp(name, "rtimetype") == 0) { int rTimeType = atoi(value); }
            if (strcmp(name, "rtime") == 0) { cwtime = atoi(value); }
        }
        
        Serial.println("-----------------------------------------");
    }

    lastUsedTime = cwtime;
    if (lastRnbwVal != rnbwVal) {
        if (rnbwVal == 1) {
            lastRnbwVal = 1;
            colorWheelBreakVal = 1;
        } else if (rnbwVal == 0) {
            lastRnbwVal = 0;
        } else {
            Serial.println("///////////////");
            Serial.println("Something went wrong with comparing RnBw values.");
            Serial.println("///////////////");
        }
    } else if (lastSwitchVal == switchVal) {
        Serial.println("------------");
        Serial.println("RnBw values are the same.");
        Serial.println("----------");
    }

    if ((type == WebServer::POST) || (type == WebServer::GET)) {
        server.println("<?xml version='1.0'?>");
        server.println("<xml>");

        server.println("<currentcolor>");
        server.print("<r>");
        server.print(currentcolor[R]);
        server.println("</r>");
        server.print("<g>");
        server.print(currentcolor[G]);
        server.println("</g>");
        server.print("<b>");
        server.print(currentcolor[B]);
        server.println("</b>");
        server.println("</currentcolor>");

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
        } else if (lastUsedTransition == INSTANT) {
            server.print("1");
        } else {
            server.print("2");
        }
        server.println("</exec>");
        
        server.println("</xml>");
    }
}

// **Start of rnbwBackend() Methods**

int calculateStep(int prevValue, int endValue) 
{
    // What's the overall gap?
    int step = endValue - prevValue;
    // If its non-zero, divide by 1020
    if (step) {             
        step = 1020 / step;
    }
    
    return step;
}

int calculateVal(int step, int val, int i) 
{
    // If step is non-zero and its time to change a value,
    if ((step) && i % step == 0) {  
        if (step > 0) { // Increment the value if step is positive...
            val += 1;
        } else if (step < 0) { // Or decrement it if step is negative
            val -= 1;
        }
    }

    // Defensive driving: make sure val stays in the range 0-255
    if (val > 255) {
        val = 255;
    } else if (val < 0) {
        val = 0;
    }

    return val;
}

void crossFade(int color[3], int wait) 
{
    // Variables to check incoming connection
    char buff[64];
    int len = 64;

    // Convert to 0-255
    int rVal = (color[R] * 255) / 100;
    int gVal = (color[G] * 255) / 100;
    int bVal = (color[B] * 255) / 100;

    int stepR = calculateStep(currentcolor[R], rVal);
    int stepG = calculateStep(currentcolor[G], gVal);
    int stepB = calculateStep(currentcolor[B], bVal);

    for (int i = 0; i <= 1020; i++) {
        webserver.processConnection(buff, &len);

        colorVal[0] = calculateVal(stepR, colorVal[R], i);
        colorVal[1] = calculateVal(stepG, colorVal[G], i);
        colorVal[2] = calculateVal(stepB, colorVal[B], i);

        setRGB(colorVal);

        delay(wait);
    }
    
    // Update current values for next loop
    currentcolor[R] = colorVal[R];
    currentcolor[G] = colorVal[G];
    currentcolor[B] = colorVal[B];
    delay(1);
} 

// **End of rnbwBackend() Methods/Functions**
// **Start of Main Methods/Functions**

// Used to handle a change in lighting
void lightChange(int color[], int ttransition, int ttime) 
{
    if (ttransition == NO_EXEC) { return; }
    int oldcolor[3];
    oldcolor[R] = currentcolor[R];
    oldcolor[G] = currentcolor[G];
    oldcolor[B] = currentcolor[B];

    switch (ttransition) {
        case INSTANT:
            setRGB(color);
            break;
        case FADE_DIRECT:
            fade(oldcolor, color, ttime);
            break;
    }

    lastUsedTime = ttime;
    lastUsedTransition = ttransition;
}

// Controls a smooth lighting fade
void fade(int startcolor[], int endcolor[], int fadeTime) 
{
    for (int t = 0; t < fadeTime; t++) {
        int color[3];
        color[R] = map(t, 0, fadeTime, startcolor[R], endcolor[R]);
        color[G] = map(t, 0, fadeTime, startcolor[G], endcolor[G]);
        color[B] = map(t, 0, fadeTime, startcolor[B], endcolor[B]);
        setRGB(color);
        delay(1);
    }

    setRGB(endcolor);
}

// Sets an RGB color
void setRGB(int color[3]) 
{
    if (color[R] < 0) { color[R] = 0; }
    if (color[R] > 255) { color[R] = 255; }
    if (color[G] < 0) { color[G] = 0; }
    if (color[G] > 255) { color[G] = 255; }
    if (color[B] < 0) { color[B] = 0; }
    if (color[B] > 255) { color[B] = 255; }

    analogWrite(RED_PIN, color[R]);
    analogWrite(GREEN_PIN, color[G]);
    analogWrite(BLUE_PIN, color[B]);

    currentcolor[R] = color[R];
     currentcolor[B] = color[B];
}

// **End of Main Methods/Functions**

void setup() {
    Serial.begin(115200);

    //SD card init()
    Serial.println("Initializing SD card...");

    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;
    }
    Serial.println("SUCCESS - SD card initialized.");

    // Check for rgbindx.htm file
    if (!SD.exists("rgbindx.htm")) {
        Serial.println("ERROR - Can't find rgbindx.htm file!");
        return;
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
