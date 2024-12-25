#include <Dezibot.h>

#define RED_LEFT 0
#define GREEN_LEFT 1

class RobotConfig {
private:
  uint16_t baseSpeed = 3900;
  uint16_t maxSpeed = 8192;
  uint16_t rotateSpeed = 2500;
  double redScale = 1.01;
  double greenScale = 1.01;
  int rotateDuration = 1500;
  int moveStraightDuration = 1500;
  double whiteTolerance = 0.7;

public:
  uint16_t getBaseSpeed() const {
    return baseSpeed;
  }
  uint16_t getMaxSpeed() const {
    return maxSpeed;
  }
  uint16_t getRotateSpeed() const {
    return rotateSpeed;
  }
  double getRedScale() const {
    return redScale;
  }
  double getGreenScale() const {
    return greenScale;
  }
  int getRotateDuration() const {
    return rotateDuration;
  }
  int getMoveStraightDuration() const {
    return moveStraightDuration;
  }
  double getWhiteTolerance() const {
    return whiteTolerance;
  }

  void setBaseSpeed(uint16_t speed) {
    baseSpeed = speed;
  }
  void setMaxSpeed(uint16_t speed) {
    maxSpeed = speed;
  }
  void setRotateSpeed(uint16_t speed) {
    rotateSpeed = speed;
  }
  void setRedScale(double scale) {
    redScale = scale;
  }
  void setGreenScale(double scale) {
    greenScale = scale;
  }
  void setRotateDuration(int duration) {
    rotateDuration = duration;
  }
  void setMoveStraightDuration(int duration) {
    moveStraightDuration = duration;
  }
  void setWhiteTolerance(double tolerance) {
    whiteTolerance = tolerance;
  }
};

RobotConfig config;
Dezibot dezibot = Dezibot();

double CALIBRATED_RED, CALIBRATED_GREEN, CALIBRATED_BLUE;
bool invertComparison = false;
bool isOnWhite = false;

int currentColorMode = RED_LEFT;

void setColorMode(int colorMode) {
  currentColorMode = colorMode;
}

int getColorMode() {
  return currentColorMode;
}

void toggleColorMode() {
  currentColorMode = (currentColorMode == RED_LEFT) ? GREEN_LEFT : RED_LEFT;
}

void calibrateWhite() {
  dezibot.display.println("Weiß Kal.");
  delay(3000);
  getColorPercentages(CALIBRATED_RED, CALIBRATED_GREEN, CALIBRATED_BLUE);
  dezibot.display.println("Auf Feld");
  delay(3000);
}

void deadEndRotation() {
  colorSwitch();

  dezibot.motion.left.setSpeed(0);
  dezibot.motion.right.setSpeed(config.getMaxSpeed());

  delay(3000);

  dezibot.motion.left.setSpeed(config.getMaxSpeed());
  dezibot.motion.right.setSpeed(0);

  delay(5000);
  double initialRed, initialGreen, initialBlue;
  getColorPercentages(initialRed, initialGreen, initialBlue);

  double newRed, newGreen, newBlue;
  bool stillOnWhite = true;

  while (stillOnWhite) {
    getColorPercentages(newRed, newGreen, newBlue);
    dezibot.display.println(isColorCloseTo(initialRed, newRed, config.getWhiteTolerance()));
    dezibot.display.println(isColorCloseTo(initialGreen, newGreen, config.getWhiteTolerance()));
    stillOnWhite = isColorCloseTo(initialRed, newRed, config.getWhiteTolerance()) && isColorCloseTo(initialGreen, newGreen, config.getWhiteTolerance());
  }

  dezibot.motion.left.setSpeed(config.getBaseSpeed());
  dezibot.motion.right.setSpeed(config.getBaseSpeed());

  delay(1000);
  stopMotors();
}

void moveStraight() {
  dezibot.motion.left.setSpeed(config.getBaseSpeed());
  dezibot.motion.right.setSpeed(config.getBaseSpeed());
  colorSwitch();
  delay(config.getMoveStraightDuration());
  stopMotors();
}

void rotateLeft() {
  dezibot.motion.left.setSpeed(config.getRotateSpeed());
  dezibot.motion.right.setSpeed(config.getBaseSpeed());
  colorSwitch();
  delay(config.getRotateDuration());
  stopMotors();
}

void rotateRight() {
  dezibot.motion.left.setSpeed(config.getBaseSpeed());
  dezibot.motion.right.setSpeed(config.getRotateSpeed());
  colorSwitch();
  delay(config.getRotateDuration());
  stopMotors();
}

void stopMotors() {
  dezibot.motion.left.setSpeed(0);
  dezibot.motion.right.setSpeed(0);
}

void colorSwitch() {
  invertComparison = !invertComparison;
}

void getColorPercentages(double &percentageRed, double &percentageGreen, double &percentageBlue) {
  uint16_t red = dezibot.colorDetection.getColorValue(VEML_RED) * config.getRedScale();
  uint16_t green = dezibot.colorDetection.getColorValue(VEML_GREEN) * config.getGreenScale();
  uint16_t blue = dezibot.colorDetection.getColorValue(VEML_BLUE);
  double sumColor = red + green + blue;

  if (sumColor == 0) {
    percentageRed = percentageGreen = percentageBlue = 0;
    return;
  }

  percentageRed = (red / sumColor) * 100.0;
  percentageGreen = (green / sumColor) * 100.0;
  percentageBlue = (blue / sumColor) * 100.0;
}

bool compareColors(double percentageRed, double percentageGreen) {
  return currentColorMode == RED_LEFT ? (percentageRed > percentageGreen) : (percentageGreen > percentageRed);
}

bool isColorCloseTo(double initialValue, double newValue, double tolerance) {
  return abs(initialValue - newValue) <= tolerance;
}

void controlMotors(bool isFirstGreater) {
  Serial.println(isFirstGreater);
  if (isFirstGreater) {
    dezibot.motion.left.setSpeed(config.getBaseSpeed());
    dezibot.motion.right.setSpeed(config.getRotateSpeed());
  } else {
    dezibot.motion.left.setSpeed(config.getRotateSpeed());
    dezibot.motion.right.setSpeed(config.getBaseSpeed());
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  dezibot.begin();
  dezibot.multiColorLight.setLed(BOTTOM, 100, 100, 100);
  setColorMode(GREEN_LEFT);
  // calibrateWhite();

  //rotateLeft();
}

void loop() {
  double percentageRed, percentageGreen, percentageBlue;
  getColorPercentages(percentageRed, percentageGreen, percentageBlue);

  // isOnWhite = isColorCloseTo(CALIBRATED_RED, percentageRed, config.getWhiteTolerance()) && isColorCloseTo(CALIBRATED_GREEN, percentageGreen, config.getWhiteTolerance());
  // if (isOnWhite) {
  //   deadEndRotation();
  // }

  bool isFirstGreater = compareColors(percentageRed, percentageGreen);
  Serial.println(isFirstGreater);
  controlMotors(isFirstGreater);
}
