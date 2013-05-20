#include <Arduino.h>
#include "HardwareTimers.h"

#define AUTO_MODE_PIN 0 //RX
#define MANUAL_DOWN_PIN 2
#define MANUAL_TWITCH_PIN 4
#define LEFT_MOTOR_PWM_PIN 3
#define LEFT_MOTOR_DIR_PIN 7
#define RIGHT_MOTOR_PWM_PIN 5
#define RIGHT_MOTOR_DIR_PIN 8
#define PWM_FREQUENCY 100

ISR(TIMER1_COMPA_vect){

}

void setup()
{
    pinMode(AUTO_MODE_PIN, INPUT);
    pinMode(MANUAL_DOWN_PIN, INPUT);
    pinMode(MANUAL_TWITCH_PIN, INPUT);
    pinMode(LEFT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_DIR_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_PWM_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_DIR_PIN, OUTPUT);
    //(void)WaveGenerationMode(FASTPWM, OCRA, BOTTOM, TOP);
    #define WGM WAVE_GENERATION_MODE(FASTPWM, OCRA, BOTTOM, TOP)
    (void)COMPARE_MATCH_OUTPUT_A_MODE(WGM, TOGGLE_OC0A);
    WGM00
}

void loop()
{
	Serial.println("Hello world!");

	delay(1000);              // wait for a second
	digitalWrite(13, HIGH);   // set the LED on
	delay(1000);              // wait for a second
	digitalWrite(13, LOW);    // set the LED off
}
