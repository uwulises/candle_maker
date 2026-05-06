#ifndef KLARA_H
#define KLARA_H
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

void init_klara();
void moveto_mm(long step_mm);

void disable_stepper(int motor);
void enable_stepper(int motor);
void homeStepper_R();
void homeStepper_H();
bool check_clearence();
void rotate_next_index();

#endif