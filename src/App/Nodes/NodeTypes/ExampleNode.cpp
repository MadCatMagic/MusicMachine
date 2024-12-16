#include "App/Nodes/NodeTypes/ExampleNode.h"
#include "Engine/DrawList.h"

void ExampleNode::Init()
{
	name = "ExampleNode";
	title = "Example Node";
	minSpace = v2(100.0f, 100.0f);
}

void ExampleNode::IO()
{
	AudioInput("audio input", &ichannel);
	AudioOutput("audio output", &ochannel);
	SequencerInput("sequencer in", &pinput);
	SequencerOutput("sequencer out", &poutput);
	FloatInput("float value", &finput);
	IntInput("int value", &iinput);
	BoolInput("boolean value", &binput);
	FloatInput("db volume", &dbinput, 0.0f, 2.0f, true, true, Node::FloatDisplayType::Db);
}

void ExampleNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	dl->Rect(topLeft, topLeft + minSpace, ImColor(0.8f, 0.8f, 0.8f));
	dl->Line(topLeft, topLeft + minSpace, ImColor(0.8f, 0.8f, 0.8f));
}
