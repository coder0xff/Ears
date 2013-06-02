#include <Arduino.h>

//configuration
#define AUTO_MODE_PIN A0
#define MANUAL_DOWN_PIN 2
#define MANUAL_TWITCH_PIN 4
#define LEFT_MOTOR_POS_PWM_PIN 3
#define LEFT_MOTOR_NEG_PWM_PIN 5
#define LEFT_MOTOR_ENABLE_PIN 7
#define LEFT_MOTOR_STOP_PIN 12
#define RIGHT_MOTOR_POS_PWM_PIN 6
#define RIGHT_MOTOR_NEG_PWM_PIN 9
#define RIGHT_MOTOR_ENABLE_PIN 8
#define RIGHT_MOTOR_STOP_PIN 13
#define ORIENT_TIMEOUT_MILLS 5000

//#define PWM_FREQUENCY 100

//constants
enum ear {
    LEFT_EAR = 0,
    RIGHT_EAR = 1
};

void set_ear_velocity(ear e, char velocity) {
    unsigned char const dutyCycleMin = 200;
    unsigned char const negDir = (unsigned char)velocity >> 7;
    unsigned char interpolationParameter = negDir ? (unsigned char)((short)velocity * 255 / -128) : (unsigned char)((short)velocity * 255 / 127);
    unsigned char const enable = interpolationParameter == 0;
    unsigned char dutyCycle = enable ? 0 : dutyCycleMin + (short)(255 - dutyCycleMin) * interpolationParameter / 255;
    unsigned char enable_pin, pos_pwm_pin, neg_pwm_pin;
    switch (e) {
        case LEFT_EAR:
            enable_pin = LEFT_MOTOR_ENABLE_PIN;
            pos_pwm_pin = LEFT_MOTOR_POS_PWM_PIN;
            neg_pwm_pin = LEFT_MOTOR_NEG_PWM_PIN;
        case RIGHT_EAR:
            enable_pin = RIGHT_MOTOR_ENABLE_PIN;
            pos_pwm_pin = RIGHT_MOTOR_POS_PWM_PIN;
            neg_pwm_pin = RIGHT_MOTOR_NEG_PWM_PIN;
            break;
    }
    digitalWrite(enable_pin, false);
    analogWrite(pos_pwm_pin, negDir ? 0 : dutyCycle);
    analogWrite(neg_pwm_pin, negDir ? dutyCycle : 0);
    digitalWrite(enable_pin, enable);
}

unsigned char stop_switch_is_activated(ear e) {
    switch (e) {
    case LEFT_EAR:
        return digitalRead(LEFT_MOTOR_STOP_PIN);
    case RIGHT_EAR:
        return digitalRead(RIGHT_MOTOR_STOP_PIN);
    }
}

enum orient_step {
    ORIENT_START = 0,
    ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION = 1,
    ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION = 2,
    ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION = 3,
    ORIENT_FAILED = 4,
    ORIENT_SUCCEEDED = 5
};

char const * ear_names[] = {"Left", "Right"};

char const * orient_step_names[] = {
    "ORIENT_START",
    "ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION",
    "ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION",
    "ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION",
    "ORIENT_FAILED",
    "ORIENT_SUCCEEDED"
};

orient_step orient_step_action(ear e, orient_step step, unsigned long *timeout_time) {
    switch (step) {
    case ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION:
        if (millis() > *timeout_time) {
            set_ear_velocity(e, 0);
            Serial.print(ear_names[e]); Serial.print(": ");
            Serial.println("ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION timed out. ORIENT_FAILED");
            return ORIENT_FAILED;
        }
        set_ear_velocity(e, 127);
        if (!stop_switch_is_activated(e)) {
            *timeout_time = millis() + ORIENT_TIMEOUT_MILLS;
            return ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION;
        } else {
            return ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION;
        }
    case ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION:
        if (millis() > *timeout_time) {
            Serial.print(ear_names[e]); Serial.print(": ");
            Serial.println("ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION timed out. ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION");
            *timeout_time = millis() + ORIENT_TIMEOUT_MILLS;
            return ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION;
        }
        set_ear_velocity(e, -128);
        if (stop_switch_is_activated(e)) {
            set_ear_velocity(e, 0);
            return ORIENT_SUCCEEDED;
        } else {
            return ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION;
        }
    case ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION:
        if (millis() > *timeout_time) {
            set_ear_velocity(e, 0);
            Serial.print(ear_names[e]); Serial.print(": ");
            Serial.println("ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION timed out. ORIENT_FAILED");
            return ORIENT_FAILED;
        }
        set_ear_velocity(e, 127);
        if (stop_switch_is_activated(e)) {
            *timeout_time = millis() + ORIENT_TIMEOUT_MILLS;
            return ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION;
        } else {
            return ORIENT_SEARCH_FOR_STOP_IN_POSITIVE_DIRECTION;
        }
    case ORIENT_SUCCEEDED:
        set_ear_velocity(e, 0);
        return ORIENT_SUCCEEDED;
    default:
        set_ear_velocity(e, 0);
        return ORIENT_FAILED;
    }
}

