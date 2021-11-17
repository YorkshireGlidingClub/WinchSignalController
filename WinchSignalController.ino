
#include "Winch/Winch.h"
#include "Winch/Winch.c"
#include "Winch/Inputs.h"
#include "Winch/Inputs.c"

//Input and output pin assignments.
#define INPUT_PIN_UP_SLACK 6
#define INPUT_PIN_ALL_OUT  7
#define INPUT_PIN_STOP     8
#define OUTPUT_BUZZER 17
#define OUTPUT_LAMP1  14
#define OUTPUT_LAMP2  15

//Input state+filtering library setup.
enum
{
    INPUT_UPSLACK = 0,
    INPUT_ALLOUT,
    INPUT_STOP,
    INPUT_COUNT
};

struct Input InputStates[INPUT_COUNT];
bool RawInputStates[INPUT_COUNT];

#define INPUT_TRANSITION_TIME   50ul    //Buttons must be on/off for this many ms to change state.
#define INTERRUPT_FREQUENCY_MS  10u     //Timed interrupts occur every this many ms.
#define TRANSMIT_FREQUENCY      50u     //Data is wirelessly transmitted to the winch every this many ms.

unsigned int lTransmitTimer = 0;

char msgUpSlack[] = "UUUUUUUU";
char msgAllOut[] = "AAAAAAAA";
char msgStop[] = "SSSSSSSS";
char msgNowt[] = "NNNNNNNN";

//10ms interrupt.
ISR(TCA0_OVF_vect)
{
    Inputs_TimedProcess(InputStates, INTERRUPT_FREQUENCY_MS);
    Winch_TimedProcess(INTERRUPT_FREQUENCY_MS);
    lTransmitTimer += INTERRUPT_FREQUENCY_MS;
    
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void _Winch_BuzzerState(boolean bOn) { digitalWrite(OUTPUT_BUZZER, !bOn); }
void _Winch_Lamp1State(boolean bOn) { digitalWrite(OUTPUT_LAMP1, !bOn); }
void _Winch_Lamp2State(boolean bOn) { digitalWrite(OUTPUT_LAMP2, !bOn); }

void loop()
{
    //Process inputs.
    RawInputStates[INPUT_UPSLACK] = !digitalRead(INPUT_PIN_UP_SLACK);
    RawInputStates[INPUT_ALLOUT] = !digitalRead(INPUT_PIN_ALL_OUT);
    RawInputStates[INPUT_STOP] = !digitalRead(INPUT_PIN_STOP);

    Inputs_Process(InputStates, RawInputStates);
    
    Winch_Process(Inputs_GetLogicalState(InputStates[INPUT_STOP]),
                  Inputs_GetLogicalState(InputStates[INPUT_PIN_ALL_OUT]),
                  Inputs_GetLogicalState(InputStates[INPUT_PIN_STOP]));

    if(lTransmitTimer > TRANSMIT_FREQUENCY)
    {
        switch(Winch_GetState())
        {
            case STATE_UP_SLACK: Serial1.write(msgUpSlack); break;
            case STATE_ALL_OUT: Serial1.write(msgAllOut); break;
            case STATE_STOP: Serial1.write(msgStop); break;
            default: Serial1.write(msgNowt); break;
        }

        lTransmitTimer = 0;
    }
}

void setup()
{
    Serial1.begin(9600);
    
    pinMode(OUTPUT_BUZZER, OUTPUT);
    pinMode(OUTPUT_LAMP1, OUTPUT);
    pinMode(OUTPUT_LAMP2, OUTPUT);

    pinMode(INPUT_PIN_UP_SLACK, INPUT_PULLUP);
    pinMode(INPUT_PIN_ALL_OUT, INPUT_PULLUP);
    pinMode(INPUT_PIN_STOP, INPUT_PULLUP);
    
    Inputs_Init(InputStates, INPUT_COUNT, INPUT_TRANSITION_TIME);

    //Configure timed interrupts.
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    TCA0.SINGLE.PER = 0x0271;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

    //Enable interrupts.
    sei();
}
