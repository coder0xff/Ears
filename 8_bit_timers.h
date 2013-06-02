/* code for low-level management of the 8-bit hardware timers for ATmega168, or ATmega328 */

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

#define CLOCK_SELECT_8_BIT_NO_CLOCK 0
#define CLOCK_SELECT_8_BIT_INTERNAL_OVER_1 1
#define CLOCK_SELECT_8_BIT_INTERNAL_OVER_8 2
#define CLOCK_SELECT_8_BIT_INTERNAL_OVER_64 3
#define CLOCK_SELECT_8_BIT_INTERNAL_OVER_256 4
#define CLOCK_SELECT_8_BIT_INTERNAL_OVER_1024 5
#define CLOCK_SELECT_8_BIT_EXTERNAL_FALLING_EDGE 6
#define CLOCK_SELECT_8_BIT_EXTERNAL_RISING_EDGE 7

struct _8_bit_timer_counter_control_register_a {
    unsigned char waveform_generation_mode_00_01 : 2;
    unsigned char reserved : 2;
    unsigned char compare_match_output_b_mode : 2;
    unsigned char compare_match_output_a_mode : 2;
};

struct _8_bit_timer_counter_control_register_b {
    unsigned char clock_select : 3;
    unsigned char waveform_generation_mode_02 : 1;
    unsigned char reserved : 2;
    unsigned char force_output_compare_a : 1;
    unsigned char force_output_compare_b : 1;
};

inline void get_8_bit_timer_counter_control_registers(int timer, _8_bit_timer_counter_control_register_a* a, _8_bit_timer_counter_control_register_b* b) {
    switch (timer) {
    case 0:
        *(char *)a = TCCR0A;
        *(char *)b = TCCR0B;
    case 2:
        *(char *)a = TCCR2A;
        *(char *)b = TCCR2B;
    default:
        exit(EXIT_FAILURE);
    }
}

inline void set_8_bit_timer_counter_control_registers(int timer, _8_bit_timer_counter_control_register_A A, _8_bit_timer_counter_control_register_B B) {
    switch (timer) {
    case 0:
        TCCR0A = *(char *)&a;
        TCCR0B = *(char *)&b;
    case 2:
        TCCR2A = *(char *)&a;
        TCCR2B = *(char *)&b;
    default:
        exit(EXIT_FAILURE);
    }
}

//ref 1 pg 108 table 14-8
inline unsigned char get_8_bit_timer_counter_wave_generation_mode(int timer) {
    _8_bit_timer_counter_control_register_A A8;
    _8_bit_timer_counter_control_register_B B8;
    get_8_bit_timer_counter_control_registers(timer, &A8, &B8);
    return B8.waveform_generation_mode_02 << 2 | A8.waveform_generation_mode_00_01;
}

//ref 1 pg 108 table 14-8
inline void set_8_bit_timer_counter_wave_generation_mode(int timer, unsigned char mode) {
    _8_bit_timer_counter_control_register_A A8;
    _8_bit_timer_counter_control_register_B B8;
    get_8_bit_timer_counter_control_registers(timer, &A8, &B8);
    A8.waveform_generation_mode_00_01 = mode & 3;
    B8.waveform_generation_mode_02 = mode >> 2;
    set_8_bit_timer_counter_control_registers(timer, A8, B8);
}

//see ref 1 pg 106-108 tables 14-2 thru 14-7
inline unsigned char get_8_bit_timer_counter_output_compare_mode_a(int timer, unsigned char mode) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    return a.compare_match_output_a_mode;
}

inline unsigned char set_8_bit_timer_counter_output_compare_mode_a(int timer, unsigned char mode) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    a.compare_match_output_a_mode = mode;
    set_8_bit_timer_counter_control_registers(timer, &a, &b);
}

inline unsigned char get_8_bit_timer_counter_output_compare_mode_b(int timer, unsigned char mode) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    return a.compare_match_output_b_mode;
}

inline unsigned char set_8_bit_timer_counter_output_compare_mode_b(int timer, unsigned char mode) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    a.compare_match_output_b_mode = mode;
    set_8_bit_timer_counter_control_registers(timer, &a, &b);
}

//see ref 1 pg 109
inline void strobe_8_bit_timer_counter_force_output_compare_a(int timer) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    b.force_output_compare_a = 1;
    set_8_bit_timer_counter_control_registers(timer, &a, &b);
}

inline void strobe_8_bit_timer_counter_force_output_compare_b(int timer) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    b.force_output_compare_b = 1;
    set_8_bit_timer_counter_control_registers(timer, &a, &b);
}

inline unsigned char get_8_bit_timer_counter_clock_select(int timer, unsigned char clock_select) {
    _8_bit_timer_counter_control_register_a a;
    _8_bit_timer_counter_control_register_a b;
    _8_bit_timer_counter_control_register_a b;
    get_8_bit_timer_counter_control_registers(timer, &a, &b);
    b.clock_select = clock_select;
    set_8_bit_timer_counter_control_registers(timer, &a, &b);
}

inline unsigned char get_8_bit_timer_counter_register(int timer) {
    switch (timer) {
    case 0:
        return TCNT0;
    case 2:
        return TCNT2;
    default:
        exit(EXIT_FAILURE);
    }
}

inline void set_8_bit_timer_counter_register(int timer, unsigned char value) {
    switch (timer) {
    case 0:
        TCNT0 = value;
    case 2:
        TCNT2 = value;
    default:
        exit(EXIT_FAILURE);
    }
}

/* References
1: http://www.atmel.com/Images/doc8161.pdf
*/
