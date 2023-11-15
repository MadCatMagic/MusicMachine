#include "App/NodeTypes.h"

void MathsNode::Init()
{
	name = "Maths Innit";
	minSpace = v2(30, 40);
}

void MathsNode::IO()
{
	FloatInput("Input A", &a);
	FloatInput("Input B", &b);
	FloatOutput("Result", &result);
}
