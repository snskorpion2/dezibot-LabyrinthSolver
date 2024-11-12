#include <Dezibot.h>
#include <Wire.h>
#include <veml6040.h>

Dezibot dezibot = Dezibot();
VEML6040 rgbwSensor;

const double MAX_COLOR_VALUE = 65536;

const double TARGET_RED = 31.5;
const double TARGET_GREEN = 35;
const double TARGET_BLUE = 33.14;
const double THRESHOLD = 1.3;

void setup() {
  Serial.begin(9600);
  Serial.println("Started");
  dezibot.begin();
  Serial.println("Initialised");

  if (!rgbwSensor.begin()) {
    Serial.println("ERROR: couldn't detect the sensor");
    while (1) {}
  }
  
  rgbwSensor.setConfiguration(VEML6040_IT_320MS + VEML6040_AF_AUTO + VEML6040_SD_ENABLE);
  dezibot.multiColorLight.setLed(BOTTOM, 100, 100, 100);
  delay(1000);
}

void loop() {
  dezibot.motion.move();
  dezibot.motion.rotateClockwise(5000); //TODO: hier weitermachen MOVE muss ausgeführt werden beim rotate sonst kein rotate und Delay muss mind. Summe der Zeit des rotates sein sonst abbruch
  delay(5000);

  double red = getRawColorValue(VEML_RED);
  double green = getRawColorValue(VEML_GREEN);
  double blue = getRawColorValue(VEML_BLUE);
  double sumColor = red + green + blue;

  double percentageRed = (red / sumColor) * 100;
  double percentageGreen = (green / sumColor) * 100;
  double percentageBlue = (blue / sumColor) * 100;

  printValue(percentageRed, "R");
  printValue(percentageGreen, "G");
  printValue(percentageBlue, "B");
  dezibot.display.println(isBlackLine(percentageRed, percentageGreen, percentageBlue));
    
  dezibot.motion.move();

  dezibot.motion.rotateAntiClockwise(1000);
  delay(200);
  if (isBlackLine(percentageRed, percentageGreen, percentageBlue)) {
    dezibot.motion.move();
  } 
  else 
  {
    dezibot.motion.rotateClockwise(1000);
    delay(200);
  }

 }
  delay(1000);
  Serial.println("");
  dezibot.display.clear();
}


bool isBlackLine(double red, double green, double blue) {
  return (abs(red - TARGET_RED) <= THRESHOLD) &&
         (abs(green - TARGET_GREEN) <= THRESHOLD) &&
         (abs(blue - TARGET_BLUE) <= THRESHOLD);
}

double getRawColorValue(color color) {
  switch (color) {
    case VEML_RED:
      return rgbwSensor.getRed();
    case VEML_GREEN:
      return rgbwSensor.getGreen();
    case VEML_BLUE:
      return rgbwSensor.getBlue();
    case VEML_WHITE:
      return rgbwSensor.getWhite();
  }
}

void printValue(double colorValue, const char* prefix) {
  dezibot.display.print(prefix);
  dezibot.display.print(" ");
  dezibot.display.println(String(colorValue, 2));
  
  Serial.print(prefix);
  Serial.print(" ");
  Serial.println(colorValue);
}
