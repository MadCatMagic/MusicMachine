#include "App/Sequencer.h"

void PitchSequencer::CopyTo(PitchSequencer* target)
{
	target->length = length;
	target->pitch = pitch;
	target->velocity = velocity;
	target->cumulativeSamples = cumulativeSamples;
}