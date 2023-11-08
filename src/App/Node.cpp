#include "App/Node.h"
#include "App/NodeNetwork.h"

bool Node::IntInput(const std::string& name, int* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Int32;
	inputs.push_back(o);
	return true;
}

bool Node::IntOutput(const std::string& name, int* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Int32;
	outputs.push_back(o);
	return true;
}

bool Node::FloatInput(const std::string& name, float* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Float;
	inputs.push_back(o);
	return true;
}

bool Node::FloatOutput(const std::string& name, float* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Float;
	outputs.push_back(o);
	return true;
}

#include "imgui.h"
void Node::Draw(NodeNetwork* network)
{
	
}

v2 Node::GetBounds() const
{
	return v2(20, 20);
}
