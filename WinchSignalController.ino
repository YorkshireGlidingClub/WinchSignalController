
#include "Winch/Winch.h"
#include "Winch/Winch.c"

#define OUTPUT_RELAY_1  6
#define OUTPUT_RELAY_2  7
#define OUTPUT_RELAY_3  8
#define OUTPUT_RELAY_4  9

#define OUTPUT_BUZZER OUTPUT_RELAY_1
#define OUTPUT_LAMP1  OUTPUT_RELAY_2
#define OUTPUT_LAMP2  OUTPUT_RELAY_3

#define INPUT_UP_SLACK 2
#define INPUT_ALL_OUT  3
#define INPUT_STOP     4

#define INTERRUPT_FREQUENCY_MS 10u

boolean bStopPressed = false;
boolean bUpSlackPressed = false;
boolean bAllOutPressed = false;

//10ms interrupt.
ISR( TIMER0_COMPA_vect)
{
    Winch_TimedProcess(INTERRUPT_FREQUENCY_MS);
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
    pinMode(OUTPUT_RELAY_4, OUTPUT);

    pinMode(INPUT_UP_SLACK, INPUT_PULLUP);
    pinMode(INPUT_ALL_OUT, INPUT_PULLUP);
    pinMode(INPUT_STOP, INPUT_PULLUP);
    
    //Set timer0 interrupt at 10ms
    TCCR0A = 0;// set entire TCCR2A register to 0
    TCCR0B = 0;// same for TCCR2B
    TCNT0  = 0;//initialize counter value to 0
    // set compare match register for 10ms increments
    OCR0A = 156;// = (16*10^6) / (100*1024) - 1 (must be <256)
    // turn on CTC mode
    TCCR0A |= (1 << WGM01);
    // Set CS01 and CS00 bits for 64 prescaler
    TCCR0B |= (1 << CS02) | (1 << CS00);   
    // enable timer compare interrupt
    TIMSK0 |= (1 << OCIE0A);

    //Enable interrupts.
    sei();
}
