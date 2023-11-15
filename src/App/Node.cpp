#include "App/Node.h"
#include "App/NodeNetwork.h"

bool Node::HandleClick(const v2& nodePos)
{
	v2 centre = v2(size.x - 8.0f, headerHeight * 0.5f);
	if (v2::Distance(nodePos, centre) <= 6.0f)
	{
		mini = !mini;
		return true;
	}
	return false;
}

void Node::Init()
{
}

void Node::IO()
{
	BoolInput("A boolean input", nullptr);
	FloatInput("floating input", nullptr);
	BoolOutput("A bool out", nullptr);
	FloatOutput("and out we go", nullptr);
}

bool Node::BoolInput(const std::string& name, bool* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Bool;
	TransferInput(o);
	return false;
}

bool Node::BoolOutput(const std::string& name, bool* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Bool;
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
	inputs[inputs.size() - 1].touchedThisFrame = true;
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
	v2 cursor = position;
	UpdateDimensions();

	if (!mini)
	{
		// leave spaces
		cursor.y += 24.0f;
		cursor.y += 16.0f * outputs.size();
		cursor.y += minSpace.y;

		// draw inputs
		for (const NodeInput& input : inputs)
		{
			cursor.y += 8.0f;
			network->DrawInput(cursor, input.name, input.type);
			cursor.y += 8.0f;
		}

		cursor = position;
		// draw node header
		network->DrawHeader(cursor, name, size.x, headerHeight, mini);
		cursor.y += headerHeight + 4.0f;

		// draw outputs
		for (const NodeOutput& output : outputs)
		{
			cursor.y += 8.0f;
			network->DrawOutput(cursor, size.x, output.name, output.type);
			cursor.y += 8.0f;
		}
	}
	else 
		network->DrawHeader(cursor, name, size.x, headerHeight, mini);
}

void Node::UpdateDimensions()
{
	// calculate node width
	float maxXOff = 0.0f;
	for (const NodeInput& input : inputs)
		maxXOff = std::max(IOWidth(input.name), maxXOff);
	for (const NodeOutput& output : outputs)
		maxXOff = std::max(IOWidth(output.name), maxXOff);
	maxXOff = std::max(minSpace.x, maxXOff);
	maxXOff = std::max(IOWidth(name) + 6.0f, maxXOff);

	if (mini)
		size = v2(maxXOff, headerHeight);
	else
		size = v2(maxXOff, headerHeight + 8.0f + minSpace.y + inputs.size() * 16.0f + outputs.size() * 16.0f);
}

float Node::IOWidth(const std::string& text)
{
	// return length
	// text space + 8px padding either side
	return 6.0f * (float)(text.size() + 1) + 16.0f;
}