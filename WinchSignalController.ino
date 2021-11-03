
#include "Winch/Winch.h"
#include "Winch/Winch.c"

#define OUTPUT_BUZZER 17
#define OUTPUT_LAMP1  14
#define OUTPUT_LAMP2  15

#define INPUT_UP_SLACK 6
#define INPUT_ALL_OUT  7
#define INPUT_STOP     8

#define INTERRUPT_FREQUENCY_MS 10u

boolean bStopPressed = false;
boolean bUpSlackPressed = false;
boolean bAllOutPressed = false;

//10ms interrupt.
ISR(TCA0_OVF_vect)
{
    Winch_TimedProcess(INTERRUPT_FREQUENCY_MS);
    
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void _Winch_BuzzerState(boolean bOn) { digitalWrite(OUTPUT_BUZZER, !bOn); }
void _Winch_Lamp1State(boolean bOn) { digitalWrite(OUTPUT_LAMP1, !bOn); }
void _Winch_Lamp2State(boolean bOn) { digitalWrite(OUTPUT_LAMP2, !bOn); }

void loop()
{
    //Process inputs.
    bUpSlackPressed = !digitalRead(INPUT_UP_SLACK);
    bAllOutPressed = !digitalRead(INPUT_ALL_OUT);
    bStopPressed = !digitalRead(INPUT_STOP);

    Winch_Process(bStopPressed, bUpSlackPressed, bAllOutPressed);
}

void setup()
{
    pinMode(OUTPUT_BUZZER, OUTPUT);
    pinMode(OUTPUT_LAMP1, OUTPUT);
    pinMode(OUTPUT_LAMP2, OUTPUT);

    pinMode(INPUT_UP_SLACK, INPUT_PULLUP);
    pinMode(INPUT_ALL_OUT, INPUT_PULLUP);
    pinMode(INPUT_STOP, INPUT_PULLUP);

    //Configure timed interrupts.
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    TCA0.SINGLE.PER = 0x0271;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

    //Enable interrupts.
    sei();
}
