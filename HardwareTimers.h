/* interface for managing the hardware timers for ATmega168, or ATmega328 */

#include <iom328p.h>

//Wave Generation Modes, see ref 1 pg 108
//Use the WAVE_GENERATION_MODE macro to create a WGM value using parameters
#define _WGM_NORMAL_MAX_IMMEDIATE_MAX 0
#define _WGM_PCPWM_MAX_TOP_BOTTOM 1
#define _WGM_CTC_OCRA_IMMEDIATE_MAX 2
#define _WGM_FASTPWM_MAX_BOTTOM_MAX 3
#define _WGM_PCPWM_OCRA_TOP_BOTTOM 5
#define _WGM_FASTPWM_OCRA_BOTTOM_TOP 7
#define WAVE_GENERATION_MODE(Mode, Top, UpdateOfOCRxAt, TOVFlagSetOn) _WGM_##Mode##_##Top##_##UpdateOfOCRxAt##_##TOVFlagSetOn

#define _COM_EFFECT_0 _WGM_NOPWM
#define _COM_EFFECT_2 _WGM_NOPWM
#define _COM_EFFECT_3 _WGM_FASTPWM
#define _COM_EFFECT_7 _WGM_FASTPWM_OCRA
#define _COM_EFFECT_1 _WGM_PCPWM
#define _COM_EFFECT_5 _WGM_PCPWM_OCRA

#define _COMA_WGM_NOPWM_NORMAL 0
#define _COMA_WGM_NOPWM_TOGGLE_OC0A 1
#define _COMA_WGM_NOPWM_CLEAR_OC0A 2
#define _COMA_WGM_NOPWM_SET_OC0A 3

#define _COMA_WGM_FASTPWM_NORMAL 0
#define _COMA_WGM_FASTPWM_OCRA_TOGGLE_OC0A 1
#define _COMA_WGM_FASTPWM_MATCH_CLEAR_BOTTOM_SET_OC0A 2
#define _COMA_WGM_FASTPWM_MATCH_SET_BOTTOM_CLEAR_OC0A 3

#define _COMA_WGM_PCPWM_NORMAL 0
#define _COMA_WGM_PCPWM_OCRA_TOGGLE_OC0A 1
#define _COMA_WGM_PCPWM_UP_CLEAR_DOWN_SET_OC0A 2
#define _COMA_WGM_PCPWM_UP_SET_DOWN_CLEAR_OC0A 3

// Mode values are NORMAL, TOGGLE_OC0A, MATCH_SET_BOTTOM_CLEAR_OC0A, MATCH_CLEAR_BOTTOM_SET_OC0A, UP_SET_DOWN_CLEAR_OC0A, or DOWN_SET_UP_CLEAR_OC0A
// Which modes are valid depends on the specified WGM_MODE, and is enforced by macro evaluation, for example:
//
// #define WGM WaveGeneartionMode(FASTPWM, OCRA, BOTTOM, TOP)
// COMPARE_MATCH_OUTPUT_A_MODE(WGM, TOGGLE_OC0A) //macro evaluation results in the value 1 from ref 1 pg. 106 table 14-3
// COMPARE_MATCH_OUTPUT_A_MODE(WGM, SET_OC0A) //results in error since _COMA_WGM_FASTPWM_OCRA_SET_OC0A is undefined
#define COMPARE_MATCH_OUTPUT_A_MODE_4(COM_EFFECT, Mode) _COMA##COM_EFFECT##_##Mode
#define COMPARE_MATCH_OUTPUT_A_MODE_3(COM_EFFECT, Mode) COMPARE_MATCH_OUTPUT_A_MODE_4(COM_EFFECT, Mode) //force preprocessor prescan
#define COMPARE_MATCH_OUTPUT_A_MODE_2(PRESCANED_WGM_MODE, Mode) COMPARE_MATCH_OUTPUT_A_MODE_3(_COM_EFFECT_##PRESCANED_WGM_MODE, Mode)
#define COMPARE_MATCH_OUTPUT_A_MODE(WGM_MODE, Mode) COMPARE_MATCH_OUTPUT_A_MODE_2(WGM_MODE, Mode)

#define _COMB_WGM_NOPWM_NORMAL 0
#define _COMB_WGM_NOPWM_TOGGLE_OC0B 1
#define _COMB_WGM_NOPWM_CLEAR_OC0B 2
#define _COMB_WGM_NOPWM_SET_OC0B 3

