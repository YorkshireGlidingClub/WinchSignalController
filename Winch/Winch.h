
#ifndef WINCH_H
#define WINCH_H

#include <stdbool.h>

enum WinchState
{
    STATE_IDLE = 0,     //Awaiting input.
    STATE_UP_SLACK,     //Show up slack sequence.
    STATE_ALL_OUT,      //Show all out sequence.
    STATE_STOP,         //Show stop sequence. Wait for reset procedure.
    STATE_STOP_RESET    //Reset procedure applied. Wait for all input to stop to return to idle.
};

extern void _Winch_BuzzerState(bool bOn);
extern void _Winch_Lamp1State(bool bOn);
extern void _Winch_Lamp2State(bool bOn);

void Winch_Process(bool bStop, bool bUpSlack, bool bAllOut);
void Winch_TimedProcess(unsigned int lMS);
unsigned char Winch_GetState();

#endif

