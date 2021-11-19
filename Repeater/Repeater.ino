
#define WINCH_REPEATER

#include "/home/tobster/WinchSignalController/Winch/Winch.h"
#include "/home/tobster/WinchSignalController/Winch/Winch.c"

//Output pin assignments.
#define OUTPUT_BUZZER 11
#define OUTPUT_LAMP1  2
#define OUTPUT_LAMP2  12

#define INTERRUPT_FREQUENCY_MS  10u     //Timed interrupts occur every this many ms.

#define COMMS_TIMEOUT           500u    //Time of no-receive to assume connection is interrupted.

#define CHAR_UP_SLACK 'U'
#define CHAR_ALL_OUT  'A'
#define CHAR_STOP     'S'
#define CHAR_NOWT     'N'

char cLastCommand = '0';
unsigned int lLastCommandTime;

//10ms interrupt.
ISR(TCA0_OVF_vect)
{
    Winch_TimedProcess(INTERRUPT_FREQUENCY_MS);
    lLastCommandTime += INTERRUPT_FREQUENCY_MS;
    
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void _Winch_BuzzerState(boolean bOn) { digitalWrite(OUTPUT_BUZZER, !bOn); }
void _Winch_Lamp1State(boolean bOn) { digitalWrite(OUTPUT_LAMP1, !bOn); }
void _Winch_Lamp2State(boolean bOn) { digitalWrite(OUTPUT_LAMP2, !bOn); }

void loop()
{
    while(Serial.available())
    {
        cLastCommand = Serial.read();
    }

    switch(cLastCommand)
    {
        case CHAR_UP_SLACK:
        {
            Winch_SetUpSlack();
            lLastCommandTime = 0ul;
        }
        break;
        
        case CHAR_ALL_OUT:
        {
            Winch_SetAllOut();
            lLastCommandTime = 0ul;
        }
        break;
        
        case CHAR_STOP:
        {
            Winch_SetStop();
            lLastCommandTime = 0ul;
        }
        break;
        
        case CHAR_NOWT:
        {
            Winch_SetIdle();
            lLastCommandTime = 0ul;
        }
        break;
    }
    
    Winch_Process();
}

void setup()
{
    Winch_Init(true);
    
    Serial1.begin(9600);
    
    pinMode(OUTPUT_BUZZER, OUTPUT);
    pinMode(OUTPUT_LAMP1, OUTPUT);
    pinMode(OUTPUT_LAMP2, OUTPUT);
    
    //Configure timed interrupts.
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.EVCTRL &= ~(TCA_SINGLE_CNTEI_bm);
    TCA0.SINGLE.PER = 0x0271;
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV256_gc | TCA_SINGLE_ENABLE_bm;

    //Enable interrupts.
    sei();
}
