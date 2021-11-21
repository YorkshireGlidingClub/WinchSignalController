
#define WINCH_REPEATER

#include "/home/tobster/WinchSignalController/Winch/Winch.h"
#include "/home/tobster/WinchSignalController/Winch/Winch.c"

//Output pin assignments.
#define OUTPUT_BUZZER   11
#define OUTPUT_LAMP1    2
#define OUTPUT_LAMP2    12
#define OUTPUT_NO_COMMS 3

#define INTERRUPT_FREQUENCY_MS  10u     //Timed interrupts occur every this many ms.

#define COMMS_TIMEOUT           500u    //Time of no-receive to assume connection is interrupted.

#define CHAR_UP_SLACK 'U'
#define CHAR_ALL_OUT  'A'
#define CHAR_STOP     'S'
#define CHAR_NOWT     'N'

unsigned int lLastCommandTime = 0;

bool bLED = false;

//10ms interrupt.
ISR(TIMER1_COMPA_vect)
{
    Winch_TimedProcess(INTERRUPT_FREQUENCY_MS);
    lLastCommandTime += INTERRUPT_FREQUENCY_MS;
}

void _Winch_BuzzerState(boolean bOn) { digitalWrite(OUTPUT_BUZZER, !bOn); }
void _Winch_Lamp1State(boolean bOn) { digitalWrite(OUTPUT_LAMP1, bOn); }
void _Winch_Lamp2State(boolean bOn) { digitalWrite(OUTPUT_LAMP2, bOn); }

void loop()
{
    char cLastCommand = '0';

    while(Serial1.available())
    {
        cLastCommand = Serial1.read();
        bLED = !bLED;
        digitalWrite(LED_BUILTIN, bLED? HIGH : LOW);
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

    if(lLastCommandTime > COMMS_TIMEOUT)
    {
        Winch_SetIdle();
        digitalWrite(OUTPUT_NO_COMMS, true);
    }
    else
    {
        digitalWrite(OUTPUT_NO_COMMS, false);
    }
    
    Winch_Process();
}

void setup()
{
    Winch_Init(true);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial1.begin(9600);
    
    pinMode(OUTPUT_BUZZER, OUTPUT);
    pinMode(OUTPUT_LAMP1, OUTPUT);
    pinMode(OUTPUT_LAMP2, OUTPUT);
    pinMode(OUTPUT_NO_COMMS, OUTPUT);
    
    //Configure timed 10ms interrupts on Timer/Counter 1 (ATMega23U4).
    TCCR1A = 0;
    TCCR1B = 0x0C;
    TCCR1C = 0;
    TCNT1 = 0;
    OCR1A = 0x0271;
    TIMSK1 = 2;
    

    //Enable interrupts.
    sei();
}
