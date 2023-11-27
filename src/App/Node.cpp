#include "App/Node.h"
#include "App/NodeNetwork.h"

NodeClickResponse Node::HandleClick(const v2& nodePos)
{
	NodeClickResponse r;
	v2 centre = v2(size.x - 8.0f, headerHeight * 0.5f);
	if (v2::Distance(nodePos, centre) <= 6.0f)
	{
		mini = !mini;
		r.handled = true;
		r.type = NodeClickResponseType::Minimise;
		return r;
	}

	if (mini)
		return r;
	
	v2 worldPos = nodePos + position;
	v2 firstOutput = GetOutputPos(0);
	for (size_t i = 0; i < outputs.size(); i++)
		if (v2::Distance(firstOutput + v2(0.0f, 16.0f * i), worldPos) <= 8.0f)
		{
			r.handled = true;
			r.type = NodeClickResponseType::BeginConnection;
			r.originName = outputs[i].name;
			r.origin = this;
			return r;
		}

	v2 firstInput = GetInputPos(0);
	for (size_t i = 0; i < inputs.size(); i++)
		if (v2::Distance(firstInput + v2(0.0f, 16.0f * i), worldPos) <= 8.0f)
		{
			r.handled = true;
			if (inputs[i].source != nullptr)
			{
				r.type = NodeClickResponseType::BeginConnection;
				r.originName = inputs[i].sourceName;
				r.origin = inputs[i].source;
				Disconnect(i);
			}
			else 
			{
				r.type = NodeClickResponseType::BeginConnectionReversed;
				r.originName = inputs[i].name;
				r.origin = this;
			}
			return r;
		}

	return r;
}

bool Node::Connect(size_t inputIndex, Node* origin, size_t originIndex)
{
	if (inputs.size() <= inputIndex || origin->outputs.size() <= originIndex)
		return false;
	if (inputs[inputIndex].source != nullptr)
		Disconnect(inputIndex);
	inputs[inputIndex].source = origin;
	inputs[inputIndex].sourceName = origin->outputs[originIndex].name;
	inputs[inputIndex].target = origin->outputs[originIndex].data;
	origin->outputs[originIndex].connections++;
	return true;
}

