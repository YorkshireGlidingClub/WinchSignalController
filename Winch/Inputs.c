
#include "Inputs.h"

enum InputState
{
    STATE_OFF,          //Off, no input.
    STATE_ON_DWELL,     //Off, input present.
    STATE_ON,           //On, input present.
    STATE_OFF_DWELL     //On, no input.
};

struct Inputs
{
    unsigned int lInputCount;
    unsigned int lTransitionTime;    
};

struct Inputs inputs;

void Inputs_Init(struct Input* pInputs,
                 unsigned int lInputCount,
                 unsigned int lTransitionTime)
{
    memset(pInputs, 0x00u, sizeof(struct Input) * lInputCount);
    inputs.lInputCount = lInputCount;
    inputs.lTransitionTime = lTransitionTime;
}

void Inputs_Process(struct Input* pInputs, bool* pInputRawStates)
{
    for(unsigned int i = 0; i < inputs.lInputCount; i++)
    {
        struct Input* pInput = &pInputs[i];
        bool bRawState = pInputRawStates[i];

        switch(pInput->cState)
        {
            case STATE_OFF:
            {
                if(bRawState)
                    pInput->cState = STATE_ON_DWELL;
            }
            break;

            case STATE_ON_DWELL:
            {
                if(!bRawState)
                    pInput->cState = STATE_OFF;
                else if(pInput->lTransitionTime >= inputs.lTransitionTime)
                    pInput->cState = STATE_ON;
            }
            break;

            case STATE_ON:
            {
                if(!bRawState)
                    pInput->cState = STATE_OFF_DWELL;
            }
            break;

            case STATE_OFF_DWELL:
            {
                if(bRawState)
                    pInput->cState = STATE_ON;
                else if(pInput->lTransitionTime >= inputs.lTransitionTime)
                    pInput->cState = STATE_OFF;
            }
            break;

            default:
            {
                //Catch corruption by returning to off.
                pInput->cState = STATE_OFF;
            }
        }
    }
}

void Inputs_TimedProcess(struct Input* pInputs, unsigned int lMS)
{
    for(unsigned int i = 0; i < inputs.lInputCount; i++)
    {
        struct Input* pInput = &pInputs[i];

        if((STATE_ON_DWELL == pInput->cState) ||
           (STATE_OFF_DWELL == pInput->cState))
        {
            pInput->lTransitionTime += lMS;
        }
        else
        {
            pInput->lTransitionTime = 0ul;
        }
    }
}

bool Inputs_GetLogicalState(struct Input input)
{
    return ((STATE_ON == input.cState) ||
            (STATE_OFF_DWELL == input.cState));
}

