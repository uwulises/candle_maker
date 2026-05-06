#include "klara.h"
Encoder encoder_R(A_PULSE, B_PULSE);
AccelStepper stepper_H(AccelStepper::DRIVER, STEP_H_PIN, DIR_H_PIN);
AccelStepper stepper_R(AccelStepper::DRIVER, STEP_R_PIN, DIR_R_PIN);
AccelStepper stepper_P(AccelStepper::DRIVER, STEP_P_PIN, DIR_P_PIN);


double actual_pos_mm = 0.0;
int actual_index = 0;
double actual_pos_index = 0.0;
float pitch_mm = 20.0;
float microstep = 0.0025;
int check_home = 0;
int check_home_R = 0;
long step_index = 0;
int rotation_speed = 256;
int H_speed = 256;
float pulse_to_mm = pitch_mm * microstep;
float pulse_to_deg = 360.0 / (400 * 10 * 1.3); // considering gear and pulley reductor
float encoder_pulse_to_deg = 360.0 / (0.5*1000.0);
long encoder_pos_deg = 0;
float R_position_error = 0.0;
bool permit = false;


void init_klara()
{
  pinMode(LIMIT_H_PIN1, INPUT_PULLUP); // Active LOW
  pinMode(LIMIT_H_PIN2, INPUT_PULLUP); // Active LOW
  pinMode(LIMIT_INDEX, INPUT_PULLUP); // Active LOW
  stepper_H.setEnablePin(EN_H_PIN);
  stepper_H.setMaxSpeed(1600);
  stepper_H.setAcceleration(800);
  stepper_H.setCurrentPosition(0);
  stepper_R.setEnablePin(EN_R_PIN);
  stepper_R.setMaxSpeed(800);
  stepper_R.setAcceleration(800);
  stepper_R.setCurrentPosition(0);
  encoder_R.write(0); // Reset encoder count to 0
  stepper_P.setMaxSpeed(800);
  stepper_P.setAcceleration(800);
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
  if (motor
    == 1)
    {
      stepper_R.disableOutputs();
    }  
  else if (motor
    == 0)
    {
      stepper_H.disableOutputs();
    }
}

void enable_stepper(int motor)
{
  if (motor
    == 1)
    {
      stepper_R.enableOutputs();
    }
  else if (motor
    == 0)
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
  permit = false;
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
  step_index += 1309; // Move to the next index
  stepper_R.moveTo(step_index);
  while (stepper_R.distanceToGo() != 0)
  {
    stepper_R.run();
  }
  actual_pos_index = stepper_R.currentPosition() * pulse_to_deg;
  //compare encoder position with stepper position
  encoder_pos_deg = encoder_R.read() * encoder_pulse_to_deg;
  R_position_error = abs(actual_pos_index - encoder_pos_deg);
  // if (R_position_error > 5.0) // If the error is greater than 5 degrees, perform a correction
  // {
  //   stepper_R.setSpeed(rotation_speed / 2); // Reduce speed for correction
  //   if (R_position_error > 0)
  //   {
  //     stepper_R.moveTo(stepper_R.currentPosition() - (R_position_error / pulse_to_deg) * 400); // Move back to correct position
  //   }
  //   else
  //   {
  //     stepper_R.moveTo(stepper_R.currentPosition() + (abs(R_position_error) / pulse_to_deg) * 400); // Move forward to correct position
  //   }
  //   while (stepper_R.distanceToGo() != 0)
  //   {
  //     stepper_R.run();
  //   }
  //   actual_pos_index = stepper_R.currentPosition() * pulse_to_deg; // Update actual position after correction
  // }



}