void Node::Disconnect(size_t inputIndex)
{
	if (inputs.size() <= inputIndex)
		return;
	NodeInput& i = inputs[inputIndex];
	for (size_t k = 0; k < i.source->outputs.size(); k++)
		if (i.source->outputs[k].name == i.sourceName)
		{
			i.source->outputs[k].connections--;
			break;
		}
	i.source = nullptr;
	i.sourceName = "";
	i.target = nullptr;
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

float Node::headerSize() const
{
	return std::max(headerHeight, 18.0f * (inputs.size() - 1) / (PI * 0.6f));
}

// O(n^2) (over a whole frame)
void Node::TransferInput(const NodeInput& i)
{
	for (size_t j = 0; j < inputs.size(); j++)
		if (inputs[j].name == i.name && inputs[j].type == i.type)
		{
			Node* source = inputs[j].source;
			std::string sourceName = inputs[j].sourceName;
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

bool Node::TryConnect(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed)
{
	if (connectionReversed)
	{
		v2 firstOutput = GetOutputPos(0);
		for (size_t i = 0; i < outputs.size(); i++)
			if (v2::Distance(firstOutput + v2(0.0f, 16.0f * i), pos) <= 6.0f && origin->GetInputType(originName) == outputs[i].type)
			{
				origin->Connect(origin->GetInputIndex(originName), this, i);
				return true;
			}
	}
	else
	{
		v2 firstInput = GetInputPos(0);
		for (size_t i = 0; i < inputs.size(); i++)
			if (v2::Distance(firstInput + v2(0.0f, 16.0f * i), pos) <= 6.0f && origin->GetOutputType(originName) == inputs[i].type)
			{
				Connect(i, origin, origin->GetOutputIndex(originName));
				return true;
			}
	}

	return false;
}

v2 Node::GetInputPos(size_t index) const
{
	if (!mini)
	{
		return position + v2(0.0f, headerHeight + 4.0f + 16.0f * outputs.size() + minSpace.y + 16.0f * index + 8.0f);
	}
	else
	{
		if (inputs.size() > 1)
		{
			const float hh = headerSize() * 0.5f;
			const float offx = 1.0f - cosf(0.3 * PI);
			const float angle = PI * (0.2f + 0.6f * index / (float)(inputs.size() - 1));
			v2 offset = v2(sinf(-angle) - offx, -cosf(angle));
			return position + hh + offset * hh;
		}
		return position + v2(0.0f, headerHeight * 0.5f);
	}
}

v2 Node::GetInputPos(const std::string& name) const
{
	size_t n = GetInputIndex(name);
	if (n == -1)
		return v2::zero;
	return GetInputPos(n);
}

v2 Node::GetOutputPos(size_t index) const
{
	if (!mini)
	{
		return position + v2(size.x, headerHeight + 4.0f + 16.0f * index + 8.0f);
	}
	else
	{
		if (outputs.size() > 1)
		{
			const float offx = 1.0f - cosf(0.3 * PI);
			const float hh = headerSize() * 0.5f;
			const float angle = PI * (0.2f + 0.6f * index / (float)(outputs.size() - 1));
			v2 offset = v2(sinf(angle) + offx, -cosf(angle));
			return position + hh + offset * hh + v2(size.x, 0.0f);
		}		
		else
			return position + v2(size.x, headerSize() * 0.5f);
	}
}

size_t Node::GetInputIndex(const std::string& name) const
{
	for (size_t i = 0; i < inputs.size(); i++)
		if (inputs[i].name == name)
			return i;
	return -1;
}

size_t Node::GetOutputIndex(const std::string& name) const
{
	for (size_t i = 0; i < outputs.size(); i++)
		if (outputs[i].name == name)
			return i;
	return -1;
}

v2 Node::GetOutputPos(const std::string& name) const
{
	size_t n = GetOutputIndex(name);
	if (n == -1)
		return v2::zero;
	return GetOutputPos(n);
}

Node::NodeType Node::GetInputType(const std::string& name) const
{
	size_t n = GetInputIndex(name);
	if (n != -1)
		return inputs[n].type;
	return NodeType::Bool;
}

Node::NodeType Node::GetOutputType(const std::string& name) const
{
	size_t n = GetOutputIndex(name);
	if (n != -1)
		return outputs[n].type;
	return NodeType::Bool;
}

void Node::Draw(NodeNetwork* network, bool cullBody)
{
	v2 cursor = position;
	UpdateDimensions();
	if (!cullBody)
	{
		if (!mini)
		{
			// leave spaces
			cursor.y += headerHeight + 4.0f;
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
		{
			network->DrawHeader(cursor, name, size.x, headerSize(), mini);
			for (size_t i = 0; i < inputs.size(); i++)
				network->DrawConnectionEndpoint(GetInputPos(i), network->GetCol(inputs[i].type), true);
			for (size_t i = 0; i < outputs.size(); i++)
				network->DrawConnectionEndpoint(GetOutputPos(i), network->GetCol(outputs[i].type), true);
		}
	}

	// draw connections
	for (const NodeInput& inp : inputs)
	{
		if (inp.source != nullptr)
		{
			network->DrawConnection(GetInputPos(inp.name), inp.source->GetOutputPos(inp.sourceName), inp.type);
		}
	}
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
		size = v2(maxXOff, headerSize());
	else
		size = v2(maxXOff, headerHeight + 8.0f + minSpace.y + inputs.size() * 16.0f + outputs.size() * 16.0f);
}

float Node::IOWidth(const std::string& text)
{
	// return length
	// text space + 8px padding either side
	return 6.0f * (float)(text.size() + 1) + 16.0f;
}