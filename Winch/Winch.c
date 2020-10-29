
#include "Winch.h"

enum State
{
    STATE_IDLE = 0,     //Awaiting input.
    STATE_UP_SLACK,     //Show up slack sequence.
    STATE_ALL_OUT,      //Show all out sequence.
    STATE_STOP,         //Show stop sequence. Wait for reset procedure.
    STATE_STOP_RESET    //Reset procedure applied. Wait for all input to stop to return to idle.
};

struct Sequence
{
    unsigned int* plSequenceDurations;
    unsigned int lSequenceCount;
    unsigned int lSequencePosition;
    bool bState;
};

unsigned char cState = STATE_IDLE;

unsigned int lSequenceBuzzerUpSlack[] = { 100, 400 };
unsigned int lSequenceBuzzerAllOut[] = { 50, 150 };
struct Sequence sequenceBuzzerUpSlack = { &lSequenceBuzzerUpSlack[0], 2, 0, false };
struct Sequence sequenceBuzzerAllOut = { &lSequenceBuzzerAllOut[0], 2, 0, false };

unsigned int lSequenceLampUpSlack[] = { 500, 500 };
unsigned int lSequenceLampAllOut[] = { 100, 100 };
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

void Winch_Process(bool bStop, bool bUpSlack, bool bAllOut)
{
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

            _Winch_BuzzerState(SequenceIsOn(sequenceBuzzerUpSlack));
            _Winch_Lamp1State(SequenceIsOn(sequenceLampUpSlack));
            _Winch_Lamp2State(!SequenceIsOn(sequenceLampUpSlack));
        }
        break;

        case STATE_ALL_OUT:
        {
            //Lights flashing together.
            ProcessSequence(&lBuzzerSequenceTime, &sequenceBuzzerAllOut);
            ProcessSequence(&lLampSequenceTime, &sequenceLampAllOut);

            _Winch_BuzzerState(SequenceIsOn(sequenceBuzzerAllOut));
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

