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
	BoolInput("Freak out", nullptr);
	FloatOutput("Result", &result);
}

void LongNode::Init()
{
	name = "Long Node";
}

void LongNode::IO()
{
	FloatInput("in 1", nullptr);
	FloatInput("in 2", nullptr);
	BoolInput("in 3", nullptr);
	FloatInput("in 4", nullptr);
	BoolInput("and", nullptr);
	BoolInput("so", nullptr);
	FloatInput("on", nullptr);
}
