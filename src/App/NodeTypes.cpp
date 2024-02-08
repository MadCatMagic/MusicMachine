#include "App/NodeTypes.h"
#include "Engine/Console.h"

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
	Console::AddCommand(&ConsoleCommand, "ln");
}

void LongNode::IO()
{
	for (int i = 0; i < nodeNum; i++)
		FloatInput("in " + std::to_string(i + 1), nullptr);
}

void LongNode::ConsoleCommand(std::vector<std::string> arguments)
{
	if (arguments.size() > 0)
		nodeNum = std::stoi(arguments[0]);
}

int LongNode::nodeNum = 6;
