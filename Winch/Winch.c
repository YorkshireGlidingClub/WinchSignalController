
#include "Winch.h"

struct Sequence
{
    unsigned int* plSequenceDurations;
    unsigned int lSequenceCount;
    unsigned int lSequencePosition;
    bool bState;
};

unsigned char cState = STATE_IDLE;
bool bBuzzOnlyOnStop = false;

unsigned int lSequenceBuzzerUpSlack[] = { 300, 450 };
unsigned int lSequenceBuzzerAllOut[] = { 100, 300 };
struct Sequence sequenceBuzzerUpSlack = { &lSequenceBuzzerUpSlack[0], 2, 0, false };
struct Sequence sequenceBuzzerAllOut = { &lSequenceBuzzerAllOut[0], 2, 0, false };

unsigned int lSequenceLampUpSlack[] = { 750, 750 };
unsigned int lSequenceLampAllOut[] = { 200, 200 };
struct Sequence sequenceLampUpSlack = { &lSequenceLampUpSlack[0], 2, 0, false }; //Lamps alternating.
struct Sequence sequenceLampAllOut = { &lSequenceLampAllOut[0], 2, 0, false }; //Lamps flashing together.
//Stop lamp sequence is continuously illuminated.

//Times in each part of the sequence.
unsigned int lBuzzerSequenceTime = 0;
unsigned int lLampSequenceTime = 0;

static void ProcessSequence(unsigned int* plSequenceTime, struct Sequence* pSequence)
{
    if(*plSequenceTime >= pSequence->plSequenceDurations[pSequence->lSequencePosition])
    {
        *plSequenceTime = 0ul;

        pSequence->lSequencePosition++;

        if(pSequence->lSequencePosition >= pSequence->lSequenceCount)
        {
            pSequence->lSequencePosition = 0;
        }
    }
}

static bool SequenceIsOn(struct Sequence sequence)
{
    return sequence.lSequencePosition == 0u;
}

void Winch_Init(bool bBuzzOnStopOnly)
{
    bBuzzOnlyOnStop = bBuzzOnStopOnly;
}

#ifdef WINCH_REPEATER
void Winch_SetUpSlack()
{
    cState = STATE_UP_SLACK;
}

void Winch_SetAllOut()
{
    cState = STATE_ALL_OUT;
}

void Winch_SetStop()
{
    cState = STATE_STOP;
}

void Winch_SetIdle()
{
    cState = STATE_IDLE;
}

void Winch_Process()
#else
void Winch_Process(bool bStop, bool bUpSlack, bool bAllOut)
#endif
{
    #ifndef WINCH_REPEATER
    //Process state switching based on inputs.
    switch(cState)
    {
        case STATE_IDLE:
        case STATE_UP_SLACK:
        case STATE_ALL_OUT:
        {
            if(bStop)
                cState = STATE_STOP;
            else if(bAllOut)
                cState = STATE_ALL_OUT;
            else if(bUpSlack)
                cState = STATE_UP_SLACK;
            else
                cState = STATE_IDLE;
        }
        break;

        case STATE_STOP:
        {
            if(!bStop && bAllOut && bUpSlack)
                cState = STATE_STOP_RESET;
        }
        break;

        case STATE_STOP_RESET:
        {
            if(!bStop && !bAllOut && !bUpSlack)
                cState = STATE_IDLE;
        }
        break;

        default:
        {
            //Any kind of corruption of cState should trigger a stop.
            cState = STATE_STOP;
        }
    }
    #endif

    //Process outputs based on state.
    switch(cState)
    {
        case STATE_IDLE:
        case STATE_STOP_RESET:
        {
            //Everything off.
            _Winch_BuzzerState(false);
            _Winch_Lamp1State(false);
            _Winch_Lamp2State(false);
        }
        break;

        case STATE_UP_SLACK:
        {
            //Lights alternating.
            ProcessSequence(&lBuzzerSequenceTime, &sequenceBuzzerUpSlack);
            ProcessSequence(&lLampSequenceTime, &sequenceLampUpSlack);

            _Winch_BuzzerState(SequenceIsOn(sequenceBuzzerUpSlack) && !bBuzzOnlyOnStop);
            _Winch_Lamp1State(SequenceIsOn(sequenceLampUpSlack));
            _Winch_Lamp2State(!SequenceIsOn(sequenceLampUpSlack));
        }
        break;

        case STATE_ALL_OUT:
        {
            //Lights flashing together.
            ProcessSequence(&lBuzzerSequenceTime, &sequenceBuzzerAllOut);
            ProcessSequence(&lLampSequenceTime, &sequenceLampAllOut);

            _Winch_BuzzerState(SequenceIsOn(sequenceBuzzerAllOut) && !bBuzzOnlyOnStop);
            _Winch_Lamp1State(SequenceIsOn(sequenceLampAllOut));
            _Winch_Lamp2State(SequenceIsOn(sequenceLampAllOut));
        }
        break;

        case STATE_STOP:
        default:
        {
            //Lights constant on.
            //Any kind of corruption of cState should behave as a stop.
            _Winch_BuzzerState(true);
            _Winch_Lamp1State(true);
            _Winch_Lamp2State(true);
        }
    }
}

void Winch_TimedProcess(unsigned int lMS)
{
    lBuzzerSequenceTime += lMS;
    lLampSequenceTime += lMS;
}

unsigned char Winch_GetState()
{
    return cState;
}

