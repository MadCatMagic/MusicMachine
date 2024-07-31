#include "App/Nodes/NodeTypes/NoiseNode.h"

void NoiseNode::Init()
{
	name = "NoiseNode";
	title = "Noise";
}

void NoiseNode::IO()
{
	AudioOutput("noise", &c);
}

void NoiseNode::Work()
{
	for (int i = 0; i < c.bufferSize; i++)
		c.data[i] = v2((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
}