#define _COMB_WGM_FASTPWM_NORMAL 0
#define _COMB_WGM_FASTPWM_MATCH_CLEAR_BOTTOM_SET_OC0B 2
#define _COMB_WGM_FASTPWM_MATCH_SET_BOTTOM_CLEAR_OC0B 3

#define _COMB_WGM_PCPWM_NORMAL 0
#define _COMB_WGM_PCPWM_UP_CLEAR_DOWN_SET_OC0B 2
#define _COMB_WGM_PCPWM_UP_SET_DOWN_CLEAR_OC0B 3

#define COMPARE_MATCH_OUTPUT_B_MODE_4(COM_EFFECT, Mode) _COMB##COM_EFFECT##_##Mode
#define COMPARE_MATCH_OUTPUT_B_MODE_3(COM_EFFECT, Mode) COMPARE_MATCH_OUTPUT_B_MODE_4(COM_EFFECT, Mode) //force preprocessor prescan
#define COMPARE_MATCH_OUTPUT_B_MODE_2(PRESCANED_WGM_MODE, Mode) COMPARE_MATCH_OUTPUT_B_MODE_3(_COM_EFFECT_##PRESCANED_WGM_MODE, Mode)
#define COMPARE_MATCH_OUTPUT_B_MODE(WGM_MODE, Mode) COMPARE_MATCH_OUTPUT_B_MODE_2(WGM_MODE, Mode)

struct _8_bit_timer_counter_control_register_A {
    unsigned char waveform_generation_mode_00_01 : 2;
    unsigned char reserved : 2;
    unsigned char compare_match_output_B_mode : 2;
    unsigned char compare_match_output_A_mode : 2;
};

struct _8_bit_timer_counter_control_register_B {
    unsigned char clock_select : 3;
    unsigned char waveform_generation_mode_02 : 1;
    unsigned char reserved : 2;
    unsigned char force_output_compare_A : 1;
    unsigned char force_output_compare_B : 1;
};

void get_8_bit_timer_counter_control_registers(int timer, _8_bit_timer_counter_control_register_A* A, _8_bit_timer_counter_control_register_B* B) {
    switch (timer) {
    case 0:
        *(char *)A = TCCR0A;
        *(char *)B = TCCR0B;
    case 2:
        *(char *)A = TCCR2A;
        *(char *)B = TCCR2B;
    default:
        exit(EXIT_FAILURE);
    }
}

void set_8_bit_timer_counter_control_registers(int timer, _8_bit_timer_counter_control_register_A A, _8_bit_timer_counter_control_register_B B) {
    switch (timer) {
    case 0:
        TCCR0A = *(char *)&A;
        TCCR0B = *(char *)&B;
    case 2:
        TCCR2A = *(char *)&A;
        TCCR2B = *(char *)&B;
    default:
        exit(EXIT_FAILURE);
    }
}

unsigned char get_timer_counter_wave_generation_mode(int timer) {
    switch (timer) {
    case 0:
    case 2:
        _8_bit_timer_counter_control_register_A A8;
        _8_bit_timer_counter_control_register_B B8;
        get_8_bit_timer_counter_control_registers(timer, &A8, &B8);
        return B8.waveform_generation_mode_02 << 2 | A8.waveform_generation_mode_00_01;
    case 1:
        return 0; //todo
    default:
        exit(EXIT_FAILURE);
    }
}

void set_timer_counter_wave_generation_mode(int timer, unsigned char mode) {
    if (mode > 7) {
        exit(EXIT_FAILURE);
    }
    switch (timer) {
    case 0:
    case 2:
        _8_bit_timer_counter_control_register_A A8;
        _8_bit_timer_counter_control_register_B B8;
        get_8_bit_timer_counter_control_registers(timer, &A8, &B8);
        A8.waveform_generation_mode_00_01 = mode & 3;
        B8.waveform_generation_mode_02 = mode >> 2;
        set_8_bit_timer_counter_control_registers(timer, A8, B8);
    case 1:
        return 0; //todo
    default:
        exit(EXIT_FAILURE);
    }
}

void
void force_output_compare(int) {

}

void stop_timer_0() {

}

/* References
1: http://www.atmel.com/Images/doc8161.pdf
*/
