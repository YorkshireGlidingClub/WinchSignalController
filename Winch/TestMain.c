
/*
To build:
gcc TestMain.c -pthread o TestMain
*/

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "Winch.h"
#include "Winch.c"

#define SLEEP_DURATION_US 100000
#define TICK_DURATION_MS  100

pthread_t thread;

bool bStopPressed = false;
bool bUpSlackPressed = false;
bool bAllOutPressed = false;

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
        Winch_Process(bStopPressed, bUpSlackPressed, bAllOutPressed);
        Winch_TimedProcess(TICK_DURATION_MS);

        printf("  US:[%c] AA:[%c] Stop:[%c] Buzzer:{%c} LLamp:(%c) RLamp:(%c)  \r",
               bUpSlackPressed ? 'x' : ' ',
               bAllOutPressed ? 'x' : ' ',
               bStopPressed ? 'x' : ' ',
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
                    bUpSlackPressed = !bUpSlackPressed;
                }
                break;

                case 'X':
                case 'x':
                {
                    bAllOutPressed = !bAllOutPressed;
                }
                break;

                case 'C':
                case 'c':
                {
                    bStopPressed = !bStopPressed;
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

