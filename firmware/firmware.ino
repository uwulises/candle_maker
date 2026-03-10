#include <AccelStepper.h>
#include <ArduinoJson.h>
#include <Encoder.h>
// Height control
#define EN_H_PIN 6
#define STEP_H_PIN 4
#define DIR_H_PIN 5
#define LIMIT_H_PIN1 30
#define LIMIT_H_PIN2 31
// Rotational control
#define EN_R_PIN 10
#define STEP_R_PIN 8
#define DIR_R_PIN 9
#define LIMIT_INDEX 32

// Pump control
#define EN_R_PIN 16
#define STEP_P_PIN 14 
#define DIR_P_PIN 15

// Enconder Rotational
#define A_PULSE 2
#define B_PULSE 3

// BOTONERA
#define HOME_H 22
#define HOME_R 23

// TEMPERATURA CERA
#define TEMP_PIN A0

Encoder encoder_R(A_PULSE, B_PULSE);

// Create a JSON object
StaticJsonDocument<200> doc;

AccelStepper stepper_H(AccelStepper::DRIVER, STEP_H_PIN, DIR_H_PIN);
AccelStepper stepper_R(AccelStepper::DRIVER, STEP_R_PIN, DIR_R_PIN);

String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete
double actual_pos_mm = 0.0;
int actual_index = 0;
double actual_pos_index = 0.0;
float pitch_mm = 20.0;
float microstep = 0.0025;
int check_home = 0;
int check_home_R = 0;
int step_index = 0;
int rotation_speed = 256;
int H_speed = 256;
int motor_id = 0; // 0 for height, 1 for rotation
float pulse_to_mm = pitch_mm * microstep;
float pulse_to_deg = 360.0 / (400 * 10 * 1.3); // considering gear and pulley reductor
float encoder_pulse_to_deg = 360.0 / (0.5*1000.0);
long encoder_pos_deg = 0;
float R_position_error = 0.0;
// newPosition = myEnc.read();

void serialEvent()
{
  while (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n')
    {
      stringComplete = true;
    }
  }
}

// MOVE TO MM POSITION

void moveto_mm(long step_mm)
{
  stepper_H.setSpeed(H_speed);
  step_mm = step_mm / pulse_to_mm;
  stepper_H.moveTo(step_mm);
  while (stepper_H.distanceToGo() != 0)
  {
    stepper_H.run();
  }
  actual_pos_mm = stepper_H.currentPosition() * pulse_to_mm;
}

void disable_stepper(int motor)
{
  if motor
    == 1
    {
      stepper_R.disableOutputs();
    }
  else if motor
    == 0
    {
      stepper_H.disableOutputs();
    }
}

void enable_stepper(int motor)
{
  if motor
    == 1
    {
      stepper_R.enableOutputs();
    }
  else if motor
    == 0
    {
      stepper_H.enableOutputs();
    }
}

// ---------- HOMING FUNCTION ----------
void homeStepper_H()
{
  // Serial.println("Starting homing sequence...");
  stepper_H.setSpeed(-256);
  check_home = 0;
  while (check_home < 200)
  {
    stepper_H.runSpeed();
    if (digitalRead(LIMIT_H_PIN1) == LOW)
    {
      check_home++;
    }
  }
  check_home = 0;
  stepper_H.setCurrentPosition(0);
  actual_pos_mm = stepper_H.currentPosition() * pulse_to_mm;
}

void homeStepper_R()
{
  stepper_R.setSpeed(-256);
  check_home_R = 0;
  while (check_home_R < 200)
  {
    stepper_R.runSpeed();
    if (digitalRead(LIMIT_INDEX) == LOW)
    {
      check_home_R++;
    }
  }
  stepper_R.setCurrentPosition(0);
  encoder_R.write(0); // Reset encoder count to 0
  actual_pos_index = stepper_R.currentPosition() * pulse_to_deg;
}

// -------------------------------------

bool check_clearence()
{
  // Function to update index_cp based on current position
  bool permit = false;
  // check for reasonable height
  if (actual_pos_mm < 10.0)
  {
    permit = true;
  }
  else
  {
    homeStepper_H();
    permit = true;
  }
  return permit;
}

void rotate_next_index()
{
  // Function to rotate to the next index position
  stepper_R.setSpeed(rotation_speed);
  step_index += 400; // Move to the next index (assuming 400 steps per index)
  stepper_R.moveTo(step_index);
  while (stepper_R.distanceToGo() != 0)
  {
    stepper_R.run();
  }
  actual_pos_index = stepper_R.currentPosition() * pulse_to_deg;
  //compare encoder position with stepper position
  encoder_pos_deg = encoder_R.read() * encoder_pulse_to_deg;
  R_position_error = abs(actual_pos_index - encoder_pos_deg);
  if (R_position_error > 5.0) // If the error is greater than 5 degrees, perform a correction
  {
    stepper_R.setSpeed(rotation_speed / 2); // Reduce speed for correction
    if (R_position_error > 0)
    {
      stepper_R.moveTo(stepper_R.currentPosition() - (R_position_error / pulse_to_deg) * 400); // Move back to correct position
    }
    else
    {
      stepper_R.moveTo(stepper_R.currentPosition() + (abs(R_position_error) / pulse_to_deg) * 400); // Move forward to correct position
    }
    while (stepper_R.distanceToGo() != 0)
    {
      stepper_R.run();
    }
    actual_pos_index = stepper_R.currentPosition() * pulse_to_deg; // Update actual position after correction
  }



}

void setup()
{
  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  pinMode(LIMIT_H_PIN1, INPUT_PULLUP); // Active LOW
  pinMode(LIMIT_H_PIN2, INPUT_PULLUP); // Active LOW
  stepper_H.setEnablePin(EN_H_PIN);
  stepper_H.setMaxSpeed(1600);
  stepper_H.setAcceleration(800);
  stepper_H.setCurrentPosition(0);
  stepper_R.setEnablePin(EN_R_PIN);
  stepper_R.setMaxSpeed(800);
  stepper_R.setAcceleration(800);
  stepper_R.setCurrentPosition(0);
  encoder_R.write(0); // Reset encoder count to 0
}

void loop()
{

  // print the string when a newline arrives:
  if (stringComplete)
  {
    // call homing
    if (inputString.substring(0, 6) == "Homing")
    {
      homeStepper_H();
      if (check_clearence())
      {
        homeStepper_R();
      }
    }
    if (inputString.substring(0, 9) == "Moveto_mm")
    {
      long t_mm = inputString.substring(9, 12).toInt();
      moveto_mm(t_mm);
      // Serial.println(t_mm);
      // Serial.println(stepper_H.currentPosition());
    }

    if (inputString.substring(0, 10) == "Next_index")
    {
      if (check_clearence())
      {
        rotate_next_index();
        // Serial.println(stepper_R.currentPosition());
      }
    }

    if (inputString.substring(0, 3) == "DIS")
    {
      motor_id = inputString.substring(3, 4).toInt();
      disable_stepper(motor);
    }
    if (inputString.substring(0, 3) == "ENA")
    {
      motor_id = inputString.substring(3, 4).toInt();
      enable_stepper(motor);
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
    // Serialize JSON to Serial
    // serializeJson(doc, Serial);
    // Serial.println();
  }
}