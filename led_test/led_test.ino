// color swirl! connect an RGB LED to the PWM pins as indicated
// in the #defines
// public domain, enjoy!
 
#define RPIN 3
#define GPIN 3
#define BPIN 5
#define PWRPIN 30
 
#define FADESPEED 5     // make this higher to slow down
 
void setup() {
  pinMode(RPIN, OUTPUT);
  pinMode(GPIN, OUTPUT);
  pinMode(BPIN, OUTPUT);

}
 
 
void loop() {
  int r, g, b;

  pinMode(PWRPIN, OUTPUT);
  digitalWrite(PWRPIN, HIGH);
 
  // fade from blue to violet
  for (r = 0; r < 256; r++) { 
    analogWrite(RPIN, r);
    delay(FADESPEED);
  } 
  // fade from violet to red
  for (b = 255; b > 0; b--) { 
    analogWrite(BPIN, b);
    delay(FADESPEED);
  } 
  // fade from red to yellow
  for (g = 0; g < 256; g++) { 
    analogWrite(GPIN, g);
    delay(FADESPEED);
  } 
  // fade from yellow to green
  for (r = 255; r > 0; r--) { 
    analogWrite(RPIN, r);
    delay(FADESPEED);
  } 
  // fade from green to teal
  for (b = 0; b < 256; b++) { 
    analogWrite(BPIN, b);
    delay(FADESPEED);
  } 
  // fade from teal to blue
  for (g = 255; g > 0; g--) { 
    analogWrite(GPIN, g);
    delay(FADESPEED);
  }

  pinMode(PWRPIN, OUTPUT);
  digitalWrite(PWRPIN, LOW); 
}