//Move both ears to the position where they are upright
//If quick is false, it will find the down-most position where the switch is active - for startup
//If quick is true, it will finish immediately if it starts with the switch active - for end of sequences
void orient_ears(unsigned char quick) {
    orient_step left_orient_step = (!quick && digitalRead(LEFT_MOTOR_STOP_PIN)) ? ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION : ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION;
    Serial.print("left: "); Serial.println(orient_step_names[left_orient_step]);
    unsigned long left_timeout_time = millis() + ORIENT_TIMEOUT_MILLS;
    orient_step right_orient_step = (!quick && digitalRead(RIGHT_MOTOR_STOP_PIN)) ? ORIENT_SEARCH_FOR_NOT_STOP_IN_POSITIVE_DIRECTION : ORIENT_SEARCH_FOR_STOP_IN_NEGATIVE_DIRECTION;
    Serial.print("right: "); Serial.println(orient_step_names[right_orient_step]);
    unsigned long right_timeout_time = millis() + ORIENT_TIMEOUT_MILLS;
    while ((left_orient_step != ORIENT_SUCCEEDED && left_orient_step != ORIENT_FAILED) || (right_orient_step != ORIENT_SUCCEEDED && right_orient_step != ORIENT_FAILED)) {
        left_orient_step = orient_step_action(LEFT_EAR, left_orient_step, &left_timeout_time);
        right_orient_step = orient_step_action(RIGHT_EAR, right_orient_step, &right_timeout_time);
    }
    if (left_orient_step == ORIENT_FAILED || right_orient_step == ORIENT_FAILED) {
        Serial.println("Orientation failed");
        pinMode(13, OUTPUT);
        while(1) {
            digitalWrite(13, HIGH);
            delay(1000);
            digitalWrite(13, LOW);
            delay(1000);
        }
    }
}

long next_auto_move_millis = -1;

void ears_down_sequence() {
    next_auto_move_millis = -1;
    set_ear_velocity(LEFT_EAR, 127);
    set_ear_velocity(RIGHT_EAR, 127);
    delay(500);
    set_ear_velocity(LEFT_EAR, 0);
    set_ear_velocity(RIGHT_EAR, 0);
    delay(2000);
    orient_ears(true);
}

char left_ear_will_twitch_next = 0;

void ear_twitch_sequence() {
    next_auto_move_millis = -1;
    ear const e = left_ear_will_twitch_next ? LEFT_EAR : RIGHT_EAR;
    left_ear_will_twitch_next = ~left_ear_will_twitch_next;
    set_ear_velocity(e, 127);
    delay(250);
    orient_ears(true);
}

void setup()
{
    Serial.begin(9600);
    pinMode(AUTO_MODE_PIN, INPUT);
    pinMode(MANUAL_DOWN_PIN, INPUT);
    pinMode(MANUAL_TWITCH_PIN, INPUT);
    pinMode(LEFT_MOTOR_ENABLE_PIN, OUTPUT);
    pinMode(LEFT_MOTOR_STOP_PIN, INPUT);
    pinMode(RIGHT_MOTOR_ENABLE_PIN, OUTPUT);
    pinMode(RIGHT_MOTOR_STOP_PIN, OUTPUT);
    Serial.println("Pins setup, orienting ears.");
    orient_ears(false);
}

void loop()
{
    if (digitalRead(MANUAL_DOWN_PIN)) ears_down_sequence();
    if (digitalRead(MANUAL_TWITCH_PIN)) ear_twitch_sequence();
    if (digitalRead(AUTO_MODE_PIN)) {
        if (next_auto_move_millis == -1) {
            next_auto_move_millis = millis() + random(30000, 60000);
        }
        if (next_auto_move_millis < millis()) {
            switch(random(0,2)) {
            case 0:
                ears_down_sequence();
            case 1:
                ear_twitch_sequence();
            }
        }
    } else {
        next_auto_move_millis = -1;
    }
}
