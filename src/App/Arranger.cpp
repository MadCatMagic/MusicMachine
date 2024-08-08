#include "App/Arranger.h"
#include "App/Nodes/NodeTypes/VariableNode.h"

#include "imgui.h"

Arranger* Arranger::instance = nullptr;

Arranger::Arranger()
{
	instance = this;
}

void Arranger::Work()
{
	for (VariableNode* node : VariableNode::variableNodes)
		node->output = node->getValue(time);
	time += AudioChannel::dt * tempo / 60.0f;
}

// in our own window
void Arranger::UI()
{
	ImGui::InputFloat("tempo", &tempo, 20.0f, 300.0f);
	tempo = clamp(tempo, 20.0f, 600.0f);
}

int Arranger::getBeat(int mod, float division) const
{
	return (int)(time / division) % mod;
}

float Arranger::getTime(float period) const
{
	return fmodf(time / (float)period, 1.0f);
}
