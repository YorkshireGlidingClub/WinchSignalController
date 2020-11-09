
#ifndef INPUTS_H
#define INPUTS_H

#include <stdbool.h>

struct Input
{
    unsigned char cState;
    unsigned int lTransitionTime;
};

void Inputs_Init(struct Input* pInputs,
                 unsigned int lInputCount,
                 unsigned int lTransitionTime);
void Inputs_Process(struct Input* pInputs, bool* pInputRawStates);
void Inputs_TimedProcess(struct Input* pInputs, unsigned int lMS);
bool Inputs_GetLogicalState(struct Input input);

#endif

