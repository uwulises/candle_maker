#include "klara.h"

// Create a JSON object
StaticJsonDocument<200> doc;
String inputString = "";     // a String to hold incoming data
bool stringComplete = false; // whether the string is complete
int motor_id = 0; // 0 for height, 1 for rotation
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

void setup()
{
  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the inputString:
  inputString.reserve(200);
  init_klara();
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
      disable_stepper(motor_id);
    }
    if (inputString.substring(0, 3) == "ENA")
    {
      motor_id = inputString.substring(3, 4).toInt();
      enable_stepper(motor_id);
    }
    // clear the string:
    inputString = "";
    stringComplete = false;
    // Serialize JSON to Serial
    // serializeJson(doc, Serial);
    // Serial.println();
  }
}