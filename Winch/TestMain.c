
/*
To build:
gcc TestMain.c -pthread -o TestMain
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "Winch.h"
#include "Winch.c"
#include "Inputs.h"
#include "Inputs.c"

#define SLEEP_DURATION_US       10000ul
#define TICK_DURATION_MS        10ul
#define INPUT_TRANSITION_TIME   100ul

pthread_t thread;

enum
{
    INPUT_UPSLACK = 0,
    INPUT_ALLOUT,
    INPUT_STOP,
    INPUT_COUNT
};

struct Input InputStates[INPUT_COUNT];
bool RawInputStates[INPUT_COUNT];

bool bBuzzer = false;
bool bLamp1 = false;
bool bLamp2 = false;

void _Winch_BuzzerState(bool bOn)
{
    bBuzzer = bOn;
}

void _Winch_Lamp1State(bool bOn)
{
    bLamp1 = bOn;
}

void _Winch_Lamp2State(bool bOn)
{
    bLamp2 = bOn;
}

bool bRunning = true;

void* LightsThread(void *arg)
{
    unsigned int number = 0;

    while(bRunning)
    {
        Inputs_TimedProcess(InputStates, TICK_DURATION_MS);   
        Winch_TimedProcess(TICK_DURATION_MS);

        Inputs_Process(InputStates, RawInputStates);
        Winch_Process(Inputs_GetLogicalState(InputStates[INPUT_STOP]),
                      Inputs_GetLogicalState(InputStates[INPUT_UPSLACK]),
                      Inputs_GetLogicalState(InputStates[INPUT_ALLOUT]));

        printf("  US:[%c%c] AA:[%c%c] Stop:[%c%c] Buzzer:{%c} LLamp:(%c) RLamp:(%c)  \r",
               RawInputStates[INPUT_UPSLACK] ? '|' : ' ',
               Inputs_GetLogicalState(InputStates[INPUT_UPSLACK]) ? 'x' : ' ',
               RawInputStates[INPUT_ALLOUT] ? '|' : ' ',
               Inputs_GetLogicalState(InputStates[INPUT_ALLOUT]) ? 'x' : ' ',
               RawInputStates[INPUT_STOP] ? '|' : ' ',
               Inputs_GetLogicalState(InputStates[INPUT_STOP]) ? 'x' : ' ',
               bBuzzer ? '!' : ' ',
               bLamp1 ? '#' : ' ',
               bLamp2 ? '#' : ' ');

        fflush(stdout);
        number++;
        usleep(SLEEP_DURATION_US);
    }
}

int main()
{
    int c;
    system ("/bin/stty raw");
    Winch_Init(false);
    Inputs_Init(InputStates, INPUT_COUNT, INPUT_TRANSITION_TIME);

    printf("Z - Up slack    X - All Out    C - Stop    Z+X - Stop Reset (all toggle)\r\n");
    printf("Any other button quits.\r\n");

    pthread_create(&thread, NULL, &LightsThread, NULL);

    while(bRunning)
    {
        c = getchar();
        {
            switch(c)
            {
                case 'Z':
                case 'z':
                {
                    RawInputStates[INPUT_UPSLACK] = !RawInputStates[INPUT_UPSLACK];
                }
                break;

                case 'X':
                case 'x':
                {
                    RawInputStates[INPUT_ALLOUT] = !RawInputStates[INPUT_ALLOUT];
                }
                break;

                case 'C':
                case 'c':
                {
                    RawInputStates[INPUT_STOP] = !RawInputStates[INPUT_STOP];
                }
                break;

                default: bRunning = false;
            }
        }

        usleep(SLEEP_DURATION_US);
    }

    system ("/bin/stty cooked");

    return 0;
}

