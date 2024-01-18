#include "App/NodeTypes.h"
#include "Engine/Console.h"

void MathsNode::Init()
{
	name = "Maths Innit";
	minSpace = v2(30, 40);
}

void MathsNode::IO()
{
	FloatInput("Input A", &a, 0.0f, 2.0f);
	FloatInput("Input B", &b);
	IntInput("Integer brugh", &c);
	BoolInput("Freak out", nullptr);
	FloatOutput("Result", &result);
	IntOutput("Result rounded", &resultRounded);
}

void LongNode::Init()
{
	name = "Long Node";
}

void LongNode::IO()
{
	FloatInput("in 1", &in1);
	FloatInput("in 2", &in2);
	FloatInput("in 3", &in3);
	FloatInput("in 4", &in4);
	FloatInput("in 5", &in5);
	FloatInput("in 6", &in6);
}

std::string LongNode::Result()
{
	return
		std::to_string(in1) + ", " +
		std::to_string(in2) + ", " +
		std::to_string(in3) + ", " +
		std::to_string(in4) + ", " +
		std::to_string(in5) + ", " +
		std::to_string(in6);
}