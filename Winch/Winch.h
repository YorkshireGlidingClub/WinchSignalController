
#ifndef WINCH_H
#define WINCH_H

#include <stdbool.h>

extern void _Winch_BuzzerState(bool bOn);
extern void _Winch_Lamp1State(bool bOn);
extern void _Winch_Lamp2State(bool bOn);

void Winch_Process(bool bStop, bool bUpSlack, bool bAllOut);
void Winch_TimedProcess(unsigned int lMS);

#endif

