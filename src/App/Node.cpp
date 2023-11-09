#include "App/Node.h"
#include "App/NodeNetwork.h"

void Node::Init()
{
}

void Node::UI()
{
	IntInput("An integer input", nullptr);
	FloatInput("floating input", nullptr);
	IntOutput("An int out", nullptr);
	FloatOutput("and out we go", nullptr);
}

bool Node::IntInput(const std::string& name, int* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Int32;
	TransferInput(o);
	return false;
}

bool Node::IntOutput(const std::string& name, int* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Int32;
	outputs.push_back(o);
	return false;
}

bool Node::FloatInput(const std::string& name, float* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Float;
	TransferInput(o);
	return false;
}

bool Node::FloatOutput(const std::string& name, float* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Float;
	outputs.push_back(o);
	return false;
}

// O(n^2) (over a whole frame)
void Node::TransferInput(const NodeInput& i)
{
	for (size_t j = 0; j < inputs.size(); j++)
		if (inputs[j].name == i.name && inputs[j].type == i.type)
		{
			Node* source = inputs[j].source;
			const std::string& sourceName = inputs[j].sourceName;
			inputs[j] = i;
			inputs[j].source = source;
			inputs[j].sourceName = sourceName;
			inputs[j].touchedThisFrame = true;
			return;
		}
	inputs.push_back(i);
}

void Node::ResetTouchedStatus()
{
	for (size_t i = 0; i < inputs.size(); i++)
		inputs[i].touchedThisFrame = false;
	outputs.clear();
}

void Node::CheckTouchedStatus()
{
	for (size_t i = 0; i < inputs.size(); i++)
		if (inputs[i].touchedThisFrame == false)
			inputs.erase(inputs.begin() + i);
}

void Node::Draw(NodeNetwork* network)
{
	
}